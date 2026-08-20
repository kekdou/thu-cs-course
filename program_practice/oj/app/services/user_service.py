from __future__ import annotations

from app.core.errors import Forbidden, NotFound
from app.models.user import UserUpdateRequest
from app.repositories import logs, users
from app.repositories.users import public_user

DISABLE_USER = "DISABLE_USER"
UPDATE_USER_ROLE = "UPDATE_USER_ROLE"


def list_public_users(page: int, page_size: int) -> tuple[list[dict], int]:
    items, total = users.list_users(page=page, page_size=page_size)
    return [public_user(user) for user in items], total


def get_public_user(user_id: str) -> dict:
    user = users.get_user(user_id)
    if user is None:
        raise NotFound("user not found")
    return public_user(user)


def update_user(user_id: str, data: UserUpdateRequest, operator: dict) -> dict:
    if user_id == operator["id"] and data.is_active is False:
        raise Forbidden("cannot disable yourself")
    current = users.get_user(user_id)
    if current is None:
        raise NotFound("user not found")
    user = users.update_user_role_status(user_id, data.role.value, data.is_active)
    if user is None:
        raise NotFound("user not found")
    if current["role"] != user["role"]:
        logs.create_audit_log(
            UPDATE_USER_ROLE,
            operator_id=operator["id"],
            target_type="user",
            target_id=user_id,
            detail=f"{current['role']} -> {user['role']}",
        )
    if bool(current["is_active"]) and not bool(user["is_active"]):
        logs.create_audit_log(
            DISABLE_USER,
            operator_id=operator["id"],
            target_type="user",
            target_id=user_id,
        )
    return public_user(user)
