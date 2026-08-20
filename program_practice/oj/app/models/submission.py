from __future__ import annotations

from pydantic import BaseModel, Field, field_validator

from app.models.enums import JudgeResult, SubmissionStatus


MAX_SOURCE_SIZE = 64 * 1024


class SubmissionCreateRequest(BaseModel):
    problem_id: str = Field(min_length=1)
    language: str
    source_code: str

    @field_validator("language")
    @classmethod
    def validate_language(cls, value: str) -> str:
        if value != "python":
            raise ValueError("only python is supported")
        return value

    @field_validator("source_code")
    @classmethod
    def validate_source_code(cls, value: str) -> str:
        if not value.strip():
            raise ValueError("source_code cannot be empty")
        if len(value.encode("utf-8")) > MAX_SOURCE_SIZE:
            raise ValueError("source_code must not exceed 64 KiB")
        return value


class SubmissionPublic(BaseModel):
    id: str
    user_id: str
    problem_id: str
    language: str
    status: SubmissionStatus
    result: JudgeResult | None
    score: float
    total_time: float | None
    created_at: str
    started_at: str | None
    finished_at: str | None


class SubmissionDetail(SubmissionPublic):
    source_code: str
