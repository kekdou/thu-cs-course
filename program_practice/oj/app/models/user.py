from __future__ import annotations

from pydantic import BaseModel, Field

from app.models.enums import UserRole


class RegisterRequest(BaseModel):
    username: str = Field(min_length=3, max_length=32)
    password: str = Field(min_length=8)


class LoginRequest(BaseModel):
    username: str = Field(min_length=3, max_length=32)
    password: str = Field(min_length=8)


class UserUpdateRequest(BaseModel):
    role: UserRole
    is_active: bool


class UserPublic(BaseModel):
    id: str
    username: str
    role: UserRole
    is_active: bool
    created_at: str
    updated_at: str
