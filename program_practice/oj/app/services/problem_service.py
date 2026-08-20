from __future__ import annotations

import sqlite3

from app.core.errors import BadRequest, Conflict, InternalError, NotFound
from app.models.problem import ProblemCreateRequest, ProblemUpdateRequest
from app.repositories import problems


def _problem_payload(problem: ProblemCreateRequest | ProblemUpdateRequest) -> dict:
    return problem.model_dump(mode="json")


def _list_item(problem: dict) -> dict:
    return {
        "id": problem["id"],
        "title": problem["title"],
        "difficulty": problem["difficulty"],
        "tags": problem["tags"],
        "time_limit": problem["time_limit"],
        "memory_limit": problem["memory_limit"],
    }


def _student_detail(problem: dict) -> dict:
    return {key: value for key, value in problem.items() if key not in {"test_cases", "created_at", "updated_at"}}


def list_problem_items(page: int, page_size: int) -> tuple[list[dict], int]:
    items, total = problems.list_problems(page=page, page_size=page_size)
    return [_list_item(problem) for problem in items], total


def get_problem_detail(problem_id: str, user: dict) -> dict:
    is_teacher = user["role"] in {"teacher", "admin"}
    problem = problems.get_problem(problem_id, include_cases=is_teacher)
    if problem is None:
        raise NotFound("problem not found")
    if is_teacher:
        return problem
    return _student_detail(problem)


def create_problem(data: ProblemCreateRequest) -> dict:
    try:
        return problems.create_problem(_problem_payload(data))
    except sqlite3.IntegrityError as exc:
        raise Conflict("problem already exists") from exc
    except sqlite3.Error as exc:
        raise InternalError("problem save failed") from exc


def update_problem(problem_id: str, data: ProblemUpdateRequest) -> dict:
    if data.id != problem_id:
        raise BadRequest("problem id cannot be changed")
    try:
        updated = problems.update_problem(problem_id, _problem_payload(data))
    except sqlite3.Error as exc:
        raise InternalError("problem save failed") from exc
    if updated is None:
        raise NotFound("problem not found")
    return updated


def delete_problem(problem_id: str) -> None:
    if not problems.delete_problem(problem_id):
        raise NotFound("problem not found")
