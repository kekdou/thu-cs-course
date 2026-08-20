from __future__ import annotations

import asyncio
from typing import Any

from app.core.config import get_settings
from app.judge import docker_runner
from app.models.enums import JudgeResult
from helper import CORRECT_CODE, FakeDockerProcess, run_fake_case, single_case


def test_docker_command_limits_cpu_memory_network_and_filesystem(monkeypatch) -> None:
    """测试 Docker 命令包含 CPU、内存、网络禁用和文件系统限制参数"""
    commands: list[tuple[str, ...]] = []
    result = run_fake_case(monkeypatch, FakeDockerProcess(tuple()), commands)
    command = commands[0]
    
    assert result["result"] == JudgeResult.AC.value
    assert command[command.index("--network") + 1] == "none"
    assert "--read-only" in command
    assert command[command.index("--tmpfs") + 1] == "/tmp:rw,noexec,nosuid,size=16m"
    assert command[command.index("--memory") + 1] == "64m"
    assert command[command.index("--memory-swap") + 1] == "64m"
    assert command[command.index("--cpus") + 1] == "0.5"
    assert command[command.index("--pids-limit") + 1] == "64"
    assert command[command.index("--cap-drop") + 1] == "ALL"
    assert command[command.index("--security-opt") + 1] == "no-new-privileges:true"


def test_memory_limit_exceeded_returns_mle(monkeypatch) -> None:
    """测试容器因内存限制被 kill 时返回 MLE"""
    commands: list[tuple[str, ...]] = []
    result = run_fake_case(monkeypatch, FakeDockerProcess(tuple(), returncode=137, stderr=b"killed"), commands)

    assert result["result"] == JudgeResult.MLE.value
    assert result["score"] == 0
    assert result["message"] == "memory limit exceeded"


def test_student_process_killed_by_memory_limit_returns_mle(monkeypatch) -> None:
    """测试学生进程被 OOM kill 时返回 MLE，而不是 RE"""
    commands: list[tuple[str, ...]] = []
    result = run_fake_case(
        monkeypatch,
        FakeDockerProcess(tuple(), meta='{"timeout": false, "exit_code": -9, "time_used": 0.01}', stderr=b"killed"),
        commands,
    )

    assert result["result"] == JudgeResult.MLE.value
    assert result["message"] == "memory limit exceeded"
    assert result["stderr"] == ""


def test_python_memory_error_returns_mle(monkeypatch) -> None:
    """测试 Python MemoryError 会被归类为 MLE"""
    commands: list[tuple[str, ...]] = []
    result = run_fake_case(
        monkeypatch,
        FakeDockerProcess(tuple(), meta='{"timeout": false, "exit_code": 1, "time_used": 0.01}', stderr=b"Traceback\nMemoryError\n"),
        commands,
    )

    assert result["result"] == JudgeResult.MLE.value
    assert result["message"] == "memory limit exceeded"
    assert result["stderr"] == ""


def test_network_access_error_log_is_cleaned(monkeypatch) -> None:
    """测试联网失败日志会被清洗为受控信息"""
    commands: list[tuple[str, ...]] = []
    result = run_fake_case(
        monkeypatch,
        FakeDockerProcess(
            tuple(),
            meta='{"timeout": false, "exit_code": 1, "time_used": 0.01}',
            stderr=b'Traceback\nFile "/work/main.py", line 1\nsocket.gaierror: [Errno -3] Temporary failure in name resolution\n',
        ),
        commands,
    )

    assert result["result"] == JudgeResult.RE.value
    assert result["message"] == "network access is disabled"
    assert result["stderr"] == ""


def test_timeout_log_is_cleaned(monkeypatch) -> None:
    """测试 TLE 日志只保留受控超时信息"""
    commands: list[tuple[str, ...]] = []
    result = run_fake_case(
        monkeypatch,
        FakeDockerProcess(tuple(), meta='{"timeout": true, "exit_code": null, "time_used": 0.2}', stderr=b"partial traceback"),
        commands,
    )

    assert result["result"] == JudgeResult.TLE.value
    assert result["message"] == "time limit exceeded"
    assert result["stderr"] == ""


def test_container_start_failure_returns_system_error(monkeypatch) -> None:
    """测试容器启动失败会返回 SE，并保留受控错误信息"""
    commands: list[tuple[str, ...]] = []
    result = run_fake_case(monkeypatch, FakeDockerProcess(tuple(), returncode=125, stderr=b"docker failed"), commands)

    assert result["result"] == JudgeResult.SE.value
    assert "docker failed" in result["message"]


def test_outer_docker_timeout_removes_container(monkeypatch) -> None:
    """测试外层 Docker 超时时会 kill 进程并执行 docker rm -f 清理容器"""
    commands: list[tuple[str, ...]] = []
    slow_process = FakeDockerProcess(tuple())

    async def fake_create_subprocess_exec(*command: str, **_kwargs: Any) -> FakeDockerProcess:
        commands.append(command)
        slow_process.command = command
        return slow_process

    async def fake_wait_for(awaitable: Any, timeout: float | None = None) -> Any:
        """立即触发外层超时，避免测试真实等待 docker_timeout"""
        if hasattr(awaitable, "close"):
            awaitable.close()
        raise asyncio.TimeoutError

    monkeypatch.setattr(docker_runner.asyncio, "create_subprocess_exec", fake_create_subprocess_exec)
    monkeypatch.setattr(docker_runner.asyncio, "wait_for", fake_wait_for)
    work_dir = get_settings().temp_dir / "sandbox-timeout"
    work_dir.mkdir(parents=True, exist_ok=True)

    result = asyncio.run(
        docker_runner.run_case(
            work_dir=work_dir,
            input_data="1 2\n",
            expected_output="3\n",
            case_id="case_01",
            score=100,
            is_hidden=False,
            time_limit=0.01,
            memory_limit=64,
            docker_image="python:3.10-slim",
            docker_cpus="0.5",
        )
    )

    assert result["result"] == JudgeResult.SE.value
    assert result["message"] == "docker runner timed out"
    assert slow_process.killed is True
    assert any(command[:3] == ("docker", "rm", "-f") for command in commands)


def test_judge_python_cleans_temp_directory_without_real_docker(monkeypatch) -> None:
    """测试评测结束后会清理本次沙盒临时目录"""
    async def ok_environment(_image: str) -> None:
        """模拟 Docker 环境可用"""
        return None

    async def fake_run_case(**_kwargs: Any) -> dict:
        """模拟单测试点评测成功"""
        return {
            "case_id": "case_01",
            "result": JudgeResult.AC.value,
            "score": 100,
            "time_used": 0.01,
            "memory_used": None,
            "exit_code": 0,
            "stdout": "3\n",
            "stderr": "",
            "message": "",
            "input": "1 2\n",
            "output": "3\n",
            "is_hidden": False,
        }

    monkeypatch.setattr(docker_runner, "docker_environment_error", ok_environment)
    monkeypatch.setattr(docker_runner, "run_case", fake_run_case)

    result = asyncio.run(docker_runner.judge_python(CORRECT_CODE, "P_SANDBOX", 1.0, 64, single_case(input_data="1 2\n")))
    leftovers = list(get_settings().temp_dir.iterdir()) if get_settings().temp_dir.exists() else []

    assert result["result"] == JudgeResult.AC.value
    assert leftovers == []


def test_docker_environment_failure_returns_system_error(monkeypatch) -> None:
    """测试沙盒环境检查失败时返回 SE 系统错误"""
    async def broken_environment(_image: str) -> str:
        """模拟 Docker daemon 或镜像不可用"""
        return "docker daemon unavailable"

    monkeypatch.setattr(docker_runner, "docker_environment_error", broken_environment)

    result = asyncio.run(docker_runner.judge_python(CORRECT_CODE, "P_SANDBOX", 1.0, 64, single_case(input_data="1 2\n")))

    assert result["result"] == JudgeResult.SE.value
    assert result["cases"][0]["case_id"] == "system"
    assert "docker daemon unavailable" in result["cases"][0]["message"]
