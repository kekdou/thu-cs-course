import json
import os
import random
import re
import time
import requests

from config.ncss_config import (
    DEFAULT_HEADERS,
    LIST_API_URL,
)

from .utility import build_company_url, build_detail_url, build_list_params

PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTPUT_DIR = os.path.join(PROJECT_DIR, "output")
RAW_JSON_DIR = os.path.join(OUTPUT_DIR, "raw_json")
RAW_HTML_DIR = os.path.join(OUTPUT_DIR, "raw_html")
RAW_JOB_HTML_PATH = os.path.join(RAW_HTML_DIR, "job")
RAW_COMPANY_HTML_PATH = os.path.join(RAW_HTML_DIR, "company")


MAX_REQUEST_RETRIES = 4
LIST_DELAY = (5.0, 9.0)
DETAIL_DELAY = (3.0, 6.0)
REST_DELAY = (60.0, 120.0)


class RequestFailedTooManyTimes(Exception):
    pass


class NCSSDownloader:
    def __init__(self, cookie=""):
        self.session = requests.Session()
        self.session.headers.update(DEFAULT_HEADERS)
        if cookie:
            self.session.headers.update({"Cookie": cookie})

    def fetch_list_json(self, area_name, area_code, page_num):
        """
        接收取
        """
        params = build_list_params(area_code, page_num)
        response = self._get(LIST_API_URL, params=params)
        data = response.json()

        filepath = self._build_list_path(area_name, page_num)
        with open(filepath, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=2)

        self._sleep(*LIST_DELAY)
        return filepath

    def fetch_detail_html(self, job_id):
        url = build_detail_url(job_id)
        response = self._get(url)

        filepath = self._build_detail_path(job_id)
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(response.text)

        self._sleep(*DETAIL_DELAY)
        return filepath

    def fetch_company_html(self, company_id, company_href):
        url = build_company_url(company_id, company_href)
        if not url:
            return ""

        filepath = self._build_company_path(company_id)
        if os.path.exists(filepath):
            return filepath

        response = self._get(url)

        with open(filepath, "w", encoding="utf-8") as f:
            f.write(response.text)

        self._sleep(*DETAIL_DELAY)
        return filepath

    def rest(self):
        self._sleep(*REST_DELAY)

    def _build_list_path(self, area_name, page_num):
        safe_area_name = self._safe_filename(area_name)
        return os.path.join(RAW_JSON_DIR, f"ncss_{safe_area_name}_page_{page_num}.json")

    def _build_detail_path(self, job_id):
        safe_job_id = self._safe_filename(job_id)
        return os.path.join(RAW_JOB_HTML_PATH, f"ncss_detail_{safe_job_id}.html")

    def _build_company_path(self, company_id):
        safe_company_id = self._safe_filename(company_id)
        return os.path.join(
            RAW_COMPANY_HTML_PATH, f"ncss_company_{safe_company_id}.html"
        )

    def _safe_filename(self, value):
        value = str(value).strip()
        return re.sub(r"[^0-9a-zA-Z_\-\u4e00-\u9fff]+", "_", value)

    def _get(self, url, params=None):
        last_error = None
        for retry_count in range(1, MAX_REQUEST_RETRIES + 1):
            try:
                response = self.session.get(url, params=params, timeout=20)
                response.raise_for_status()
                return response
            except requests.RequestException as error:
                last_error = error
                print(f"request failed {retry_count}/{MAX_REQUEST_RETRIES}: {url}")
                print(error)
                if retry_count < MAX_REQUEST_RETRIES:
                    self._sleep(10.0, 20.0)

        raise RequestFailedTooManyTimes(
            f"same request failed {MAX_REQUEST_RETRIES} times: {url}"
        ) from last_error

    def _sleep(self, min_delay, max_delay):
        time.sleep(random.uniform(min_delay, max_delay))
