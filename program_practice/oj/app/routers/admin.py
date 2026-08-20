from __future__ import annotations

from fastapi import APIRouter, Depends, status

from app.core.responses import api_response
from app.models.enums import UserRole
from app.routers.deps import require_roles
from app.services import backup_service


router = APIRouter(prefix="/api/admin", tags=["admin"])


@router.post("/backups")
async def create_backup(admin: dict = Depends(require_roles(UserRole.admin))):
    return api_response(backup_service.create_backup(admin), message="backup created", code=status.HTTP_201_CREATED)


@router.get("/backups")
async def list_backups(_admin: dict = Depends(require_roles(UserRole.admin))):
    return api_response(backup_service.list_backups())


@router.post("/backups/{backup_id}/restore")
async def restore_backup(backup_id: str, admin: dict = Depends(require_roles(UserRole.admin))):
    return api_response(backup_service.restore_backup(backup_id, admin))
