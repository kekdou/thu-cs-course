from __future__ import annotations

import asyncio
from pathlib import Path
from typing import Any
from uuid import uuid4

from fastapi.testclient import TestClient

from app.core.config import get_settings
from app.core.security import hash_password
from app.judge import docker_runner
from app.judge.docker_runner import judge_python
from app.models.enums import JudgeResult
from app.repositories import logs, problems, submissions, users


PASSWORD = "password123"
CORRECT_CODE = "a, b = map(int, input().split())\nprint(a + b)\n"
WRONG_ANSWER_CODE = "print(0)\n"
RUNTIME_ERROR_CODE = "print(1 / 0)\n"
TIME_LIMIT_CODE = "while True:\n    pass\n"


def client() -> TestClient:
    """创建带 Cookie Session 支持的 FastAPI 测试客户端"""
    from app.main import app

    return TestClient(app)


def create_user(role: str = "student", username: str | None = None) -> dict:
    """创建指定角色的测试用户，密码使用统一测试口令并进行哈希保存"""
    return users.create_user(username or f"{role}_{uuid4().hex[:8]}", hash_password(PASSWORD), role=role)


def login_user(user: dict) -> TestClient:
    """登录已创建的测试用户，返回已携带登录态的客户端"""
    api = client()
    response = api.post("/api/auth/login", json={"username": user["username"], "password": PASSWORD})
    assert response.status_code == 200
    return api


def login_as(role: str) -> TestClient:
    """创建指定角色用户并登录，返回已携带登录态的客户端"""
    return login_user(create_user(role))


def login_as_student() -> TestClient:
    """创建并登录一个学生账号，用于学生提交和权限相关测试"""
    return login_as("student")


def register_payload(username: str | None = None, password: str = PASSWORD) -> dict[str, str]:
    """构造注册和登录接口使用的用户名密码载荷"""
    return {"username": username or f"user_{uuid4().hex[:8]}", "password": password}


def problem_payload(problem_id: str | None = None, time_limit: float = 1.0) -> dict[str, Any]:
    """构造合法 A+B 题目，包含一个公开测试点和一个隐藏测试点"""
    return {
        "id": problem_id or f"P{uuid4().hex[:8]}",
        "title": "A+B Problem",
        "description": "Read two integers and print their sum.",
        "input_description": "Two integers a and b.",
        "output_description": "One integer.",
        "samples": [{"input": "1 2\n", "output": "3\n"}],
        "constraints": "|a|, |b| <= 10^9",
        "time_limit": time_limit,
        "memory_limit": 128,
        "difficulty": "easy",
        "tags": ["basic", "io"],
        "test_cases": [
            {"case_id": "case_01", "input": "1 2\n", "output": "3\n", "score": 50, "is_hidden": False},
            {"case_id": "case_02", "input": "-1 2\n", "output": "1\n", "score": 50, "is_hidden": True},
        ],
    }


def create_problem(problem_id: str | None = None) -> dict:
    """创建并返回合法 A+B 题目，用于提交、评测和权限测试"""
    return problems.create_problem(problem_payload(problem_id))


def add_case_log(
    submission_id: str,
    case_id: str = "case_01",
    result: str = JudgeResult.AC.value,
    is_hidden: bool = False,
    stdout: str = "3\n",
    stderr: str = "",
    message: str = "",
) -> dict:
    """写入一条完整测试点日志，供日志权限和展示测试复用"""
    return logs.create_case_log(
        {
            "submission_id": submission_id,
            "case_id": case_id,
            "result": result,
            "score": 100 if result == JudgeResult.AC.value else 0,
            "time_used": 0.01,
            "memory_used": None,
            "exit_code": 0,
            "stdout": stdout,
            "stderr": stderr,
            "input_data": "1 2\n" if not is_hidden else "-1 2\n",
            "expected_output": "3\n" if not is_hidden else "1\n",
            "is_hidden": int(is_hidden),
            "message": message,
        }
    )


def create_logged_submission(user: dict) -> dict:
    """创建一个带公开和隐藏测试点日志的提交"""
    problem = create_problem()
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    add_case_log(submission["id"], case_id="case_01", is_hidden=False)
    add_case_log(submission["id"], case_id="case_02", is_hidden=True, stdout="1\n")
    return submission


def create_finished_logged_submission(user: dict, problem_id: str | None = None) -> dict:
    """创建一个已完成且带测试点日志的 AC 提交"""
    problem = create_problem(problem_id)
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    submissions.mark_running(submission["id"])
    finished = submissions.finish_submission(submission["id"], JudgeResult.AC.value, 100, 0.01)
    add_case_log(submission["id"], case_id="case_01", is_hidden=False)
    add_case_log(submission["id"], case_id="case_02", is_hidden=True, stdout="1\n")
    assert finished is not None
    return finished


def single_case(input_data: str = "", output: str = "3\n") -> list[dict[str, Any]]:
    """构造总分 100 的单测试点列表，用于输出规范化等单点评测"""
    return [{"case_id": "case_01", "input": input_data, "output": output, "score": 100, "is_hidden": False}]


def judge_result(result: str = "AC", score: int = 100) -> dict:
    """构造 submission_service.evaluate_submission 可消费的评测结果"""
    return {
        "result": result,
        "score": score,
        "total_time": 0.01,
        "cases": [
            {
                "case_id": "case_01",
                "result": result,
                "score": score,
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
        ],
    }


def run_judge(source_code: str, test_cases: list[dict[str, Any]] | None = None, time_limit: float = 1.0) -> dict:
    """同步运行异步 Python 评测器，返回评测结果结构"""
    return asyncio.run(
        judge_python(
            source_code,
            f"P{uuid4().hex[:8]}",
            time_limit,
            128,
            test_cases or problem_payload()["test_cases"],
        )
    )


class FakeDockerProcess:
    """模拟 docker 子进程，按测试传入的返回码和输出写入 runner 文件"""

    def __init__(
        self,
        command: tuple[str, ...],
        returncode: int = 0,
        stdout: bytes = b"3\n",
        stderr: bytes = b"",
        delay: float = 0,
        meta: str = '{"timeout": false, "exit_code": 0, "time_used": 0.01}',
    ) -> None:
        self.command = command
        self.returncode = returncode
        self.stdout = stdout
        self.stderr = stderr
        self.delay = delay
        self.meta = meta
        self.killed = False

    async def communicate(self, _stdin: bytes | None = None) -> tuple[bytes, bytes]:
        """模拟容器执行结束，并向挂载工作目录写入评测产物"""
        if self.delay:
            await asyncio.sleep(self.delay)
        work_dir = docker_work_dir(self.command)
        if work_dir is not None:
            (work_dir / "meta.json").write_text(self.meta, encoding="utf-8")
            (work_dir / "stdout.bin").write_bytes(self.stdout)
            (work_dir / "stderr.bin").write_bytes(self.stderr)
        return self.stdout, self.stderr

    def kill(self) -> None:
        """记录外层超时时是否尝试终止容器进程"""
        self.killed = True

    async def wait(self) -> int:
        """模拟等待子进程退出"""
        return self.returncode


def docker_work_dir(command: tuple[str, ...]) -> Path | None:
    """从 docker -v 参数中解析主机侧工作目录"""
    if "-v" not in command:
        return None
    mount = command[command.index("-v") + 1]
    host_path, _container_path, _mode = mount.split(":", 2)
    return Path(host_path)


def run_fake_case(monkeypatch: Any, process: FakeDockerProcess, commands: list[tuple[str, ...]]) -> dict:
    """使用 fake docker 进程执行一个测试点，并记录实际 docker 命令"""
    async def fake_create_subprocess_exec(*command: str, **_kwargs: Any) -> FakeDockerProcess:
        commands.append(command)
        process.command = command
        return process

    monkeypatch.setattr(docker_runner.asyncio, "create_subprocess_exec", fake_create_subprocess_exec)
    work_dir = get_settings().temp_dir / "sandbox-case"
    work_dir.mkdir(parents=True, exist_ok=True)
    return asyncio.run(
        docker_runner.run_case(
            work_dir=work_dir,
            input_data="1 2\n",
            expected_output="3\n",
            case_id="case_01",
            score=100,
            is_hidden=False,
            time_limit=0.2,
            memory_limit=64,
            docker_image="python:3.10-slim",
            docker_cpus="0.5",
        )
    )
