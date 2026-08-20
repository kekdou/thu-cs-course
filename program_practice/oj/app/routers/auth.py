from __future__ import annotations

from fastapi import APIRouter, Depends, Request, status

from app.core.responses import api_response
from app.models.user import LoginRequest, RegisterRequest
from app.routers.deps import current_user
from app.services import auth_service
from app.repositories.users import public_user


router = APIRouter(prefix="/api/auth", tags=["auth"])


@router.post("/register")
async def register(data: RegisterRequest):
    return api_response(auth_service.register(data), code=status.HTTP_201_CREATED)


@router.post("/login")
async def login(request: Request, data: LoginRequest):
    return api_response(auth_service.login(request, data))


@router.post("/logout")
async def logout(request: Request):
    auth_service.logout(request)
    return api_response(None)


@router.get("/me")
async def me(user: dict = Depends(current_user)):
    return api_response(public_user(user))
