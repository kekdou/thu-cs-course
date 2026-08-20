from __future__ import annotations

from fastapi import APIRouter, Depends, status

from app.core.responses import api_response, page_data
from app.models.enums import UserRole
from app.models.problem import ProblemCreateRequest, ProblemUpdateRequest
from app.routers.deps import current_user, pagination, require_roles
from app.services import problem_service


router = APIRouter(prefix="/api/problems", tags=["problems"])


@router.get("")
async def list_problems(
    paging: tuple[int, int] = Depends(pagination),
    _user: dict = Depends(current_user),
):
    page, page_size = paging
    items, total = problem_service.list_problem_items(page, page_size)
    return api_response(page_data(items, total, page, page_size))


@router.get("/{problem_id}")
async def get_problem(problem_id: str, user: dict = Depends(current_user)):
    return api_response(problem_service.get_problem_detail(problem_id, user))


@router.post("")
async def create_problem(
    data: ProblemCreateRequest,
    _user: dict = Depends(require_roles(UserRole.teacher, UserRole.admin)),
):
    return api_response(problem_service.create_problem(data), code=status.HTTP_201_CREATED)


@router.put("/{problem_id}")
async def update_problem(
    problem_id: str,
    data: ProblemUpdateRequest,
    _user: dict = Depends(require_roles(UserRole.teacher, UserRole.admin)),
):
    return api_response(problem_service.update_problem(problem_id, data))


@router.delete("/{problem_id}")
async def delete_problem(
    problem_id: str,
    _user: dict = Depends(require_roles(UserRole.teacher, UserRole.admin)),
):
    problem_service.delete_problem(problem_id)
    return api_response(None)
