from __future__ import annotations

import sqlite3

from starlette.requests import Request

from app.core.errors import Conflict, Forbidden, Unauthorized
from app.core.security import hash_password, verify_password
from app.models.user import LoginRequest, RegisterRequest
from app.repositories import users
from app.repositories.users import public_user


SESSION_USER_ID = "user_id"


def register(data: RegisterRequest) -> dict:
    try:
        user = users.create_user(data.username, hash_password(data.password), role="student")
    except sqlite3.IntegrityError as exc:
        raise Conflict("username already exists") from exc
    return public_user(user)


def login(request: Request, data: LoginRequest) -> dict:
    user = users.get_user_by_username(data.username)
    if user is None or not verify_password(data.password, user["password_hash"]):
        raise Unauthorized("invalid username or password")
    if not bool(user["is_active"]):
        raise Forbidden("user is disabled")
    request.session[SESSION_USER_ID] = user["id"]
    return public_user(user)


def logout(request: Request) -> None:
    request.session.clear()


def current_user_from_session(request: Request) -> dict:
    user_id = request.session.get(SESSION_USER_ID)
    if not user_id:
        raise Unauthorized()
    user = users.get_user(user_id)
    if user is None:
        raise Unauthorized()
    if not bool(user["is_active"]):
        raise Forbidden("user is disabled")
    return user
