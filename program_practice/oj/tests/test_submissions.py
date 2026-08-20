from __future__ import annotations

import asyncio

from app.models.enums import JudgeResult
from app.repositories import logs, submissions
from app.services import submission_service
from helper import (
    CORRECT_CODE,
    RUNTIME_ERROR_CODE,
    TIME_LIMIT_CODE,
    WRONG_ANSWER_CODE,
    create_problem,
    create_user,
    judge_result,
    login_as,
    login_user,
)


async def noop_evaluate(_submission_id: str) -> None:
    """只创建任务，不在执行真实后台评测"""


def test_create_submission_returns_submission_id_immediately(monkeypatch) -> None:
    """测试创建提交立即返回 202 和 submission_id，评测由后台任务处理"""
    monkeypatch.setattr(submission_service, "evaluate_submission", noop_evaluate)
    problem = create_problem()
    student = login_as("student")
    # 成功创建 submission
    response = student.post(
        "/api/submissions",
        json={"problem_id": problem["id"], "language": "python", "source_code": CORRECT_CODE},
    )
    assert response.status_code == 202
    assert response.json()["data"]["id"]
    assert response.json()["data"]["submission_id"] == response.json()["data"]["id"]


def test_submission_status_changes_in_legal_order() -> None:
    """测试提交状态按 pending -> running -> finished/failed 的合法顺序变化"""
    problem = create_problem()
    user = create_user("student")
    # 提交 submission 并且 status 为 pending
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    assert submission["status"] == "pending"
    assert submission["result"] is None
    # 运行代码，并且 status 为 running
    running = submissions.mark_running(submission["id"])
    assert running is not None and running["status"] == "running"
    assert running["result"] is None
    # 运行完毕，status 为 finished
    finished = submissions.finish_submission(submission["id"], JudgeResult.AC.value, 100, 0.01)
    assert finished is not None and finished["status"] == "finished"
    assert finished["result"] == JudgeResult.AC.value
    # 运行代码出错，返回 SE 并且状态为 failed
    failed_submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    failed_running = submissions.mark_running(failed_submission["id"])
    assert failed_running is not None and failed_running["status"] == "running"
    failed = submissions.finish_submission(failed_submission["id"], JudgeResult.SE.value, 0, 0.0, failed=True)
    assert failed is not None and failed["status"] == "failed"
    assert failed["result"] == JudgeResult.SE.value


def test_submission_evaluation_results_match_step2(monkeypatch) -> None:
    """测试提交服务保存的评测结果与 Step2 中 AC、WA、RE、TLE 的判断一致"""
    async def fake_judge(source_code: str, *_args) -> dict:
        if source_code == CORRECT_CODE:
            return judge_result(JudgeResult.AC.value, 100)
        if source_code == WRONG_ANSWER_CODE:
            return judge_result(JudgeResult.WA.value, 0)
        if source_code == RUNTIME_ERROR_CODE:
            return judge_result(JudgeResult.RE.value, 0)
        if source_code == TIME_LIMIT_CODE:
            return judge_result(JudgeResult.TLE.value, 0)
        return judge_result(JudgeResult.SE.value, 0)

    monkeypatch.setattr(submission_service, "judge_python", fake_judge)
    problem = create_problem()
    user = create_user("student")
    cases = {
        CORRECT_CODE: JudgeResult.AC.value,
        WRONG_ANSWER_CODE: JudgeResult.WA.value,
        RUNTIME_ERROR_CODE: JudgeResult.RE.value,
        TIME_LIMIT_CODE: JudgeResult.TLE.value,
    }

    for source_code, expected_result in cases.items():
        submission = submissions.create_submission(user["id"], problem["id"], "python", source_code)
        asyncio.run(submission_service.evaluate_submission(submission["id"]))
        updated = submissions.get_submission(submission["id"])
        assert updated is not None
        assert updated["status"] == "finished"
        assert updated["result"] == expected_result


def test_student_can_view_own_submission_but_not_other_students_submission() -> None:
    """测试学生可查看自己的提交，不能查看其他学生的提交详情或列表结果"""
    problem = create_problem()
    user_a = create_user("student")
    user_b = create_user("student")
    client_a = login_user(user_a)
    client_b = login_user(user_b)
    own = submissions.create_submission(user_a["id"], problem["id"], "python", CORRECT_CODE)
    other = submissions.create_submission(user_b["id"], problem["id"], "python", WRONG_ANSWER_CODE)
    # 查看自己的提交详情
    own_detail = client_a.get(f"/api/submissions/{own['id']}")
    assert own_detail.status_code == 200
    # 查看他人的提交详情
    other_detail = client_a.get(f"/api/submissions/{other['id']}")
    assert other_detail.status_code == 403
    # 查看自己的提交列表和他人的提交列表
    own_listing = client_a.get("/api/submissions")
    hidden_listing = client_b.get("/api/submissions", params={"user_id": user_a["id"]})
    own_ids = {item["id"] for item in own_listing.json()["data"]["items"]}
    assert own["id"] in own_ids
    assert other["id"] not in own_ids
    hidden_ids = {item["id"] for item in hidden_listing.json()["data"]["items"]}
    assert other["id"] in hidden_ids
    assert own["id"] not in hidden_ids


def test_teacher_can_filter_all_submissions() -> None:
    """测试教师可以按题目和结果筛选全部学生提交"""
    teacher = login_as("teacher")
    problem_a = create_problem()
    problem_b = create_problem()
    user_a = create_user("student")
    user_b = create_user("student")
    ac = submissions.create_submission(user_a["id"], problem_a["id"], "python", CORRECT_CODE)
    wa = submissions.create_submission(user_b["id"], problem_b["id"], "python", WRONG_ANSWER_CODE)
    submissions.mark_running(ac["id"])
    submissions.finish_submission(ac["id"], JudgeResult.AC.value, 100, 0.01)
    submissions.mark_running(wa["id"])
    submissions.finish_submission(wa["id"], JudgeResult.WA.value, 0, 0.01)

    response = teacher.get("/api/submissions", params={"problem_id": problem_b["id"], "result": JudgeResult.WA.value})
    ids = {item["id"] for item in response.json()["data"]["items"]}
    assert response.status_code == 200
    assert wa["id"] in ids
    assert ac["id"] not in ids


def test_missing_problem_and_empty_code_cannot_be_submitted(monkeypatch) -> None:
    """测试不存在的题目无法提交，空白源码也无法提交"""
    monkeypatch.setattr(submission_service, "evaluate_submission", noop_evaluate)
    problem = create_problem()
    student = login_as("student")
    # 测试不存在的题目
    missing_problem = student.post(
        "/api/submissions",
        json={"problem_id": "P_NOT_EXISTS", "language": "python", "source_code": CORRECT_CODE},
    )
    assert missing_problem.status_code == 404
    # 提交空代码
    empty_code = student.post(
        "/api/submissions",
        json={"problem_id": problem["id"], "language": "python", "source_code": "   \n"},
    )
    assert empty_code.status_code == 422


def test_finished_submission_cannot_be_changed_back_to_running() -> None:
    """测试已完成提交不能通过 running 状态更新接口改回 running"""
    problem = create_problem()
    user = create_user("student")
    submission = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    submissions.mark_running(submission["id"])
    submissions.finish_submission(submission["id"], JudgeResult.AC.value, 100, 0.01)

    rerun = submissions.mark_running(submission["id"])
    assert rerun is not None
    assert rerun["status"] == "finished"
    assert rerun["result"] == JudgeResult.AC.value


def test_rejudge_permission_and_status_checks(monkeypatch) -> None:
    """测试重新评测要求教师权限，且只能重评 finished/failed 提交"""
    monkeypatch.setattr(submission_service, "evaluate_submission", noop_evaluate)
    problem = create_problem()
    user = create_user("student")
    student = login_user(user)
    teacher = login_as("teacher")
    pending = submissions.create_submission(user["id"], problem["id"], "python", CORRECT_CODE)
    # student 无法 rejudge
    student_forbidden = student.post(f"/api/submissions/{pending['id']}/rejudge")
    assert student_forbidden.status_code == 403
    # pending 时无法 rejudge
    pending_conflict = teacher.post(f"/api/submissions/{pending['id']}/rejudge")
    assert pending_conflict.status_code == 409
    # 成功 rejudge
    submissions.mark_running(pending["id"])
    submissions.finish_submission(pending["id"], JudgeResult.AC.value, 100, 0.01)
    accepted = teacher.post(f"/api/submissions/{pending['id']}/rejudge")
    assert accepted.status_code == 202
    assert accepted.json()["data"]["status"] == "pending"
    # rejudge 后的 pending 依然无法 pending
    conflict_after_reset = teacher.post(f"/api/submissions/{pending['id']}/rejudge")
    assert conflict_after_reset.status_code == 409
    # 写入了审计记录
    audit_items, _total = logs.list_audit_logs(action="REJUDGE_SUBMISSION", target_id=pending["id"])
    assert audit_items
