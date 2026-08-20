from __future__ import annotations

from typing import Any

from fastapi import status
from fastapi.responses import JSONResponse


def api_response(data: Any = None, message: str = "ok", code: int = status.HTTP_200_OK) -> JSONResponse:
    return JSONResponse(status_code=code, content={"code": code, "message": message, "data": data})


def api_error(code: int, message: str) -> JSONResponse:
    return JSONResponse(status_code=code, content={"code": code, "message": message, "data": None})


def page_data(items: list[Any], total: int, page: int = 1, page_size: int = 20) -> dict[str, Any]:
    return {"items": items, "total": total, "page": page, "page_size": page_size}

