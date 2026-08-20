from __future__ import annotations

from pathlib import Path

from app.repositories.database import row_to_dict, transaction
from app.utils.ids import new_id
from app.utils.time import utc_now


def create_submission(user_id: str, problem_id: str, language: str, source_code: str, path: Path | None = None) -> dict:
    now = utc_now()
    submission = {
        "id": new_id(),
        "user_id": user_id,
        "problem_id": problem_id,
        "language": language,
        "source_code": source_code,
        "status": "pending",
        "result": None,
        "score": 0,
        "total_time": None,
        "created_at": now,
        "started_at": None,
        "finished_at": None,
    }
    with transaction(path) as conn:
        conn.execute(
            """
            INSERT INTO submissions (
                id, user_id, problem_id, language, source_code, status, result, score,
                total_time, created_at, started_at, finished_at
            )
            VALUES (
                :id, :user_id, :problem_id, :language, :source_code, :status, :result, :score,
                :total_time, :created_at, :started_at, :finished_at
            )
            """,
            submission,
        )
    return submission


def get_submission(submission_id: str, path: Path | None = None) -> dict | None:
    with transaction(path) as conn:
        row = conn.execute("SELECT * FROM submissions WHERE id = ?", (submission_id,)).fetchone()
    return row_to_dict(row)


def list_submissions(
    page: int = 1,
    page_size: int = 20,
    user_id: str | None = None,
    problem_id: str | None = None,
    status: str | None = None,
    result: str | None = None,
    start_time: str | None = None,
    end_time: str | None = None,
    path: Path | None = None,
) -> tuple[list[dict], int]:
    offset = (page - 1) * page_size
    filters = {
        "submissions.user_id": user_id,
        "submissions.problem_id": problem_id,
        "submissions.status": status,
        "submissions.result": result,
    }
    conditions = [f"{column} = ?" for column, value in filters.items() if value]
    params = [value for value in filters.values() if value]
    if start_time:
        conditions.append("submissions.created_at >= ?")
        params.append(start_time)
    if end_time:
        conditions.append("submissions.created_at <= ?")
        params.append(end_time)
    where = "WHERE " + " AND ".join(conditions) if conditions else ""
    with transaction(path) as conn:
        total = conn.execute(f"SELECT COUNT(*) FROM submissions {where}", params).fetchone()[0]
        rows = conn.execute(
            f"""
            SELECT submissions.*, users.username
            FROM submissions
            LEFT JOIN users ON users.id = submissions.user_id
            {where}
            ORDER BY submissions.created_at DESC
            LIMIT ? OFFSET ?
            """,
            (*params, page_size, offset),
        ).fetchall()
    return [dict(row) for row in rows], total


def mark_running(submission_id: str, path: Path | None = None) -> dict | None:
    now = utc_now()
    with transaction(path) as conn:
        conn.execute(
            """
            UPDATE submissions
            SET status = 'running', result = NULL, score = 0, total_time = NULL, started_at = ?, finished_at = NULL
            WHERE id = ? AND status = 'pending'
            """,
            (now, submission_id),
        )
        row = conn.execute("SELECT * FROM submissions WHERE id = ?", (submission_id,)).fetchone()
    return row_to_dict(row)


def finish_submission(
    submission_id: str,
    result: str,
    score: float,
    total_time: float,
    failed: bool = False,
    path: Path | None = None,
) -> dict | None:
    now = utc_now()
    status = "failed" if failed else "finished"
    with transaction(path) as conn:
        conn.execute(
            """
            UPDATE submissions
            SET status = ?, result = ?, score = ?, total_time = ?, finished_at = ?
            WHERE id = ? AND status = 'running'
            """,
            (status, result, score, total_time, now, submission_id),
        )
        row = conn.execute("SELECT * FROM submissions WHERE id = ?", (submission_id,)).fetchone()
    return row_to_dict(row)


def reset_for_rejudge(submission_id: str, path: Path | None = None) -> dict | None:
    with transaction(path) as conn:
        conn.execute(
            """
            UPDATE submissions
            SET status = 'pending', result = NULL, score = 0, total_time = NULL, started_at = NULL, finished_at = NULL
            WHERE id = ? AND status IN ('finished', 'failed')
            """,
            (submission_id,),
        )
        row = conn.execute("SELECT * FROM submissions WHERE id = ?", (submission_id,)).fetchone()
    return row_to_dict(row)
