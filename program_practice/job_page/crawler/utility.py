import time
from urllib.parse import urljoin

from config.ncss_config import BASE_URL, DEFAULT_PARAMS


def build_list_params(area_code, page_num):
    params = DEFAULT_PARAMS.copy()
    params["areaCode"] = area_code
    params["offset"] = page_num
    params["_"] = int(time.time() * 1000)
    return params


def build_detail_url(job_id):
    return f"{BASE_URL}/student/jobs/{job_id}/detail.html"


def build_company_url(company_id, company_href):
    company_href = (company_href or "").strip()
    if company_href and not company_href.lower().startswith("javascript:"):
        return urljoin(BASE_URL, company_href)

    if company_id:
        return f"{BASE_URL}/student/jobs/{company_id}/corp.html"

    return ""
