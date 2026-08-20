from __future__ import annotations

from collections.abc import Callable

from fastapi import Depends, Query, Request

from app.core.errors import Forbidden
from app.models.enums import UserRole
from app.services.auth_service import current_user_from_session


async def current_user(request: Request) -> dict:
    return current_user_from_session(request)


def require_roles(*roles: UserRole) -> Callable:
    allowed = {role.value for role in roles}

    async def dependency(user: dict = Depends(current_user)) -> dict:
        if user["role"] not in allowed:
            raise Forbidden()
        return user

    return dependency


async def pagination(
    page: int = Query(default=1, ge=1),
    page_size: int = Query(default=20, ge=1, le=100),
) -> tuple[int, int]:
    return page, page_size
