from __future__ import annotations

from pydantic import BaseModel

from app.models.enums import JudgeResult


class CaseLogBase(BaseModel):
    case_id: str
    result: JudgeResult
    score: float
    time_used: float
    memory_used: float | None = None
    stdout: str | None = None
    stderr: str
    message: str
    created_at: str


class StudentCaseLog(CaseLogBase):
    is_hidden: bool
    expected_output: str | None = None


class TeacherCaseLog(CaseLogBase):
    exit_code: int | None = None
    input_data: str
    expected_output: str
    is_hidden: bool


class AuditLogPublic(BaseModel):
    id: str
    operator_id: str | None
    action: str
    target_type: str | None
    target_id: str | None
    success: bool
    detail: str | None
    created_at: str
