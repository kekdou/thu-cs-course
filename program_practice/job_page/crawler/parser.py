import json
import re
from datetime import datetime

from bs4 import BeautifulSoup

from .utility import build_detail_url


class NCSSParser:
    SOURCE_SITE = "ncss"

    def parse_list_json(self, filepath):
        with open(filepath, "r", encoding="utf-8") as f:
            data = json.load(f)

        if not isinstance(data, dict):
            print(f"list json format is invalid: {filepath}")
            return []

        data_body = data.get("data") or {}
        if not isinstance(data_body, dict):
            print(f"list json data is empty or invalid: {filepath}")
            return []

        return data_body.get("list") or []

    def parse_job(self, job_item, detail_data=None, company_summary=""):
        detail_data = detail_data or {}
        company_data = self._parse_company(job_item, detail_data, company_summary)
        job_data = self._parse_job(job_item, detail_data)
        return company_data, job_data

    def parse_detail_html(self, filepath):
        with open(filepath, "r", encoding="utf-8") as f:
            html_text = f.read()
        soup = BeautifulSoup(html_text, "lxml")
        return {
            "title": self._get_text(soup.select_one("#jobName")),
            "description": self._get_text(
                soup.select_one(".jobdetail-box .mainContent"), separator="\n"
            ),
            "company_name": self._get_text(soup.select_one("#realCorpName")),
            "company_id": self._get_text(soup.select_one("#corpId")),
            "company_logo": self._get_attr(soup.select_one(".corp-img"), "src"),
            "company_href": self._get_attr(soup.select_one(".corp-head"), "href"),
            "industry": self._get_text(soup.select_one("#mainindustries")),
            "scale": self._get_company_detail(soup, "公司规模"),
            "property": self._get_company_detail(soup, "公司性质"),
            "company_address": self._get_text(soup.select_one("#companyNameMap")),
            "city": self._get_text(soup.select_one(".site-tag")),
        }

    def parse_company_summary(self, filepath):
        with open(filepath, "r", encoding="utf-8") as f:
            html_text = f.read()
        soup = BeautifulSoup(html_text, "lxml")
        return self._get_text(soup.select_one(".company-prife"), separator="\n")

    def _parse_company(self, job_item, detail_data, company_summary):
        source_company_id = detail_data.get("company_id") or job_item.get("recId")
        company_name = detail_data.get("company_name") or job_item.get("recName") or ""
        if source_company_id == "-":
            source_company_id = f"name:{company_name}" if company_name else ""

        industry = detail_data.get("industry") or ""
        property_text = detail_data.get("property") or job_item.get("recProperty") or ""
        if industry and property_text:
            industry = f"{industry} / {property_text}"
        else:
            industry = industry or property_text

        return {
            "source_site": self.SOURCE_SITE,
            "source_company_id": source_company_id,
            "name": company_name,
            "summary": company_summary,
            "logo_url": detail_data.get("company_logo")
            or job_item.get("recLogo")
            or "",
            "industry": industry,
            "scale": detail_data.get("scale") or job_item.get("recScale") or "",
        }

    def _parse_job(self, job_item, detail_data):
        job_id = job_item.get("jobId") or ""
        description = detail_data.get("description") or ""
        extra_lines = self._parse_extra_description(job_item)
        if extra_lines:
            description = f"{description}\n\n{extra_lines}".strip()

        return {
            "source_site": self.SOURCE_SITE,
            "source_job_id": job_id,
            "title": detail_data.get("title") or job_item.get("jobName") or "",
            "city": detail_data.get("city") or job_item.get("areaCodeName") or "",
            "salary": self._format_salary(job_item),
            "experience": "",
            "education": job_item.get("degreeName") or "",
            "tags": self._format_tags(job_item),
            "description": description,
            "job_url": build_detail_url(job_id) if job_id else "",
            "published_time": self._format_timestamp(job_item.get("publishDate")),
        }

    def _parse_extra_description(self, job_item):
        lines = []
        if job_item.get("headCount") not in ("", None):
            lines.append(f"招聘人数: {job_item.get('headCount')}")
        if job_item.get("major"):
            lines.append(f"专业要求: {job_item.get('major')}")
        if job_item.get("recProperty"):
            lines.append(f"公司性质: {job_item.get('recProperty')}")
        return "\n".join(lines)

    def _format_salary(self, job_item):
        low = job_item.get("lowMonthPay")
        high = job_item.get("highMonthPay")
        if low in ("", None) and high in ("", None):
            return ""
        if low in ("", None):
            return f"{self._format_number(high)}k"
        if high in ("", None):
            return f"{self._format_number(low)}k"
        return f"{self._format_number(low)}k-{self._format_number(high)}k"

    def _format_tags(self, job_item):
        parts = []
        if job_item.get("recTags"):
            parts.append(str(job_item.get("recTags")).strip())
        if job_item.get("major"):
            parts.append(f"专业: {str(job_item.get('major')).strip()}")
        return "；".join(parts)

    def _format_timestamp(self, value):
        if not value:
            return ""
        try:
            return datetime.fromtimestamp(int(value) / 1000).strftime(
                "%Y-%m-%d %H:%M:%S"
            )
        except (TypeError, ValueError, OSError):
            return str(value)

    def _format_number(self, value):
        try:
            number = float(value)
        except (TypeError, ValueError):
            return str(value)
        if number.is_integer():
            return str(int(number))
        return str(number)

    def _get_company_detail(self, soup, label):
        for item in soup.select(".company .details li"):
            text = self._get_text(item)
            if label not in text:
                continue
            text = re.sub(r"\s+", " ", text)
            return text.replace(label, "", 1).strip()
        return ""

    def _get_text(self, tag, separator=" ", strip=True):
        if tag is None:
            return ""
        return tag.get_text(separator=separator, strip=strip)

    def _get_attr(self, tag, attr_name):
        if tag is None:
            return ""
        return tag.get(attr_name, "")
