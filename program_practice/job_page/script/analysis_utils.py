import json
import os
import re
from statistics import mean, median

import pymysql


BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DB_CONFIG_PATH = os.path.join(BASE_DIR, "config", "db_config.json")
ANALYSIS_OUTPUT_DIR = os.path.join(BASE_DIR, "output", "analysis")


REGION_PINYIN = {
    "北京": "Beijing",
    "天津": "Tianjin",
    "河北": "Hebei",
    "山西": "Shanxi",
    "内蒙": "Inner Mongolia",
    "辽宁": "Liaoning",
    "吉林": "Jilin",
    "黑龙": "Heilongjiang",
    "上海": "Shanghai",
    "江苏": "Jiangsu",
    "浙江": "Zhejiang",
    "安徽": "Anhui",
    "福建": "Fujian",
    "江西": "Jiangxi",
    "山东": "Shandong",
    "河南": "Henan",
    "湖北": "Hubei",
    "湖南": "Hunan",
    "广东": "Guangdong",
    "广西": "Guangxi",
    "海南": "Hainan",
    "重庆": "Chongqing",
    "四川": "Sichuan",
    "贵州": "Guizhou",
    "云南": "Yunnan",
    "西藏": "Tibet",
    "陕西": "Shaanxi",
    "甘肃": "Gansu",
    "青海": "Qinghai",
    "宁夏": "Ningxia",
    "新疆": "Xinjiang",
    "香港": "Hong Kong",
    "澳门": "Macau",
    "台湾": "Taiwan",
}


def get_connection():
    with open(DB_CONFIG_PATH, "r", encoding="utf-8") as f:
        config = json.load(f)

    return pymysql.connect(
        host=config["host"],
        port=config["port"],
        user=config["user"],
        password=config["password"],
        database=config["database"],
        charset=config["charset"],
        cursorclass=pymysql.cursors.DictCursor,
    )


def fetch_jobs():
    sql = """
    SELECT
        title,
        city,
        salary,
        education,
        tags,
        description
    FROM jobs
    """

    connection = get_connection()
    try:
        with connection.cursor() as cursor:
            cursor.execute(sql)
            return cursor.fetchall()
    finally:
        connection.close()


def parse_salary(salary_text):
    if not salary_text:
        return None

    text = salary_text.lower().replace(" ", "")
    numbers = re.findall(r"(\d+(?:\.\d+)?)k", text)

    if len(numbers) >= 2:
        return (float(numbers[0]) + float(numbers[1])) / 2
    if len(numbers) == 1:
        return float(numbers[0])
    return None


def get_region(city):
    if not city:
        return None

    city = city.strip()
    if len(city) < 2:
        return None
    return city[:2]


def chinese_region_to_pinyin(region):
    if not region:
        return None
    return REGION_PINYIN.get(region)


def get_education_group(education):
    if not education:
        return "No Limit"

    text = education.strip()
    if "硕士" in text or "研究生" in text:
        return "Master"
    if "本科" in text:
        return "Bachelor"
    if "专科" in text or "大专" in text:
        return "Junior College"
    return "No Limit"


def get_language_groups(job):
    text = " ".join(
        [
            job.get("title") or "",
            job.get("tags") or "",
            job.get("description") or "",
        ]
    ).lower()

    rules = {
        "Python": r"python",
        "Java": r"java(?!script)",
        "C++": r"c\+\+|cpp",
        "JavaScript": r"javascript|\bjs\b",
        "Go": r"\bgolang\b|\bgo\b|go语言",
        "PHP": r"\bphp\b",
        "SQL": r"\bsql\b|mysql|postgresql|sqlserver",
    }

    result = []
    for language, pattern in rules.items():
        if re.search(pattern, text):
            result.append(language)
    return result


def add_to_group(groups, key, value):
    if key not in groups:
        groups[key] = []
    groups[key].append(value)


def build_stats(values):
    if not values:
        return {
            "count": 0,
            "average": 0,
            "median": 0,
        }

    return {
        "count": len(values),
        "average": mean(values),
        "median": median(values),
    }


def print_table(title, rows, headers):
    print(f"\n{title}")
    print("-" * 60)
    print("\t".join(headers))
    for row in rows:
        print("\t".join(str(item) for item in row))
