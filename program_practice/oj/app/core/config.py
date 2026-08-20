from __future__ import annotations

import os
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[2]

@dataclass(frozen=True)
class Settings:
    db_path: Path
    temp_dir: Path
    backup_dir: Path
    session_secret: str
    docker_image: str
    docker_cpus: str
    admin_username: str
    admin_password: str


@lru_cache(maxsize=1)
def get_settings() -> Settings:
    return Settings(
        db_path=Path(os.getenv("OJ_DB_PATH", ROOT_DIR / "data" / "oj.db")),
        temp_dir=Path(os.getenv("OJ_TEMP_DIR", ROOT_DIR / "temp")),
        backup_dir=Path(os.getenv("OJ_BACKUP_DIR", ROOT_DIR / "backups")),
        session_secret=os.getenv("OJ_SESSION_SECRET", "dev-session-secret-change-me"),
        docker_image=os.getenv("OJ_DOCKER_IMAGE", "python:3.10-slim"),
        docker_cpus=os.getenv("OJ_DOCKER_CPUS", "1.0"),
        admin_username=os.getenv("OJ_ADMIN_USERNAME", "admin"),
        admin_password=os.getenv("OJ_ADMIN_PASSWORD", "admin12345"),
    )


def reset_settings_cache() -> None:
    get_settings.cache_clear()
