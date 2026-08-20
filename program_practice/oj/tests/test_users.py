from __future__ import annotations

from helper import PASSWORD, client, create_user, login_as, register_payload


def test_register_success_duplicate_username_and_short_password() -> None:
    """测试注册成功、重复用户名返回 409、密码过短返回 422"""
    api = client()
    body = register_payload()
    # 正常注册，role 为 student
    created = api.post("/api/auth/register", json=body)
    assert created.status_code == 201
    assert created.json()["data"]["username"] == body["username"]
    assert created.json()["data"]["role"] == "student"
    # 重复 username
    duplicate = api.post("/api/auth/register", json=body)
    assert duplicate.status_code == 409
    # 不符要求的 password
    short_password = api.post("/api/auth/register", json=register_payload(password="short"))
    assert short_password.status_code == 422


def test_login_success_failure_and_logout_protection() -> None:
    """测试登录成功、错误密码登录失败、登出后无法访问受保护接口"""
    api = client()
    body = register_payload()
    assert api.post("/api/auth/register", json=body).status_code == 201
    # 密码错误
    failed = api.post("/api/auth/login", json={**body, "password": "wrongpass"})
    assert failed.status_code == 401
    # 正常登录
    logged_in = api.post("/api/auth/login", json=body)
    assert logged_in.status_code == 200
    # 正常访问主页
    me_before_logout = api.get("/api/auth/me")
    assert me_before_logout.status_code == 200
    # 登出
    logged_out = api.post("/api/auth/logout")
    assert logged_out.status_code == 200
    # session 失效，未登录访问主页
    me_after_logout = api.get("/api/auth/me")
    assert me_after_logout.status_code == 401


def test_role_permissions_for_teacher_and_admin_interfaces() -> None:
    """测试学生调用教师接口返回 403，教师调用管理员接口返回 403"""
    student = login_as("student")
    teacher = login_as("teacher")
    # student 访问创建题目
    teacher_api_response = student.post("/api/problems", json={})
    assert teacher_api_response.status_code == 403
    # teacher 访问用户管理
    admin_api_response = teacher.get("/api/users")
    assert admin_api_response.status_code == 403


def test_admin_can_update_user_role() -> None:
    """测试管理员可以修改普通用户角色，并且响应中返回更新后的公开用户信息"""
    admin = login_as("admin")
    user = create_user("student")
    # 修改 role
    response = admin.put(f"/api/users/{user['id']}", json={"role": "teacher", "is_active": True})
    assert response.status_code == 200
    assert response.json()["data"]["id"] == user["id"]
    assert response.json()["data"]["role"] == "teacher"
    # 没有返回 password
    assert "password" not in response.text.lower()


def test_disabled_user_cannot_login() -> None:
    """测试管理员禁用用户后，该用户无法再次登录"""
    admin = login_as("admin")
    user = create_user("student")
    # 修改 is_active
    disabled = admin.put(f"/api/users/{user['id']}", json={"role": "student", "is_active": False})
    assert disabled.status_code == 200
    assert disabled.json()["data"]["is_active"] is False
    # 禁用后无法登录
    login_response = client().post("/api/auth/login", json={"username": user["username"], "password": PASSWORD})
    assert login_response.status_code == 403


def test_user_interfaces_do_not_return_password_or_hash() -> None:
    """测试注册、登录、当前用户和管理员用户列表接口不返回密码或密码哈希"""
    api = client()
    body = register_payload()
    # 注册响应
    register_response = api.post("/api/auth/register", json=body)
    assert register_response.status_code == 201
    # 登录响应
    login_response = api.post("/api/auth/login", json=body)
    assert login_response.status_code == 200
    # 主页响应
    me_response = api.get("/api/auth/me")
    assert me_response.status_code == 200
    # 获取用户列表响应
    admin = login_as("admin")
    list_response = admin.get("/api/users")
    assert list_response.status_code == 200
    # 检查没有明文密码或者哈希密码
    response_text = "\n".join(
        [
            register_response.text,
            login_response.text,
            me_response.text,
            list_response.text,
        ]
    ).lower()
    assert body["password"] not in response_text
    assert "password" not in response_text
    assert "password_hash" not in response_text
