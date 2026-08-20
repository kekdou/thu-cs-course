from __future__ import annotations

from app.core.errors import Conflict, Forbidden, NotFound
from app.judge.docker_runner import judge_python
from app.models.enums import JudgeResult
from app.models.submission import SubmissionCreateRequest
from app.repositories import logs, problems, submissions

REJUDGE_SUBMISSION = "REJUDGE_SUBMISSION"


def _can_view_all(user: dict) -> bool:
    return user["role"] in {"teacher", "admin"}


def _public_submission(submission: dict, include_source: bool = False) -> dict:
    data = {
        "id": submission["id"],
        "submission_id": submission["id"],
        "user_id": submission["user_id"],
        "problem_id": submission["problem_id"],
        "language": submission["language"],
        "status": submission["status"],
        "result": submission["result"],
        "score": submission["score"],
        "total_time": submission["total_time"],
        "created_at": submission["created_at"],
        "started_at": submission["started_at"],
        "finished_at": submission["finished_at"],
    }
    if "username" in submission:
        data["username"] = submission["username"]
    if include_source:
        data["source_code"] = submission["source_code"]
    return data


def _case_log_payload(submission_id: str, case_result: dict) -> dict:
    return {
        "submission_id": submission_id,
        "case_id": case_result["case_id"],
        "result": case_result["result"],
        "score": case_result["score"],
        "time_used": case_result["time_used"],
        "memory_used": case_result.get("memory_used"),
        "exit_code": case_result.get("exit_code"),
        "stdout": case_result.get("stdout", ""),
        "stderr": case_result.get("stderr", ""),
        "input_data": case_result.get("input", ""),
        "expected_output": case_result.get("output", ""),
        "is_hidden": int(case_result.get("is_hidden", False)),
        "message": case_result.get("message", ""),
    }


def _system_case_log_payload(submission_id: str, message: str) -> dict:
    return _case_log_payload(
        submission_id,
        {
            "case_id": "system",
            "result": JudgeResult.SE.value,
            "score": 0,
            "time_used": 0.0,
            "memory_used": None,
            "exit_code": None,
            "stdout": "",
            "stderr": "",
            "input": "",
            "output": "",
            "is_hidden": True,
            "message": message,
        },
    )


def create_submission(data: SubmissionCreateRequest, user: dict) -> dict:
    problem = problems.get_problem(data.problem_id, include_cases=True)
    if problem is None:
        raise NotFound("problem not found")
    submission = submissions.create_submission(user["id"], problem["id"], data.language, data.source_code)
    return _public_submission(submission)


def list_submissions(
    page: int,
    page_size: int,
    user: dict,
    problem_id: str | None = None,
    user_id: str | None = None,
    status: str | None = None,
    result: str | None = None,
    start_time: str | None = None,
    end_time: str | None = None,
) -> tuple[list[dict], int]:
    owner_id = user_id if _can_view_all(user) else user["id"]
    items, total = submissions.list_submissions(
        page=page,
        page_size=page_size,
        user_id=owner_id,
        problem_id=problem_id,
        status=status,
        result=result,
        start_time=start_time,
        end_time=end_time,
    )
    return [_public_submission(item) for item in items], total


def get_submission_detail(submission_id: str, user: dict) -> dict:
    submission = _visible_submission(submission_id, user)
    return _public_submission(submission, include_source=True)


def rejudge_submission(submission_id: str, user: dict) -> dict:
    if not _can_view_all(user):
        raise Forbidden()
    submission = submissions.get_submission(submission_id)
    if submission is None:
        raise NotFound("submission not found")
    if submission["status"] in {"pending", "running"}:
        raise Conflict("submission is already being judged")
    if problems.get_problem(submission["problem_id"], include_cases=True) is None:
        raise NotFound("problem not found")
    logs.delete_case_logs(submission_id)
    reset = submissions.reset_for_rejudge(submission_id)
    if reset is None:
        raise Conflict("submission cannot be rejudged now")
    logs.create_audit_log(
        REJUDGE_SUBMISSION,
        operator_id=user["id"],
        target_type="submission",
        target_id=submission_id,
    )
    return _public_submission(reset)

# 进行测评
async def evaluate_submission(submission_id: str) -> None:
    # 读取提交，要求状态是 pending
    submission = submissions.get_submission(submission_id)
    if submission is None or submission["status"] != "pending":
        return
    # 读取题目和 test_case，将状态改为 running
    problem = problems.get_problem(submission["problem_id"], include_cases=True)
    if problem is None:
        submissions.mark_running(submission_id)
        logs.create_case_log(_system_case_log_payload(submission_id, "problem configuration not found"))
        submissions.finish_submission(submission_id, JudgeResult.SE.value, 0, 0.0, failed=True)
        return

    running = submissions.mark_running(submission_id)
    if running is None or running["status"] != "running":
        return
    # 调用 judge_python 进行测评
    try:
        result = await judge_python(
            submission["source_code"],
            problem["id"],
            problem["time_limit"],
            problem["memory_limit"],
            problem["test_cases"],
        )
    except Exception:
        result = {
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
    # 保存每个测试点日志并汇总
    for case_result in result["cases"]:
        logs.create_case_log(_case_log_payload(submission_id, case_result))
    submissions.finish_submission(
        submission_id,
        result["result"],
        result["score"],
        result["total_time"],
        failed=result["result"] == JudgeResult.SE.value,
    )


def _visible_submission(submission_id: str, user: dict) -> dict:
    submission = submissions.get_submission(submission_id)
    if submission is None:
        raise NotFound("submission not found")
    if not _can_view_all(user) and submission["user_id"] != user["id"]:
        raise Forbidden()
    return submission
