from __future__ import annotations

from pathlib import Path

import pytest

from app.core.config import get_settings, reset_settings_cache
from app.repositories.database import initialize_database


@pytest.fixture(autouse=True)
def isolated_runtime(tmp_path: Path, monkeypatch: pytest.MonkeyPatch):
    """为每个测试提供独立数据库、临时目录、备份目录和 Session 配置"""
    monkeypatch.setenv("OJ_DB_PATH", str(tmp_path / "data" / "oj.db"))
    monkeypatch.setenv("OJ_TEMP_DIR", str(tmp_path / "temp"))
    monkeypatch.setenv("OJ_BACKUP_DIR", str(tmp_path / "backups"))
    monkeypatch.setenv("OJ_SESSION_SECRET", "test-session-secret")
    reset_settings_cache()
    initialize_database(get_settings().db_path)
    yield
    reset_settings_cache()
