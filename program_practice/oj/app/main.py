from fastapi import FastAPI, Request, status
from fastapi.exceptions import RequestValidationError
from starlette.middleware.sessions import SessionMiddleware

from app.core.config import get_settings
from app.core.errors import AppError
from app.core.responses import api_error, api_response
from app.repositories.database import initialize_database
from app.routers import admin, auth, logs, problems, submissions, users


app = FastAPI(title="OJ System")
app.add_middleware(SessionMiddleware, secret_key=get_settings().session_secret, same_site="lax")
initialize_database()
app.include_router(auth.router)
app.include_router(users.router)
app.include_router(problems.router)
app.include_router(submissions.router)
app.include_router(logs.router)
app.include_router(admin.router)


@app.exception_handler(AppError)
async def app_error_handler(_request: Request, exc: AppError):
    return api_error(exc.status_code, exc.message)


@app.exception_handler(RequestValidationError)
async def validation_error_handler(_request: Request, exc: RequestValidationError):
    return api_error(status.HTTP_422_UNPROCESSABLE_ENTITY, validation_message(exc))


@app.exception_handler(Exception)
async def unexpected_error_handler(_request: Request, _exc: Exception):
    return api_error(status.HTTP_500_INTERNAL_SERVER_ERROR, "internal server error")


@app.get("/api/health")
async def health():
    return api_response({"status": "ok"})


def validation_message(exc: RequestValidationError) -> str:
    first = exc.errors()[0] if exc.errors() else {}
    loc = ".".join(str(item) for item in first.get("loc", []) if item != "body")
    field = loc or "request body"
    message = str(first.get("msg", "invalid input")).replace("Value error, ", "")
    error_type = str(first.get("type", ""))
    ctx = first.get("ctx") or {}

    if error_type == "missing":
        return f"{field} is required"
    if "greater_than" in ctx:
        return f"{field} must be greater than {ctx['greater_than']}"
    if "greater_than_equal" in ctx:
        return f"{field} must be greater than or equal to {ctx['greater_than_equal']}"
    if "less_than_equal" in ctx:
        return f"{field} must be less than or equal to {ctx['less_than_equal']}"
    if error_type.startswith("int"):
        return f"{field} must be an integer"
    if error_type.startswith("float"):
        return f"{field} must be a number"
    return f"{field}: {message}"
