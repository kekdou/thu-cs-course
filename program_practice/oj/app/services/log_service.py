from __future__ import annotations

from app.core.errors import Forbidden, NotFound
from app.repositories import logs, submissions
from app.utils.text import sanitize_error_message, sanitize_student_error_message

VIEW_FULL_JUDGE_LOG = "VIEW_FULL_JUDGE_LOG"


def _can_view_full_logs(user: dict) -> bool:
    return user["role"] in {"teacher", "admin"}


def list_submission_logs(submission_id: str, user: dict) -> list[dict]:
    submission = submissions.get_submission(submission_id)
    if submission is None:
        raise NotFound("submission not found")
    if not _can_view_full_logs(user) and submission["user_id"] != user["id"]:
        raise Forbidden()

    case_logs = logs.list_case_logs(submission_id)
    if _can_view_full_logs(user):
        logs.create_audit_log(
            VIEW_FULL_JUDGE_LOG,
            operator_id=user["id"],
            target_type="submission",
            target_id=submission_id,
        )
        return [_teacher_case_log(item) for item in case_logs]
    return [_student_case_log(item) for item in case_logs]


def list_case_logs(
    page: int,
    page_size: int,
    user: dict,
    submission_id: str | None = None,
    problem_id: str | None = None,
    user_id: str | None = None,
    result: str | None = None,
    start_time: str | None = None,
    end_time: str | None = None,
) -> tuple[list[dict], int]:
    items, total = logs.list_case_logs_for_teacher(
        page=page,
        page_size=page_size,
        submission_id=submission_id,
        problem_id=problem_id,
        user_id=user_id,
        result=result,
        start_time=start_time,
        end_time=end_time,
    )
    logs.create_audit_log(
        VIEW_FULL_JUDGE_LOG,
        operator_id=user["id"],
        target_type="submission" if submission_id else "case_logs",
        target_id=submission_id,
    )
    return [_teacher_case_log(item) for item in items], total


def list_audit_logs(
    page: int,
    page_size: int,
    action: str | None = None,
    operator_id: str | None = None,
    target_type: str | None = None,
    target_id: str | None = None,
    start_time: str | None = None,
    end_time: str | None = None,
) -> tuple[list[dict], int]:
    return logs.list_audit_logs(
        page=page,
        page_size=page_size,
        action=action,
        operator_id=operator_id,
        target_type=target_type,
        target_id=target_id,
        start_time=start_time,
        end_time=end_time,
    )


def _common_case_log(log: dict) -> dict:
    return {
        "case_id": log["case_id"],
        "result": log["result"],
        "score": log["score"],
        "time_used": log["time_used"],
        "memory_used": log["memory_used"],
        "is_hidden": bool(log["is_hidden"]),
        "created_at": log["created_at"],
    }


def _student_case_log(log: dict) -> dict:
    data = _common_case_log(log)
    is_hidden = bool(log["is_hidden"])
    is_system_error = log["result"] == "SE"
    data["stderr"] = "" if is_system_error else sanitize_student_error_message(log["stderr"])
    data["message"] = "System Error" if is_system_error else sanitize_student_error_message(log["message"])
    if not is_hidden:
        data["stdout"] = "" if is_system_error else sanitize_student_error_message(log["stdout"])
        data["expected_output"] = log["expected_output"]
    return data


def _teacher_case_log(log: dict) -> dict:
    data = _common_case_log(log)
    data["exit_code"] = log["exit_code"]
    data["stdout"] = sanitize_error_message(log["stdout"])
    data["stderr"] = sanitize_error_message(log["stderr"])
    data["message"] = sanitize_error_message(log["message"])
    data["submission_id"] = log["submission_id"]
    if "problem_id" in log:
        data["problem_id"] = log["problem_id"]
    if "user_id" in log:
        data["user_id"] = log["user_id"]
    data["input_data"] = log["input_data"]
    data["expected_output"] = log["expected_output"]
    return data
