from __future__ import annotations

import asyncio

from app.core.config import get_settings
from app.models.enums import JudgeResult
from app.repositories import logs, submissions
from app.repositories.database import initialize_database
from app.services import submission_service
from helper import CORRECT_CODE, add_case_log, create_logged_submission, create_problem, create_user, login_as, login_user


def test_student_can_view_own_logs_but_not_other_students_logs() -> None:
    """测试学生可以查看自己的提交日志，访问他人日志返回 403"""
    owner = create_user("student")
    other = create_user("student")
    submission = create_logged_submission(owner)
    # 成功访问自己的 log
    own_response = login_user(owner).get(f"/api/submissions/{submission['id']}/logs")
    assert own_response.status_code == 200
    assert len(own_response.json()["data"]) == 2
    # 无法访问其他人的 log
    other_response = login_user(other).get(f"/api/submissions/{submission['id']}/logs")
    assert other_response.status_code == 403


def test_student_log_view_hides_hidden_case_sensitive_fields() -> None:
    """测试学生看不到隐藏输入、隐藏标准答案和隐藏实际输出"""
    user = create_user("student")
    submission = create_logged_submission(user)

    response = login_user(user).get(f"/api/submissions/{submission['id']}/logs")
    items = {item["case_id"]: item for item in response.json()["data"]}
    # is_hidden = False 部分可以访问
    assert "stdout" in items["case_01"]
    assert "expected_output" in items["case_01"]
    assert "input_data" not in items["case_01"]
    # is_hidden = True 部分不可访问
    assert "stdout" not in items["case_02"]
    assert "expected_output" not in items["case_02"]
    assert "input_data" not in items["case_02"]


def test_teacher_full_log_view_contains_complete_case_fields_and_audit() -> None:
    """测试教师可以查看完整日志字段，且查看后写入审计记录"""
    user = create_user("student")
    submission = create_logged_submission(user)
    teacher = login_as("teacher")
    # 教师访问 log
    response = teacher.get(f"/api/submissions/{submission['id']}/logs")
    assert response.status_code == 200
    # 可以访问所有字段
    item = response.json()["data"][0]
    assert item["submission_id"] == submission["id"]
    assert item["input_data"] == "1 2\n"
    assert item["expected_output"] == "3\n"
    assert item["exit_code"] == 0
    assert item["created_at"]
    # 查看的动作写入审计
    audit_items, _total = logs.list_audit_logs(action="VIEW_FULL_JUDGE_LOG", target_id=submission["id"])
    assert audit_items


def test_teacher_log_search_supports_filters() -> None:
    """测试教师日志检索接口支持 problem_id、user_id 和 result 筛选"""
    user = create_user("student")
    submission = create_logged_submission(user)
    teacher = login_as("teacher")
    response = teacher.get(
        "/api/logs",
        params={
            "problem_id": submission["problem_id"],
            "user_id": user["id"],
            "result": JudgeResult.AC.value,
        },
    )
    assert response.status_code == 200

    items = response.json()["data"]["items"]
    assert items
    assert all(item["problem_id"] == submission["problem_id"] for item in items)
    assert all(item["user_id"] == user["id"] for item in items)
    assert all(item["result"] == JudgeResult.AC.value for item in items)


def test_long_outputs_are_truncated_before_response() -> None:
    """测试超长 input、stdout、stderr、expected_output 和 message 会被截断并带标记"""
    user = create_user("student")
    problem = create_problem()
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    long_text = "x" * 4100

    stored = logs.create_case_log(
        {
            "submission_id": submission["id"],
            "case_id": "case_long",
            "result": JudgeResult.WA.value,
            "score": 0,
            "time_used": 0.01,
            "memory_used": None,
            "exit_code": 0,
            "stdout": long_text,
            "stderr": long_text,
            "input_data": long_text,
            "expected_output": long_text,
            "is_hidden": 0,
            "message": long_text,
        }
    )
    assert stored["stdout"].endswith("...[truncated]")
    assert stored["stderr"].endswith("...[truncated]")
    assert stored["input_data"].endswith("...[truncated]")
    assert stored["expected_output"].endswith("...[truncated]")
    assert stored["message"].endswith("...[truncated]")


def test_student_error_messages_hide_paths_and_full_tracebacks() -> None:
    """测试学生日志中的 Linux、Windows 路径会脱敏，完整 traceback 不会原样暴露"""
    user = create_user("student")
    problem = create_problem()
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    traceback_text = (
        "Traceback (most recent call last):\n"
        '  File "/home/server/oj/temp/abc/main.py", line 3, in <module>\n'
        "ZeroDivisionError: division by zero\n"
        r"C:\oj\temp\abc\main.py"
    )
    add_case_log(submission["id"], result=JudgeResult.RE.value, stderr=traceback_text, message=traceback_text)

    item = login_user(user).get(f"/api/submissions/{submission['id']}/logs").json()["data"][0]
    assert "/home/server/oj/temp" not in item["stderr"]
    assert r"C:\oj\temp" not in item["stderr"]
    assert "Traceback (most recent call last):" not in item["stderr"]
    assert "程序第 3 行发生运行错误" in item["stderr"]


def test_wa_re_tle_and_se_case_logs_are_returned_correctly() -> None:
    """测试 WA、RE、TLE、SE 四类测试点日志结果和消息可以正确返回"""
    user = create_user("student")
    problem = create_problem()
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    for result in (JudgeResult.WA.value, JudgeResult.RE.value, JudgeResult.TLE.value, JudgeResult.SE.value):
        add_case_log(submission["id"], case_id=f"case_{result}", result=result, message=f"{result} message")
    items = login_user(user).get(f"/api/submissions/{submission['id']}/logs").json()["data"]

    by_result = {item["result"]: item for item in items}
    assert by_result[JudgeResult.WA.value]["message"] == "WA message"
    assert by_result[JudgeResult.RE.value]["message"] == "RE message"
    assert by_result[JudgeResult.TLE.value]["message"] == "TLE message"
    assert by_result[JudgeResult.SE.value]["message"] == "System Error"


def test_admin_can_filter_audit_logs() -> None:
    """测试管理员可以按 action、operator_id 和 target_id 筛选审计日志"""
    admin_user = create_user("admin")
    target_id = "submission_for_audit"
    logs.create_audit_log("VIEW_FULL_JUDGE_LOG", operator_id=admin_user["id"], target_type="submission", target_id=target_id)
    # 查询审计日志
    response = login_user(admin_user).get(
        "/api/audit-logs",
        params={"action": "VIEW_FULL_JUDGE_LOG", "operator_id": admin_user["id"], "target_id": target_id},
    )
    assert response.status_code == 200
    # 筛选成功
    items = response.json()["data"]["items"]
    assert len(items) == 1
    assert items[0]["operator_id"] == admin_user["id"]
    assert items[0]["target_id"] == target_id


def test_logs_persist_after_database_reinitialization() -> None:
    """测试数据库重新初始化后，已保存的测试点日志仍然可以查询"""
    user = create_user("student")
    submission = create_logged_submission(user)
    initialize_database(get_settings().db_path)

    response = login_user(user).get(f"/api/submissions/{submission['id']}/logs")
    assert response.status_code == 200
    assert len(response.json()["data"]) == 2


def test_judge_exception_writes_system_error_case_log(monkeypatch) -> None:
    """测试评测器异常时，提交失败且写入 SE system 测试点日志"""
    async def fail_judge(*_args, **_kwargs) -> dict:
        """模拟评测器内部异常"""
        raise RuntimeError("judge crashed")

    monkeypatch.setattr(submission_service, "judge_python", fail_judge)
    user = create_user("student")
    problem = create_problem()
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)

    asyncio.run(submission_service.evaluate_submission(submission["id"]))
    case_logs = logs.list_case_logs(submission["id"])
    updated = submissions.get_submission(submission["id"])

    assert updated is not None
    assert updated["status"] == "failed"
    assert updated["result"] == JudgeResult.SE.value
    assert len(case_logs) == 1
    assert case_logs[0]["case_id"] == "system"
    assert case_logs[0]["result"] == JudgeResult.SE.value
