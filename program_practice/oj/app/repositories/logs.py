from __future__ import annotations

from pathlib import Path
from typing import Any

from app.repositories.database import transaction
from app.utils.ids import new_id
from app.utils.text import sanitize_error_message, truncate_text
from app.utils.time import utc_now


def create_case_log(log: dict, path: Path | None = None) -> dict:
    created_at = utc_now()
    data = {"memory_used": None, **log, "created_at": created_at}
    for key in ("input_data", "stdout", "expected_output"):
        data[key] = truncate_text(data.get(key))
    data["stderr"] = sanitize_error_message(data.get("stderr"))
    data["message"] = sanitize_error_message(data.get("message"))
    with transaction(path) as conn:
        conn.execute(
            """
            INSERT INTO case_logs (
                submission_id, case_id, result, score, time_used, memory_used, exit_code, stdout, stderr,
                input_data, expected_output, is_hidden, message, created_at
            )
            VALUES (
                :submission_id, :case_id, :result, :score, :time_used, :memory_used, :exit_code, :stdout, :stderr,
                :input_data, :expected_output, :is_hidden, :message, :created_at
            )
            """,
            data,
        )
    return data


def list_case_logs(submission_id: str, path: Path | None = None) -> list[dict]:
    with transaction(path) as conn:
        rows = conn.execute(
            "SELECT * FROM case_logs WHERE submission_id = ? ORDER BY id",
            (submission_id,),
        ).fetchall()
    return [dict(row) for row in rows]


def list_case_logs_for_teacher(
    page: int = 1,
    page_size: int = 20,
    submission_id: str | None = None,
    problem_id: str | None = None,
    user_id: str | None = None,
    result: str | None = None,
    start_time: str | None = None,
    end_time: str | None = None,
    path: Path | None = None,
) -> tuple[list[dict], int]:
    offset = (page - 1) * page_size
    conditions: list[str] = []
    params: list[Any] = []
    filters = {
        "case_logs.submission_id": submission_id,
        "submissions.problem_id": problem_id,
        "submissions.user_id": user_id,
        "case_logs.result": result,
    }
    for column, value in filters.items():
        if value:
            conditions.append(f"{column} = ?")
            params.append(value)
    if start_time:
        conditions.append("case_logs.created_at >= ?")
        params.append(start_time)
    if end_time:
        conditions.append("case_logs.created_at <= ?")
        params.append(end_time)

    where = "WHERE " + " AND ".join(conditions) if conditions else ""
    query_from = "FROM case_logs JOIN submissions ON submissions.id = case_logs.submission_id"
    with transaction(path) as conn:
        total = conn.execute(f"SELECT COUNT(*) {query_from} {where}", params).fetchone()[0]
        rows = conn.execute(
            f"""
            SELECT case_logs.*, submissions.problem_id, submissions.user_id
            {query_from}
            {where}
            ORDER BY case_logs.created_at DESC, case_logs.id DESC
            LIMIT ? OFFSET ?
            """,
            (*params, page_size, offset),
        ).fetchall()
    return [dict(row) for row in rows], total


def delete_case_logs(submission_id: str, path: Path | None = None) -> None:
    with transaction(path) as conn:
        conn.execute("DELETE FROM case_logs WHERE submission_id = ?", (submission_id,))


def create_audit_log(
    action: str,
    operator_id: str | None = None,
    target_type: str | None = None,
    target_id: str | None = None,
    detail: str = "",
    success: bool = True,
    path: Path | None = None,
) -> dict:
    log = {
        "id": new_id(),
        "operator_id": operator_id,
        "action": action,
        "target_type": target_type,
        "target_id": target_id,
        "success": success,
        "detail": detail,
        "created_at": utc_now(),
    }
    db_log = {**log, "success": int(success)}
    with transaction(path) as conn:
        conn.execute(
            """
            INSERT INTO audit_logs (id, operator_id, action, target_type, target_id, success, detail, created_at)
            VALUES (:id, :operator_id, :action, :target_type, :target_id, :success, :detail, :created_at)
            """,
            db_log,
        )
    return log


def list_audit_logs(
    page: int = 1,
    page_size: int = 20,
    action: str | None = None,
    operator_id: str | None = None,
    target_type: str | None = None,
    target_id: str | None = None,
    start_time: str | None = None,
    end_time: str | None = None,
    path: Path | None = None,
) -> tuple[list[dict], int]:
    offset = (page - 1) * page_size
    filters = {"action": action, "operator_id": operator_id, "target_type": target_type, "target_id": target_id}
    conditions = [f"{key} = ?" for key, value in filters.items() if value]
    params: list[Any] = [value for value in filters.values() if value]
    if start_time:
        conditions.append("created_at >= ?")
        params.append(start_time)
    if end_time:
        conditions.append("created_at <= ?")
        params.append(end_time)
    where = "WHERE " + " AND ".join(conditions) if conditions else ""
    with transaction(path) as conn:
        total = conn.execute(f"SELECT COUNT(*) FROM audit_logs {where}", params).fetchone()[0]
        rows = conn.execute(
            f"SELECT * FROM audit_logs {where} ORDER BY created_at DESC LIMIT ? OFFSET ?",
            (*params, page_size, offset),
        ).fetchall()
    return [_audit_row_to_dict(row) for row in rows], total


def _audit_row_to_dict(row: Any) -> dict:
    data = dict(row)
    data["success"] = bool(data["success"])
    return data
