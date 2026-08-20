from __future__ import annotations

from fastapi import APIRouter, Depends, Query

from app.core.responses import api_response, page_data
from app.models.enums import UserRole
from app.routers.deps import pagination, require_roles
from app.services import log_service


router = APIRouter(prefix="/api", tags=["logs"])


@router.get("/logs")
async def list_case_logs(
    paging: tuple[int, int] = Depends(pagination),
    submission_id: str | None = Query(default=None),
    problem_id: str | None = Query(default=None),
    user_id: str | None = Query(default=None),
    result: str | None = Query(default=None),
    start_time: str | None = Query(default=None),
    end_time: str | None = Query(default=None),
    teacher: dict = Depends(require_roles(UserRole.teacher, UserRole.admin)),
):
    page, page_size = paging
    items, total = log_service.list_case_logs(
        page=page,
        page_size=page_size,
        user=teacher,
        submission_id=submission_id,
        problem_id=problem_id,
        user_id=user_id,
        result=result,
        start_time=start_time,
        end_time=end_time,
    )
    return api_response(page_data(items, total, page, page_size))


@router.get("/audit-logs")
async def list_audit_logs(
    paging: tuple[int, int] = Depends(pagination),
    action: str | None = Query(default=None),
    operator_id: str | None = Query(default=None),
    target_type: str | None = Query(default=None),
    target_id: str | None = Query(default=None),
    start_time: str | None = Query(default=None),
    end_time: str | None = Query(default=None),
    _admin: dict = Depends(require_roles(UserRole.admin)),
):
    page, page_size = paging
    items, total = log_service.list_audit_logs(
        page=page,
        page_size=page_size,
        action=action,
        operator_id=operator_id,
        target_type=target_type,
        target_id=target_id,
        start_time=start_time,
        end_time=end_time,
    )
    return api_response(page_data(items, total, page, page_size))
