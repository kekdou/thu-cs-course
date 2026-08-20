from __future__ import annotations

from app.core.config import get_settings, reset_settings_cache
from app.repositories.database import initialize_database
from helper import login_as, problem_payload


def test_create_duplicate_invalid_list_update_delete_problem() -> None:
    """测试教师创建题目、重复编号、非法字段、列表查询、修改和删除流程"""
    teacher = login_as("teacher")
    body = problem_payload()
    # 创建题目
    created = teacher.post("/api/problems", json=body)
    assert created.status_code == 201
    # 重复 id
    duplicate = teacher.post("/api/problems", json=body)
    assert duplicate.status_code == 409
    # 缺少字段，以 title 为例
    missing_title = problem_payload().pop("title")
    missing_response = teacher.post("/api/problems", json=missing_title)
    assert missing_response.status_code == 422
    # 分数不匹配
    bad_score = problem_payload()
    bad_score["test_cases"][1]["score"] = 40
    bad_score_response = teacher.post("/api/problems", json=bad_score)
    assert bad_score_response.status_code == 422
    # 查询题目列表
    listing = teacher.get("/api/problems")
    assert listing.status_code == 200
    assert any(item["id"] == body["id"] for item in listing.json()["data"]["items"])
    # 查询不存在题目
    missing = teacher.get("/api/problems/P_NOT_EXISTS")
    assert missing.status_code == 404
    # 修改题目
    updated_body = {**body, "title": "Updated A+B Problem"}
    updated = teacher.put(f"/api/problems/{body['id']}", json=updated_body)
    assert updated.status_code == 200
    assert updated.json()["data"]["title"] == "Updated A+B Problem"
    # 删除题目
    deleted = teacher.delete(f"/api/problems/{body['id']}")
    assert deleted.status_code == 200
    after_delete = teacher.get(f"/api/problems/{body['id']}")
    assert after_delete.status_code == 404


def test_student_hidden_cases_and_mutation_permissions() -> None:
    """测试学生题目详情不暴露隐藏测试点，并且无权创建、修改、删除题目"""
    teacher = login_as("teacher")
    student = login_as("student")
    body = problem_payload()
    assert teacher.post("/api/problems", json=body).status_code == 201
    detail = student.get(f"/api/problems/{body['id']}")
    assert detail.status_code == 200
    # 隐藏测试点不返回
    assert "test_cases" not in detail.json()["data"]
    assert "case_02" not in detail.text
    assert "1\n" not in detail.text
    # 学生无权创建、修改、删除题目
    assert student.post("/api/problems", json=problem_payload()).status_code == 403
    assert student.put(f"/api/problems/{body['id']}", json=body).status_code == 403
    assert student.delete(f"/api/problems/{body['id']}").status_code == 403


def test_problem_persists_after_service_restart() -> None:
    """测试题目写入持久化数据源后，重新初始化服务仍可查询"""
    teacher = login_as("teacher")
    body = problem_payload()
    assert teacher.post("/api/problems", json=body).status_code == 201

    db_path = get_settings().db_path
    reset_settings_cache()
    initialize_database(db_path)

    next_teacher = login_as("teacher")
    detail = next_teacher.get(f"/api/problems/{body['id']}")

    assert detail.status_code == 200
    assert detail.json()["data"]["id"] == body["id"]
