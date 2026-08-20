## 项目介绍

招聘信息采集与检索系统  

目前数据来源 [国家大学生就业服务平台](https://www.ncss.cn/)，完成了数据爬取、mysql 存储、django web 展示和关键词搜索功能

## 文件树

```text
job_page/
├── README.md
├── main.py                         # 爬虫入口，爬取数据并写入 mysql
├── requirements.txt
├── config/
│   ├── db_config.json
│   └── ncss_config.py
├── crawler/
│   ├── downloader.py
│   ├── parser.py
│   ├── login.py
│   └── utility.py
├── database/
│   ├── db_client.py
│   └── sql_statements.py
├── script/                         # 数据分析脚本
│   ├── analysis_utils.py
│   ├── language_salary_compare.py
│   ├── salart_by_education.py
│   └── salary_by_region.py
├── example/                        # 网站 curl、html、json 示例
├── output/
│   ├── browser_data/               # 浏览器数据和 cookie 等
│   ├── raw_html/
│   └── raw_json/
└── web/
    ├── manage.py                   # web 入口
    ├── job_web/                    # django 项目配置
    └── jobs/
        ├── models.py
        ├── services.py 
        ├── views.py
        ├── templates/jobs/         # html 模板
        └── static/jobs/            # css 和 image
```

## 环境准备

下载依赖

```bash
pip install -r project/job_page/requirements.txt
playwright install chromium
```

数据库准备

```sql
CREATE DATABASE job_db;
```

修改配置文件 `/config/db_config.json`

```json
{
    "host": "localhost",
    "port": 3306,
    "user": "root",
    "password": "your_password",
    "database": "job_db",
    "charset": "utf8mb4"
}
```

## 爬取数据

爬取全部配置地区：

```bash
python main.py
```

爬取指定地区：

```bash
python main.py --city 北京
```

若环境中没有 cookie 文件，程序会自动打开浏览器，需要手动登录并保存 cookie

爬取过程中会保存原始数据：
- 列表 JSON：`output/raw_json/`
- 职位详情 HTML：`output/raw_html/job/`
- 公司详情 HTML：`output/raw_html/company/`

## Web 页面

进入 Django 项目并启动服务：

```bash
python web/manage.py runserver
```

浏览器访问：`http://127.0.0.1:8000/`

主要路由：

```text
/                 职位列表首页
/jobs/            职位列表
/jobs/<id>/       职位详情
/companies/       公司列表
/companies/<id>/  公司详情
/search/          搜索页面和搜索结果
```