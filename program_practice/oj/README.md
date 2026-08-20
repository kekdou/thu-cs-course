# OJ System

一个小型但功能完善的 Online Judge 系统  

后端主要使用 python 3.10+、fastapi、pydantic 完成，使用 sqlite 存储，cookie session 持久化访问，docker 沙盒测试以及 pytest 自动化测试；前端使用 react、typescript、vite    

核心功能包括：
- 用户注册、登录、登出，支持 student, teacher, admin 三类用户
- 题目列表、题目详情、teacher/admin 题目 CRUD
- python 代码提交，创建提交后立即返回 submission_id，后台 docker 沙盒子进程测试
- 提交列表、条件筛选、提交详情和测试点日志查看
- 学生日志脱敏，teacher/admin 可查看完整日志并生成审计记录
- sqlite 持久化，admin 可创建备份和恢复备份

## 文件树

```text
oj/
├── app/
│   ├── main.py                 # fastapi 应用入口、路由注册和异常处理
│   ├── core/
│   │   ├── config.py           # 环境变量和运行目录配置
│   │   ├── errors.py
│   │   ├── responses.py
│   │   └── security.py
│   ├── judge/
│   │   ├── compare.py
│   │   ├── docker_runner.py
│   │   └── result.py
│   ├── models/
│   │   ├── backup.py
│   │   ├── enums.py
│   │   ├── log.py
│   │   ├── problem.py
│   │   ├── submission.py
│   │   └── user.py
│   ├── repositories/
│   │   ├── backups.py
│   │   ├── database.py         # sqlite 连接、建表和 default admin
│   │   ├── logs.py
│   │   ├── problems.py
│   │   ├── submissions.py
│   │   └── users.py
│   ├── routers/
│   │   ├── admin.py
│   │   ├── auth.py
│   │   ├── deps.py
│   │   ├── logs.py
│   │   ├── problems.py
│   │   ├── submissions.py
│   │   └── users.py
│   ├── services/
│   │   ├── auth_service.py
│   │   ├── backup_service.py
│   │   ├── log_service.py
│   │   ├── problem_service.py
│   │   ├── submission_service.py
│   │   └── user_service.py
│   └── utils/
│       ├── ids.py
│       ├── text.py
│       └── time.py
├── frontend/
│   ├── src/
│   │   ├── api/                # 前端 api client
│   │   ├── components/
│   │   ├── pages/
│   │   └── styles/
│   └── vite.config.ts
├── tests/
├── data/                       # 数据文件
├── backups/                    # 备份文件
├── temp/
├── report/
├── requirements.txt
└── pyproject.toml
```

## 环境准备

安装后端依赖：

```bash
pip install -r requirements.txt
```

安装前端依赖：

```bash
cd frontend
npm install
```

准备默认评测镜像：

```bash
docker pull python:3.10-slim
```

## 使用方法

启动后端：

```bash
uvicorn app.main:app --reload
```

后端默认地址为 `http://127.0.0.1:8000`，所有业务接口使用 `/api` 前缀，fastapi 自动文档位于 `/docs`

启动前端：

```bash
cd frontend
npm run dev
```

前端默认地址为 `http://127.0.0.1:5173`，vite 会将 `/api` 代理到 `http://127.0.0.1:8000`，前端请求会携带 cookie session

首次启动后端时会自动创建 default admin 账号，默认值为：

```text
username: admin
password: admin12345
```

正式运行可以使用环境变量覆盖默认账号：

```bash
OJ_ADMIN_USERNAME=admin \
OJ_ADMIN_PASSWORD='your_password' \
uvicorn app.main:app --reload
```

常用运行目录可通过环境变量调整：

```bash
OJ_DB_PATH=data/oj.db
OJ_BACKUP_DIR=backups
OJ_TEMP_DIR=temp
OJ_DOCKER_IMAGE=python:3.10-slim
OJ_DOCKER_CPUS=1.0
```

## 测评沙盒

评测器不会在 fastapi 主进程中执行学生代码，submission 创建后，后台任务调用 docker 沙盒运行 python 代码

当前沙盒边界：
- `--network none` 禁止容器网络访问
- `--memory` 和 `--memory-swap` 限制内存，内存超限返回 `MLE`
- `--cpus` 限制 cpu 配额
- `--read-only` 使用只读根文件系统
- `-v <temp>:/work:rw` 仅挂载本次评测工作目录
- `--tmpfs /tmp:rw,noexec,nosuid,size=16m` 提供受限临时目录
- `--pids-limit 64` 限制进程数量
- `--cap-drop ALL` 移除 linux capabilities
- `--security-opt no-new-privileges:true` 禁止提权
- docker 使用 `--rm`，外层超时会执行 `docker rm -f`，评测结束后删除本次临时目录

若 Docker 不可用、镜像缺失或容器启动失败，提交会返回 `SE` 并记录受控错误信息

## 自动化测试

运行全部测试：

```bash
pytest
```

运行测试遇到 `pytest` 找不到 `app`、导入路径不正确等问题时，可以在项目根目录运行

```bash
pip install -e .
```

也可以按模块运行：

```bash
pytest tests/test_problems.py
pytest tests/test_judge.py
pytest tests/test_users.py
pytest tests/test_submissions.py
pytest tests/test_logs.py
pytest tests/test_persistence_backup.py
pytest tests/test_sandbox_security.py
```

测试使用 `tests/conftest.py` 为每个用例创建独立数据库、临时目录、备份目录和 session 配置，部分评测和沙箱安全测试通过 monkeypatch 模拟 docker 行为
