from __future__ import annotations

from pathlib import Path

from app.core.config import get_settings
from app.repositories import logs, problems, submissions, users
from app.repositories.database import initialize_database
from helper import create_finished_logged_submission, create_user, login_user


def test_data_persists_after_service_restart_and_backup_restore() -> None:
    """测试用户、题目、提交和日志重启后仍存在，备份恢复可还原被删除的数据"""
    admin_user = create_user("admin")
    student_user = create_user("student")
    finished = create_finished_logged_submission(student_user, problem_id="P_STEP6")
    student = login_user(student_user)

    # 完成一次提交并确认提交详情和测试点日志可查询
    detail_before_restart = student.get(f"/api/submissions/{finished['id']}")
    assert detail_before_restart.status_code == 200
    assert detail_before_restart.json()["data"]["status"] == "finished"
    logs_before_restart = student.get(f"/api/submissions/{finished['id']}/logs")
    assert logs_before_restart.status_code == 200
    assert len(logs_before_restart.json()["data"]) == 2
    # 关闭当前客户端并重新初始化数据库
    student.close()
    initialize_database(get_settings().db_path)
    restarted_student = login_user(student_user)
    # 重启后，用户、题目、提交和日志存在
    assert users.get_user(student_user["id"]) is not None
    assert problems.get_problem("P_STEP6", include_cases=True) is not None
    assert submissions.get_submission(finished["id"]) is not None
    assert len(logs.list_case_logs(finished["id"])) == 2
    assert restarted_student.get(f"/api/submissions/{finished['id']}").status_code == 200
    # 创建备份
    admin = login_user(admin_user)
    backup_response = admin.post("/api/admin/backups")
    assert backup_response.status_code == 201
    backup = backup_response.json()["data"]
    backup_id = backup["backup_id"]
    backup_path = Path(backup["path"])
    assert (backup_path / "manifest.json").exists()
    assert (backup_path / "oj.db").exists()
    # 删除题目和测试点日志
    logs.delete_case_logs(finished["id"])
    assert problems.delete_problem("P_STEP6") is True
    assert problems.get_problem("P_STEP6", include_cases=True) is None
    assert logs.list_case_logs(finished["id"]) == []
    # 恢复备份，被删除的题目和日志应恢复提交可查询
    restore_response = admin.post(f"/api/admin/backups/{backup_id}/restore")
    assert restore_response.status_code == 200
    assert problems.get_problem("P_STEP6", include_cases=True) is not None
    assert submissions.get_submission(finished["id"]) is not None
    assert len(logs.list_case_logs(finished["id"])) == 2
    assert admin.get(f"/api/submissions/{finished['id']}/logs").status_code == 200
    # 损坏同一备份数据库后恢复备份失败，且当前数据不被破坏
    (backup_path / "oj.db").write_text("not a sqlite database", encoding="utf-8")
    failed_restore = admin.post(f"/api/admin/backups/{backup_id}/restore")
    assert failed_restore.status_code == 400
    assert problems.get_problem("P_STEP6", include_cases=True) is not None
    assert submissions.get_submission(finished["id"]) is not None
    assert len(logs.list_case_logs(finished["id"])) == 2
