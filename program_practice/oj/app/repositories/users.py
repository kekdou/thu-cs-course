from __future__ import annotations

from pathlib import Path

from app.repositories.database import row_to_dict, transaction
from app.utils.ids import new_id
from app.utils.time import utc_now


def create_user(username: str, password_hash: str, role: str = "student", path: Path | None = None) -> dict:
    now = utc_now()
    user = {
        "id": new_id(),
        "username": username,
        "password_hash": password_hash,
        "role": role,
        "is_active": 1,
        "created_at": now,
        "updated_at": now,
    }
    with transaction(path) as conn:
        conn.execute(
            """
            INSERT INTO users (id, username, password_hash, role, is_active, created_at, updated_at)
            VALUES (:id, :username, :password_hash, :role, :is_active, :created_at, :updated_at)
            """,
            user,
        )
    return user


def get_user(user_id: str, path: Path | None = None) -> dict | None:
    with transaction(path) as conn:
        row = conn.execute("SELECT * FROM users WHERE id = ?", (user_id,)).fetchone()
    return row_to_dict(row)


def get_user_by_username(username: str, path: Path | None = None) -> dict | None:
    with transaction(path) as conn:
        row = conn.execute("SELECT * FROM users WHERE username = ?", (username,)).fetchone()
    return row_to_dict(row)


def list_users(page: int = 1, page_size: int = 20, path: Path | None = None) -> tuple[list[dict], int]:
    offset = (page - 1) * page_size
    with transaction(path) as conn:
        total = conn.execute("SELECT COUNT(*) FROM users").fetchone()[0]
        rows = conn.execute(
            "SELECT * FROM users ORDER BY created_at DESC LIMIT ? OFFSET ?",
            (page_size, offset),
        ).fetchall()
    return [dict(row) for row in rows], total


def update_user_role_status(user_id: str, role: str, is_active: bool, path: Path | None = None) -> dict | None:
    now = utc_now()
    with transaction(path) as conn:
        conn.execute(
            "UPDATE users SET role = ?, is_active = ?, updated_at = ? WHERE id = ?",
            (role, int(is_active), now, user_id),
        )
        row = conn.execute("SELECT * FROM users WHERE id = ?", (user_id,)).fetchone()
    return row_to_dict(row)


def public_user(user: dict) -> dict:
    return {
        "id": user["id"],
        "username": user["username"],
        "role": user["role"],
        "is_active": bool(user["is_active"]),
        "created_at": user["created_at"],
        "updated_at": user["updated_at"],
    }
