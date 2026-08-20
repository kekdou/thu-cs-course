from __future__ import annotations

import sqlite3
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

from app.core.config import get_settings
from app.core.security import hash_password
from app.utils.ids import new_id
from app.utils.time import utc_now


def db_path() -> Path:
    return get_settings().db_path


def connect(path: Path | None = None) -> sqlite3.Connection:
    target = path or db_path()
    target.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(target)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    return conn

# 用 with statement 优化 rollback 过程
@contextmanager
def transaction(path: Path | None = None) -> Iterator[sqlite3.Connection]:
    conn = connect(path)
    try:
        yield conn
        conn.commit()
    except Exception:
        conn.rollback()
        raise
    finally:
        conn.close()


def initialize_database(path: Path | None = None) -> None:
    with transaction(path) as conn:
        create_schema(conn)
        create_default_admin(conn)


def create_schema(conn: sqlite3.Connection) -> None:
    conn.executescript(
        """
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL CHECK (role IN ('student', 'teacher', 'admin')),
            is_active INTEGER NOT NULL DEFAULT 1,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS problems (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            input_description TEXT NOT NULL,
            output_description TEXT NOT NULL,
            samples_json TEXT NOT NULL,
            constraints_text TEXT NOT NULL,
            time_limit REAL NOT NULL,
            memory_limit INTEGER NOT NULL,
            difficulty TEXT NOT NULL CHECK (difficulty IN ('easy', 'medium', 'hard')),
            tags_json TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS test_cases (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            problem_id TEXT NOT NULL,
            case_id TEXT NOT NULL,
            input_data TEXT NOT NULL,
            expected_output TEXT NOT NULL,
            score REAL NOT NULL,
            is_hidden INTEGER NOT NULL,
            UNIQUE (problem_id, case_id),
            FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS submissions (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            problem_id TEXT NOT NULL,
            language TEXT NOT NULL,
            source_code TEXT NOT NULL,
            status TEXT NOT NULL CHECK (status IN ('pending', 'running', 'finished', 'failed')),
            result TEXT,
            score REAL NOT NULL DEFAULT 0,
            total_time REAL,
            created_at TEXT NOT NULL,
            started_at TEXT,
            finished_at TEXT
        );

        CREATE TABLE IF NOT EXISTS case_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            submission_id TEXT NOT NULL,
            case_id TEXT NOT NULL,
            result TEXT NOT NULL,
            score REAL NOT NULL,
            time_used REAL NOT NULL,
            memory_used REAL,
            exit_code INTEGER,
            stdout TEXT NOT NULL,
            stderr TEXT NOT NULL,
            input_data TEXT NOT NULL,
            expected_output TEXT NOT NULL,
            is_hidden INTEGER NOT NULL,
            message TEXT NOT NULL,
            created_at TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS audit_logs (
            id TEXT PRIMARY KEY,
            operator_id TEXT,
            action TEXT NOT NULL,
            target_type TEXT,
            target_id TEXT,
            success INTEGER NOT NULL DEFAULT 1,
            detail TEXT NOT NULL,
            created_at TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS backups (
            id TEXT PRIMARY KEY,
            backup_id TEXT NOT NULL UNIQUE,
            path TEXT NOT NULL,
            manifest_json TEXT NOT NULL,
            created_at TEXT NOT NULL
        );
        """
    )
    columns = {row[1] for row in conn.execute("PRAGMA table_info(audit_logs)").fetchall()}
    if "success" not in columns:
        conn.execute("ALTER TABLE audit_logs ADD COLUMN success INTEGER NOT NULL DEFAULT 1")


def create_default_admin(conn: sqlite3.Connection) -> None:
    settings = get_settings()
    exists = conn.execute("SELECT 1 FROM users WHERE username = ?", (settings.admin_username,)).fetchone()
    if exists:
        return
    now = utc_now()
    conn.execute(
        """
        INSERT INTO users (id, username, password_hash, role, is_active, created_at, updated_at)
        VALUES (?, ?, ?, 'admin', 1, ?, ?)
        """,
        (new_id(), settings.admin_username, hash_password(settings.admin_password), now, now),
    )


def row_to_dict(row: sqlite3.Row | None) -> dict | None:
    return dict(row) if row is not None else None
