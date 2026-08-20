from __future__ import annotations

import re


LINUX_TEMP_PATH = re.compile(r"/[^\s:]*temp/[^\s:]+/main\.py")
WINDOWS_TEMP_PATH = re.compile(r"[A-Za-z]:\\[^\s:]+\\temp\\[^\s:]+\\main\.py")
DOCKER_WORK_PATH = re.compile(r"/work/main\.py")
TRACEBACK_LINE = re.compile(r'File "<submission>/main\.py", line (\d+)')


def truncate_text(text: str | None, limit: int = 4000) -> str:
    if not text:
        return ""
    if len(text) <= limit:
        return text
    return text[:limit] + "...[truncated]"


def sanitize_error_message(text: str | None, limit: int = 4000) -> str:
    value = truncate_text(text, limit)
    value = LINUX_TEMP_PATH.sub("<submission>/main.py", value)
    value = DOCKER_WORK_PATH.sub("<submission>/main.py", value)
    return WINDOWS_TEMP_PATH.sub(lambda _match: "<submission>/main.py", value)


def sanitize_student_error_message(text: str | None, limit: int = 4000) -> str:
    value = sanitize_error_message(text, limit)
    if "Traceback (most recent call last):" not in value:
        return value
    match = TRACEBACK_LINE.search(value)
    tail = next(
        (
            line.strip()
            for line in reversed(value.splitlines())
            if line.strip() and "<submission>/main.py" not in line and "Traceback" not in line
        ),
        "runtime error",
    )
    if match:
        return truncate_text(f"程序第 {match.group(1)} 行发生运行错误：{tail}", limit)
    return truncate_text(f"程序发生运行错误：{tail}", limit)
