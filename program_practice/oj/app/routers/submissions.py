from __future__ import annotations

from fastapi import APIRouter, BackgroundTasks, Depends, Query, status

from app.core.responses import api_response, page_data
from app.models.enums import UserRole
from app.models.submission import SubmissionCreateRequest
from app.routers.deps import current_user, pagination, require_roles
from app.services import log_service, submission_service


router = APIRouter(prefix="/api/submissions", tags=["submissions"])

# 创建 submission 请求
@router.post("")
async def create_submission(
    data: SubmissionCreateRequest,
    background_tasks: BackgroundTasks,
    user: dict = Depends(current_user),
):
    submission = submission_service.create_submission(data, user)
    background_tasks.add_task(submission_service.evaluate_submission, submission["id"])
    return api_response(submission, message="submission accepted", code=status.HTTP_202_ACCEPTED)


@router.get("")
async def list_submissions(
    paging: tuple[int, int] = Depends(pagination),
    problem_id: str | None = Query(default=None),
    user_id: str | None = Query(default=None),
    status: str | None = Query(default=None),
    result: str | None = Query(default=None),
    start_time: str | None = Query(default=None),
    end_time: str | None = Query(default=None),
    user: dict = Depends(current_user),
):
    page, page_size = paging
    items, total = submission_service.list_submissions(
        page,
        page_size,
        user,
        problem_id=problem_id,
        user_id=user_id,
        status=status,
        result=result,
        start_time=start_time,
        end_time=end_time,
    )
    return api_response(page_data(items, total, page, page_size))


@router.get("/{submission_id}")
async def get_submission(submission_id: str, user: dict = Depends(current_user)):
    return api_response(submission_service.get_submission_detail(submission_id, user))


@router.get("/{submission_id}/logs")
async def list_submission_logs(submission_id: str, user: dict = Depends(current_user)):
    return api_response(log_service.list_submission_logs(submission_id, user))


@router.post("/{submission_id}/rejudge")
async def rejudge_submission(
    submission_id: str,
    background_tasks: BackgroundTasks,
    _teacher: dict = Depends(require_roles(UserRole.teacher, UserRole.admin)),
):
    submission = submission_service.rejudge_submission(submission_id, _teacher)
    background_tasks.add_task(submission_service.evaluate_submission, submission["id"])
    return api_response(submission, message="submission accepted", code=status.HTTP_202_ACCEPTED)
