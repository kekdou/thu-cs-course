from __future__ import annotations

from pydantic import BaseModel


class BackupPublic(BaseModel):
    backup_id: str
    created_at: str
    path: str | None = None
    manifest: dict | None = None
