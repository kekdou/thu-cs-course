from __future__ import annotations

import asyncio
import json
import shutil
from pathlib import Path

from app.core.config import get_settings
from app.judge.compare import is_standard_match
from app.judge.result import summarize_cases
from app.models.enums import JudgeResult
from app.utils.ids import new_id
from app.utils.text import sanitize_error_message, truncate_text


STOP_RESULTS = {
    JudgeResult.RE.value,
    JudgeResult.TLE.value,
    JudgeResult.MLE.value,
    JudgeResult.SE.value,
}

# 测评辅助脚本，每次写入 runner.py
RUNNER_SOURCE = r'''
from __future__ import annotations

import json
import subprocess
import sys
import time


def write_bytes(path: str, data: bytes | None) -> None:
    with open(path, "wb") as file:
        file.write(data or b"")


limit = float(sys.argv[1])
input_data = sys.stdin.buffer.read()
started = time.perf_counter()
try:
    completed = subprocess.run(
        [sys.executable, "main.py"],
        input=input_data,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=limit,
    )
    time_used = time.perf_counter() - started
    write_bytes("stdout.bin", completed.stdout)
    write_bytes("stderr.bin", completed.stderr)
    meta = {"timeout": False, "exit_code": completed.returncode, "time_used": time_used}
    exit_code = 0
except subprocess.TimeoutExpired as exc:
    time_used = time.perf_counter() - started
    write_bytes("stdout.bin", exc.stdout)
    write_bytes("stderr.bin", exc.stderr)
    meta = {"timeout": True, "exit_code": None, "time_used": time_used}
    exit_code = 124
except BaseException as exc:
    time_used = time.perf_counter() - started
    write_bytes("stdout.bin", b"")
    write_bytes("stderr.bin", str(exc).encode("utf-8", errors="replace"))
    meta = {"timeout": False, "exit_code": 1, "time_used": time_used}
    exit_code = 1

with open("meta.json", "w", encoding="utf-8") as file:
    json.dump(meta, file)
raise SystemExit(exit_code)
'''.lstrip()


async def judge_python(source_code: str, problem_id: str, time_limit: float, memory_limit: int, test_cases: list[dict]) -> dict:
    settings = get_settings()
    # 创建临时目录
    work_dir = settings.temp_dir / f"{problem_id}-{new_id()}"
    work_dir.mkdir(parents=True, exist_ok=True)
    work_dir.chmod(0o777)
    try:
        # 写入 main.py 和 runner.py
        (work_dir / "main.py").write_text(source_code, encoding="utf-8")
        (work_dir / "runner.py").write_text(RUNNER_SOURCE, encoding="utf-8")
        docker_error = await docker_environment_error(settings.docker_image)
        if docker_error:
            return system_error(docker_error)
        cases = []
        for case in test_cases:
            case_result = await run_case(
                work_dir=work_dir,
                input_data=case["input"],
                expected_output=case["output"],
                case_id=case["case_id"],
                score=case["score"],
                is_hidden=case["is_hidden"],
                time_limit=time_limit,
                memory_limit=memory_limit,
                docker_image=settings.docker_image,
                docker_cpus=settings.docker_cpus,
            )
            cases.append(case_result)
            if case_result["result"] in STOP_RESULTS:
                break
        return summarize_cases(cases)
    except Exception:
        return {
            "result": JudgeResult.SE.value,
            "score": 0,
            "total_time": 0.0,
            "cases": [
                {
                    "case_id": "system",
                    "result": JudgeResult.SE.value,
                    "score": 0,
                    "time_used": 0.0,
                    "memory_used": None,
                    "exit_code": None,
                    "stdout": "",
                    "stderr": "",
                    "message": "judge system error",
                    "input": "",
                    "output": "",
                    "is_hidden": True,
                }
            ],
        }
    finally:
        shutil.rmtree(work_dir, ignore_errors=True)


def system_error(message: str) -> dict:
    return {
        "result": JudgeResult.SE.value,
        "score": 0,
        "total_time": 0.0,
        "cases": [
            {
                "case_id": "system",
                "result": JudgeResult.SE.value,
                "score": 0,
                "time_used": 0.0,
                "memory_used": None,
                "exit_code": None,
                "stdout": "",
                "stderr": "",
                "message": sanitize_error_message(message),
                "input": "",
                "output": "",
                "is_hidden": True,
            }
        ],
    }


async def docker_environment_error(image: str) -> str | None:
    try:
        info = await run_docker_check("docker", "info")
        if info:
            return info
        inspect = await run_docker_check("docker", "image", "inspect", image)
        if inspect:
            return f"docker image is not available: {image}"
    except FileNotFoundError:
        return "docker command not found"
    return None


async def run_docker_check(*command: str) -> str | None:
    process = await asyncio.create_subprocess_exec(
        *command,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
    )
    try:
        stdout, stderr = await asyncio.wait_for(process.communicate(), timeout=3.0)
    except asyncio.TimeoutError:
        process.kill()
        await process.wait()
        return "docker environment check timed out"
    if process.returncode != 0:
        text = (stderr or stdout).decode("utf-8", errors="replace")
        return sanitize_error_message(text) or "docker environment is unavailable"
    return None

# docker run ... -v {work_dir}:/work:rw -w /work image python runner.py {time_limit}
async def run_case(
    work_dir: Path,
    input_data: str,
    expected_output: str,
    case_id: str,
    score: float,
    is_hidden: bool,
    time_limit: float,
    memory_limit: int,
    docker_image: str,
    docker_cpus: str,
) -> dict:
    container_name = "oj-" + new_id()
    docker_timeout = max(time_limit, 0.1) + 8.0
    command = [
        "docker",
        "run",
        "--rm",
        "-i",
        "--pull",
        "never",
        "--name",
        container_name,
        "--network",
        "none",
        "--read-only",
        "--tmpfs",
        "/tmp:rw,noexec,nosuid,size=16m",
        "--memory",
        f"{memory_limit}m",
        "--memory-swap",
        f"{memory_limit}m",
        "--cpus",
        docker_cpus,
        "--pids-limit",
        "64",
        "--cap-drop",
        "ALL",
        "--security-opt",
        "no-new-privileges:true",
        "-e",
        "PYTHONDONTWRITEBYTECODE=1",
        "-v",
        f"{work_dir}:/work:rw",
        "-w",
        "/work",
        docker_image,
        "python",
        "runner.py",
        str(time_limit),
    ]
    try:
        # 创建子进程
        process = await asyncio.create_subprocess_exec(
            *command,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )
        try:
            stdout_bytes, stderr_bytes = await asyncio.wait_for(
                process.communicate(input_data.encode("utf-8")),
                timeout=docker_timeout,
            )
        except asyncio.TimeoutError:
            process.kill()
            await process.wait()
            await remove_container(container_name)
            return case_log(case_id, JudgeResult.SE.value, 0, 0.0, None, "", "", input_data, expected_output, is_hidden, "docker runner timed out")

        meta = read_runner_meta(work_dir)
        time_used = float(meta.get("time_used", 0.0))
        exit_code = meta.get("exit_code")
        stdout_raw = read_bytes(work_dir / "stdout.bin", fallback=stdout_bytes)
        stderr_raw = read_bytes(work_dir / "stderr.bin", fallback=stderr_bytes)
        try:
            stdout = stdout_raw.decode("utf-8")
            stderr = stderr_raw.decode("utf-8")
        except UnicodeDecodeError:
            return case_log(case_id, JudgeResult.RE.value, 0, time_used, exit_code, "", "output is not valid UTF-8", input_data, expected_output, is_hidden, "output is not valid UTF-8")

        if process.returncode == 125:
            message = sanitize_error_message(stderr) or "docker container failed to start"
            return case_log(case_id, JudgeResult.SE.value, 0, time_used, process.returncode, stdout, stderr, input_data, expected_output, is_hidden, message)
        if is_memory_limit_exceeded(process.returncode, exit_code, stderr):
            return case_log(case_id, JudgeResult.MLE.value, 0, time_used, process.returncode, stdout, "", input_data, expected_output, is_hidden, "memory limit exceeded")
        if not meta and process.returncode == 0:
            if is_standard_match(stdout, expected_output):
                return case_log(case_id, JudgeResult.AC.value, score, 0.0, process.returncode, stdout, stderr, input_data, expected_output, is_hidden, "")
            return case_log(case_id, JudgeResult.WA.value, 0, 0.0, process.returncode, stdout, stderr, input_data, expected_output, is_hidden, "wrong answer")
        if is_docker_environment_error(stdout, stderr) or not meta:
            message = sanitize_error_message(stderr or stdout)
            return case_log(case_id, JudgeResult.SE.value, 0, time_used, process.returncode, stdout, stderr, input_data, expected_output, is_hidden, message)
        if meta.get("timeout"):
            return case_log(case_id, JudgeResult.TLE.value, 0, time_used, exit_code, stdout, "", input_data, expected_output, is_hidden, "time limit exceeded")
        if is_network_access_error(stderr):
            return case_log(case_id, JudgeResult.RE.value, 0, time_used, exit_code, stdout, "", input_data, expected_output, is_hidden, "network access is disabled")
        if exit_code != 0:
            message = sanitize_error_message(stderr) or "runtime error"
            return case_log(case_id, JudgeResult.RE.value, 0, time_used, exit_code, stdout, stderr, input_data, expected_output, is_hidden, message)
        if is_standard_match(stdout, expected_output):
            return case_log(case_id, JudgeResult.AC.value, score, time_used, exit_code, stdout, stderr, input_data, expected_output, is_hidden, "")
        return case_log(case_id, JudgeResult.WA.value, 0, time_used, exit_code, stdout, stderr, input_data, expected_output, is_hidden, "wrong answer")
    except FileNotFoundError:
        return case_log(case_id, JudgeResult.SE.value, 0, 0.0, None, "", "docker command not found", input_data, expected_output, is_hidden, "docker command not found")


def read_runner_meta(work_dir: Path) -> dict:
    meta_path = work_dir / "meta.json"
    if not meta_path.exists():
        return {}
    try:
        return json.loads(meta_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return {}


def read_bytes(path: Path, fallback: bytes) -> bytes:
    if path.exists():
        return path.read_bytes()
    return fallback


def is_docker_environment_error(stdout: str, stderr: str) -> bool:
    text = (stdout + "\n" + stderr).lower()
    markers = [
        "docker daemon",
        "docker desktop",
        "could not be found in this wsl",
        "cannot connect to the docker daemon",
        "error response from daemon",
    ]
    return any(marker in text for marker in markers)


def is_memory_limit_exceeded(process_returncode: int | None, exit_code: int | None, stderr: str) -> bool:
    if process_returncode == 137 or exit_code in {-9, 137}:
        return True
    return "memoryerror" in stderr.lower()


def is_network_access_error(stderr: str) -> bool:
    text = stderr.lower()
    markers = [
        "network is unreachable",
        "temporary failure in name resolution",
        "name or service not known",
        "nodename nor servname provided",
        "socket.gaierror",
        "urllib.error.urlerror",
        "requests.exceptions.connectionerror",
        "failed to establish a new connection",
        "max retries exceeded",
    ]
    return any(marker in text for marker in markers)


async def remove_container(container_name: str) -> None:
    process = await asyncio.create_subprocess_exec(
        "docker",
        "rm",
        "-f",
        container_name,
        stdout=asyncio.subprocess.DEVNULL,
        stderr=asyncio.subprocess.DEVNULL,
    )
    await process.wait()


def case_log(
    case_id: str,
    result: str,
    score: float,
    time_used: float,
    exit_code: int | None,
    stdout: str,
    stderr: str,
    input_data: str,
    expected_output: str,
    is_hidden: bool,
    message: str,
) -> dict:
    return {
        "case_id": case_id,
        "result": result,
        "score": score,
        "time_used": round(time_used, 6),
        "memory_used": None,
        "exit_code": exit_code,
        "stdout": truncate_text(stdout),
        "stderr": sanitize_error_message(stderr),
        "message": sanitize_error_message(message),
        "input": input_data,
        "output": expected_output,
        "is_hidden": bool(is_hidden),
    }
