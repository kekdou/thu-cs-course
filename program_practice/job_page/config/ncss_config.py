BASE_URL = "https://www.ncss.cn"
LIST_API_URL = f"{BASE_URL}/student/jobs/jobslist/ajax/"
SEARCH_PAGE_URL = f"{BASE_URL}/student/jobs/index.html"

DEFAULT_HEADERS = {
    "Accept": "*/*",
    "Accept-Language": "zh-CN,zh;q=0.9,en;q=0.8",
    "Connection": "keep-alive",
    "DNT": "1",
    "Referer": SEARCH_PAGE_URL,
    "User-Agent": (
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/126.0.0.0 Safari/537.36"
    ),
    "X-Requested-With": "XMLHttpRequest",
}

DEFAULT_PARAMS = {
    "jobType": "",
    "areaCode": "",
    "jobName": "",
    "monthPay": "",
    "industrySectors": "",
    "property": "",
    "categoryCode": "",
    "memberLevel": "",
    "recruitType": "",
    "offset": 1,
    "limit": 10,
    "keyUnits": "",
    "degreeCode": "",
    "sourcesName": "0",
    "sourcesType": "",
}

AREA_CODES = {
    "北京": "11",
    "天津": "12",
    "河北": "13",
    "上海": "31",
    "江苏": "32",
    "浙江": "33",
    "福建": "35",
    "山东": "37",
    "湖北": "42",
    "湖南": "43",
    "广东": "44",
    "四川": "51",
    "江西": "36"
}
