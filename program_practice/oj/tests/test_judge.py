from __future__ import annotations

import asyncio

from app.core.config import get_settings
from app.models.enums import JudgeResult
from app.repositories import problems, submissions
from app.services import submission_service
from helper import (
    CORRECT_CODE,
    RUNTIME_ERROR_CODE,
    TIME_LIMIT_CODE,
    WRONG_ANSWER_CODE,
    create_user,
    login_as_student,
    problem_payload,
    run_judge,
    single_case,
)


def test_recognizes_ac_wa_re_and_tle_code() -> None:
    """测试评测器能正确识别 AC、WA、RE、TLE 四类典型 Python 代码"""
    # AC
    assert run_judge(CORRECT_CODE)["result"] == JudgeResult.AC.value
    # WA
    assert run_judge(WRONG_ANSWER_CODE)["result"] == JudgeResult.WA.value
    # RE
    assert run_judge(RUNTIME_ERROR_CODE)["result"] == JudgeResult.RE.value
    # TLE
    assert run_judge(TIME_LIMIT_CODE, time_limit=0.2)["result"] == JudgeResult.TLE.value


def test_multiple_test_cases_are_scored() -> None:
    """测试多测试点评测会逐点执行并按 AC 测试点累加得分"""
    result = run_judge(CORRECT_CODE)

    assert result["result"] == JudgeResult.AC.value
    assert result["score"] == 100
    assert [case["case_id"] for case in result["cases"]] == ["case_01", "case_02"]


def test_trailing_spaces_and_windows_line_endings_are_normalized() -> None:
    """测试输出比较会忽略行末空格，并统一 Windows 与 Linux 换行符"""
    trailing_spaces = run_judge('print("3   ")\n', single_case(output="3\n"))
    windows_newline = run_judge(
        'import sys\nsys.stdout.write("3\\r\\n")\n',
        single_case(output="3\n"),
    )

    assert trailing_spaces["result"] == JudgeResult.AC.value
    assert windows_newline["result"] == JudgeResult.AC.value


def test_empty_code_is_rejected_by_submission_api() -> None:
    """测试提交接口在评测前拒绝空白源码并返回 422"""
    body = problem_payload()
    problems.create_problem(body)
    api = login_as_student()

    response = api.post("/api/submissions", json={"problem_id": body["id"], "language": "python", "source_code": "   \n"})

    assert response.status_code == 422
    assert "source_code" in response.json()["message"]


def test_non_utf8_output_is_runtime_error() -> None:
    """测试学生程序输出非 UTF-8 字节时，评测结果为 RE"""
    result = run_judge('import sys\nsys.stdout.buffer.write(b"\\xff")\n', single_case(output=""))

    assert result["result"] == JudgeResult.RE.value
    assert "UTF-8" in result["cases"][0]["message"]


def test_temporary_directory_is_cleaned() -> None:
    """测试评测结束后会清理本次提交的临时工作目录"""
    settings = get_settings()

    result = run_judge(CORRECT_CODE)
    leftovers = list(settings.temp_dir.iterdir()) if settings.temp_dir.exists() else []

    assert result["result"] == JudgeResult.AC.value
    assert leftovers == []


def test_judge_exception_marks_submission_as_system_error(monkeypatch) -> None:
    """测试评测器自身异常时，提交被标记为 failed/SE"""
    async def fail_judge(*_args, **_kwargs) -> dict:
        """模拟评测器内部崩溃"""
        raise RuntimeError("judge crashed")

    monkeypatch.setattr(submission_service, "judge_python", fail_judge)
    user = create_user("student")
    problem = problems.create_problem(problem_payload())
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)

    asyncio.run(submission_service.evaluate_submission(submission["id"]))
    updated = submissions.get_submission(submission["id"])

    assert updated is not None
    assert updated["status"] == "failed"
    assert updated["result"] == JudgeResult.SE.value
