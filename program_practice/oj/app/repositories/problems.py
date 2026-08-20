from __future__ import annotations

import json
from pathlib import Path

from app.repositories.database import transaction
from app.utils.time import utc_now


def _problem_row_to_dict(row: dict) -> dict:
    data = dict(row)
    data["samples"] = json.loads(data.pop("samples_json"))
    data["constraints"] = data.pop("constraints_text")
    data["tags"] = json.loads(data.pop("tags_json"))
    return data


def _test_case_row_to_dict(row: dict) -> dict:
    data = dict(row)
    data["input"] = data.pop("input_data")
    data["output"] = data.pop("expected_output")
    data["is_hidden"] = bool(data["is_hidden"])
    data.pop("id", None)
    data.pop("problem_id", None)
    return data


def create_problem(problem: dict, path: Path | None = None) -> dict:
    now = utc_now()
    with transaction(path) as conn:
        conn.execute(
            """
            INSERT INTO problems (
                id, title, description, input_description, output_description, samples_json,
                constraints_text, time_limit, memory_limit, difficulty, tags_json, created_at, updated_at
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                problem["id"],
                problem["title"],
                problem["description"],
                problem["input_description"],
                problem["output_description"],
                json.dumps(problem["samples"], ensure_ascii=False),
                problem.get("constraints", ""),
                problem["time_limit"],
                problem["memory_limit"],
                problem["difficulty"],
                json.dumps(problem.get("tags", []), ensure_ascii=False),
                now,
                now,
            ),
        )
        for case in problem["test_cases"]:
            conn.execute(
                """
                INSERT INTO test_cases (problem_id, case_id, input_data, expected_output, score, is_hidden)
                VALUES (?, ?, ?, ?, ?, ?)
                """,
                (
                    problem["id"],
                    case["case_id"],
                    case["input"],
                    case["output"],
                    case["score"],
                    int(case["is_hidden"]),
                ),
            )
    return get_problem(problem["id"], include_cases=True, path=path) or problem


def update_problem(problem_id: str, problem: dict, path: Path | None = None) -> dict | None:
    now = utc_now()
    with transaction(path) as conn:
        exists = conn.execute("SELECT 1 FROM problems WHERE id = ?", (problem_id,)).fetchone()
        if exists is None:
            return None
        conn.execute(
            """
            UPDATE problems
            SET title = ?,
                description = ?,
                input_description = ?,
                output_description = ?,
                samples_json = ?,
                constraints_text = ?,
                time_limit = ?,
                memory_limit = ?,
                difficulty = ?,
                tags_json = ?,
                updated_at = ?
            WHERE id = ?
            """,
            (
                problem["title"],
                problem["description"],
                problem["input_description"],
                problem["output_description"],
                json.dumps(problem["samples"], ensure_ascii=False),
                problem.get("constraints", ""),
                problem["time_limit"],
                problem["memory_limit"],
                problem["difficulty"],
                json.dumps(problem.get("tags", []), ensure_ascii=False),
                now,
                problem_id,
            ),
        )
        conn.execute("DELETE FROM test_cases WHERE problem_id = ?", (problem_id,))
        for case in problem["test_cases"]:
            conn.execute(
                """
                INSERT INTO test_cases (problem_id, case_id, input_data, expected_output, score, is_hidden)
                VALUES (?, ?, ?, ?, ?, ?)
                """,
                (
                    problem_id,
                    case["case_id"],
                    case["input"],
                    case["output"],
                    case["score"],
                    int(case["is_hidden"]),
                ),
            )
    return get_problem(problem_id, include_cases=True, path=path)


def get_problem(problem_id: str, include_cases: bool = False, path: Path | None = None) -> dict | None:
    with transaction(path) as conn:
        row = conn.execute("SELECT * FROM problems WHERE id = ?", (problem_id,)).fetchone()
        if row is None:
            return None
        problem = _problem_row_to_dict(row)
        if include_cases:
            cases = conn.execute(
                "SELECT * FROM test_cases WHERE problem_id = ? ORDER BY id",
                (problem_id,),
            ).fetchall()
            problem["test_cases"] = [_test_case_row_to_dict(case) for case in cases]
    return problem


def list_problems(page: int = 1, page_size: int = 20, path: Path | None = None) -> tuple[list[dict], int]:
    offset = (page - 1) * page_size
    with transaction(path) as conn:
        total = conn.execute("SELECT COUNT(*) FROM problems").fetchone()[0]
        rows = conn.execute(
            "SELECT * FROM problems ORDER BY id LIMIT ? OFFSET ?",
            (page_size, offset),
        ).fetchall()
    return [_problem_row_to_dict(row) for row in rows], total


def delete_problem(problem_id: str, path: Path | None = None) -> bool:
    with transaction(path) as conn:
        conn.execute("DELETE FROM test_cases WHERE problem_id = ?", (problem_id,))
        cursor = conn.execute("DELETE FROM problems WHERE id = ?", (problem_id,))
    return cursor.rowcount > 0


def problem_exists(problem_id: str, path: Path | None = None) -> bool:
    with transaction(path) as conn:
        row = conn.execute("SELECT 1 FROM problems WHERE id = ?", (problem_id,)).fetchone()
    return row is not None
