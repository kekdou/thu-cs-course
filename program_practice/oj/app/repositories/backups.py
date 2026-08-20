from __future__ import annotations

import json
from pathlib import Path

from app.repositories.database import row_to_dict, transaction
from app.utils.ids import new_id
from app.utils.time import utc_now


def create_backup_record(backup_id: str, backup_path: Path, manifest: dict, path: Path | None = None) -> dict:
    record = {
        "id": new_id(),
        "backup_id": backup_id,
        "path": str(backup_path),
        "manifest_json": json.dumps(manifest, ensure_ascii=False),
        "created_at": utc_now(),
    }
    with transaction(path) as conn:
        conn.execute(
            """
            INSERT INTO backups (id, backup_id, path, manifest_json, created_at)
            VALUES (:id, :backup_id, :path, :manifest_json, :created_at)
            """,
            record,
        )
    return record


def get_backup_record(backup_id: str, path: Path | None = None) -> dict | None:
    with transaction(path) as conn:
        row = conn.execute("SELECT * FROM backups WHERE backup_id = ?", (backup_id,)).fetchone()
    record = row_to_dict(row)
    if record is not None:
        record["manifest"] = json.loads(record.pop("manifest_json"))
    return record


def list_backup_records(path: Path | None = None) -> list[dict]:
    with transaction(path) as conn:
        rows = conn.execute("SELECT * FROM backups ORDER BY created_at DESC").fetchall()
    records = []
    for row in rows:
        record = dict(row)
        record["manifest"] = json.loads(record.pop("manifest_json"))
        records.append(record)
    return records
