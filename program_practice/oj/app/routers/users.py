from __future__ import annotations

from fastapi import APIRouter, Depends

from app.core.responses import api_response, page_data
from app.models.enums import UserRole
from app.models.user import UserUpdateRequest
from app.routers.deps import pagination, require_roles
from app.services import user_service


router = APIRouter(prefix="/api/users", tags=["users"])


@router.get("")
async def list_users(
    paging: tuple[int, int] = Depends(pagination),
    admin: dict = Depends(require_roles(UserRole.admin)),
):
    page, page_size = paging
    items, total = user_service.list_public_users(page, page_size)
    return api_response(page_data(items, total, page, page_size))


@router.get("/{user_id}")
async def get_user(user_id: str, admin: dict = Depends(require_roles(UserRole.admin))):
    return api_response(user_service.get_public_user(user_id))


@router.put("/{user_id}")
async def update_user(
    user_id: str,
    data: UserUpdateRequest,
    admin: dict = Depends(require_roles(UserRole.admin)),
):
    return api_response(user_service.update_user(user_id, data, admin))
