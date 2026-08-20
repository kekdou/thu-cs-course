from __future__ import annotations

from fastapi import status


class AppError(Exception):
    def __init__(self, status_code: int, message: str) -> None:
        self.status_code = status_code
        self.message = message
        super().__init__(message)


class BadRequest(AppError):
    def __init__(self, message: str) -> None:
        super().__init__(status.HTTP_400_BAD_REQUEST, message)


class Unauthorized(AppError):
    def __init__(self, message: str = "not logged in") -> None:
        super().__init__(status.HTTP_401_UNAUTHORIZED, message)


class Forbidden(AppError):
    def __init__(self, message: str = "permission denied") -> None:
        super().__init__(status.HTTP_403_FORBIDDEN, message)


class NotFound(AppError):
    def __init__(self, message: str = "resource not found") -> None:
        super().__init__(status.HTTP_404_NOT_FOUND, message)


class Conflict(AppError):
    def __init__(self, message: str = "resource conflict") -> None:
        super().__init__(status.HTTP_409_CONFLICT, message)


class InternalError(AppError):
    def __init__(self, message: str = "internal server error") -> None:
        super().__init__(status.HTTP_500_INTERNAL_SERVER_ERROR, message)

