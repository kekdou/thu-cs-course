from __future__ import annotations

import json
import shutil
import sqlite3
from pathlib import Path

from app.core.config import get_settings
from app.core.errors import BadRequest, NotFound
from app.repositories import backups, logs
from app.repositories.database import initialize_database
from app.utils.ids import new_id
from app.utils.time import utc_now


DB_FILE_NAME = "oj.db"
MANIFEST_FILE_NAME = "manifest.json"
REQUIRED_TABLES = {"users", "problems", "test_cases", "submissions", "case_logs", "audit_logs", "backups"}
CREATE_BACKUP = "CREATE_BACKUP"
RESTORE_BACKUP = "RESTORE_BACKUP"


def create_backup(user: dict) -> dict:
    settings = get_settings()
    source = settings.db_path
    if not source.exists():
        initialize_database(source)

    backup_id = "backup_" + utc_now().replace("-", "").replace(":", "").replace(".", "_") + "_" + new_id()[:8]
    backup_dir = settings.backup_dir / backup_id
    backup_dir.mkdir(parents=True, exist_ok=False)

    manifest = {
        "backup_id": backup_id,
        "storage": "sqlite",
        "files": [DB_FILE_NAME],
        "database": DB_FILE_NAME,
        "created_at": utc_now(),
    }
    try:
        (backup_dir / MANIFEST_FILE_NAME).write_text(json.dumps(manifest, ensure_ascii=False, indent=2), encoding="utf-8")
        record = backups.create_backup_record(backup_id, backup_dir, manifest)
        logs.create_audit_log(CREATE_BACKUP, operator_id=user["id"], target_type="backup", target_id=backup_id)
        shutil.copy2(source, backup_dir / DB_FILE_NAME)
        return _public_backup(record, manifest)
    except Exception as exc:
        logs.create_audit_log(CREATE_BACKUP, operator_id=user["id"], target_type="backup", target_id=backup_id, detail=str(exc), success=False)
        raise


def list_backups() -> list[dict]:
    return [_public_backup(record) for record in backups.list_backup_records()]


def restore_backup(backup_id: str, user: dict) -> dict:
    record = backups.get_backup_record(backup_id)
    if record is None:
        raise NotFound("backup not found")

    try:
        backup_dir = Path(record["path"])
        manifest = _read_manifest(backup_dir, backup_id)
        backup_db = backup_dir / manifest["database"]
        _validate_backup_db(backup_db)
        _replace_database(backup_db, get_settings().db_path)
    except Exception as exc:
        logs.create_audit_log(RESTORE_BACKUP, operator_id=user["id"], target_type="backup", target_id=backup_id, detail=str(exc), success=False)
        raise

    logs.create_audit_log(RESTORE_BACKUP, operator_id=user["id"], target_type="backup", target_id=backup_id)
    return _public_backup(record, manifest)


def _public_backup(record: dict, manifest: dict | None = None) -> dict:
    return {
        "backup_id": record["backup_id"],
        "created_at": record["created_at"],
        "path": record["path"],
        "manifest": manifest or record.get("manifest"),
    }


def _read_manifest(backup_dir: Path, backup_id: str) -> dict:
    manifest_path = backup_dir / MANIFEST_FILE_NAME
    if not manifest_path.exists():
        raise BadRequest("backup manifest is missing")
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise BadRequest("backup manifest is invalid") from exc
    if manifest.get("backup_id") != backup_id or manifest.get("storage") != "sqlite":
        raise BadRequest("backup manifest does not match")
    database_name = manifest.get("database")
    if not isinstance(database_name, str) or Path(database_name).name != database_name:
        raise BadRequest("backup manifest database is invalid")
    files = manifest.get("files")
    if not isinstance(files, list) or database_name not in files:
        raise BadRequest("backup manifest files are invalid")
    return manifest


def _validate_backup_db(backup_db: Path) -> None:
    if not backup_db.exists():
        raise BadRequest("backup database is missing")
    try:
        conn = sqlite3.connect(backup_db)
        try:
            result = conn.execute("PRAGMA integrity_check").fetchone()
            rows = conn.execute("SELECT name FROM sqlite_master WHERE type = 'table'").fetchall()
        finally:
            conn.close()
    except sqlite3.Error as exc:
        raise BadRequest("backup database is invalid") from exc
    if result is None or result[0] != "ok":
        raise BadRequest("backup database is corrupted")
    if not REQUIRED_TABLES.issubset({row[0] for row in rows}):
        raise BadRequest("backup database schema is invalid")


def _replace_database(backup_db: Path, current_db: Path) -> None:
    current_db.parent.mkdir(parents=True, exist_ok=True)
    restore_tmp = current_db.with_name(current_db.name + "." + new_id() + ".restore")
    shutil.copy2(backup_db, restore_tmp)
    try:
        _validate_backup_db(restore_tmp)
        restore_tmp.replace(current_db)
    finally:
        if restore_tmp.exists():
            restore_tmp.unlink()
