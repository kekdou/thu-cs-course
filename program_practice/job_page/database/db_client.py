import pymysql
from database import sql_statements as sql
import json


class DBClient:
    def __init__(self, filepath):
        with open(filepath, "r", encoding="utf-8") as f:
            self.config = json.load(f)
        self.connection = pymysql.connect(
            host=self.config["host"],
            port=self.config["port"],
            user=self.config["user"],
            password=self.config["password"],
            database=self.config["database"],
            charset=self.config["charset"],
        )

    def close(self):
        if self.connection:
            self.connection.close()

    def init_table(self):
        """
        创建 company 和 job 表
        """
        cursor = self.connection.cursor()
        try:
            cursor.execute(sql.create_company_table)
            cursor.execute(sql.create_job_table)
            self.connection.commit()
        except pymysql.MySQLError as e:
            self.connection.rollback()
            print(f"tables initialize failed: {e}")
            raise
        finally:
            cursor.close()

    def save_job_with_company(self, company_data, job_data):
        """
        接受 company_data 和 job_data，同时 insert 到 table 中
        """
        cursor = self.connection.cursor()
        try:
            company_id = self._insert_company_data(cursor, company_data)
            job_data["company_id"] = company_id
            job_id = self._insert_job_data(cursor, job_data)
            self.connection.commit()
            return company_id, job_id
        except pymysql.MySQLError as e:
            self.connection.rollback()
            print(f"save job with company failed: {e}")
            raise
        finally:
            cursor.close()

    def count_data(self):
        """
        统计数据，返回数量
        """
        cursor = self.connection.cursor()
        try:
            cursor.execute(sql.count_companies)
            count_company = cursor.fetchone()
            cursor.execute(sql.count_jobs)
            count_job = cursor.fetchone()
            return count_company[0] if count_company else 0, count_job[0] if count_job else 0
        finally:
            cursor.close()

    def _insert_company_data(self, cursor, company_data: dict) -> int | None:
        company_data = self._build_company_data(company_data)
        cursor.execute(sql.insert_company, company_data)
        return self._find_company_id(
            cursor,
            company_data["source_site"],
            company_data["source_company_id"],
            company_data["name"],
        )

    def _insert_job_data(self, cursor, job_data: dict) -> int | None:
        job_data = self._build_job_data(job_data)
        cursor.execute(sql.insert_job, job_data)
        return self._find_job_id(
            cursor,
            job_data["source_site"],
            job_data["source_job_id"],
            job_data["title"],
            job_data["company_id"],
        )

    def _build_company_data(self, company_data: dict) -> dict:
        return {
            "source_site": company_data.get("source_site"),
            "source_company_id": company_data.get("source_company_id"),
            "name": company_data.get("name"),
            "summary": company_data.get("summary"),
            "logo_url": company_data.get("logo_url"),
            "industry": company_data.get("industry"),
            "scale": company_data.get("scale"),
        }

    def _build_job_data(self, job_data: dict) -> dict:
        return {
            "source_site": job_data.get("source_site"),
            "source_job_id": job_data.get("source_job_id"),
            "title": job_data.get("title"),
            "city": job_data.get("city"),
            "salary": job_data.get("salary"),
            "experience": job_data.get("experience") or "经验不限",
            "education": job_data.get("education"),
            "tags": job_data.get("tags"),
            "description": job_data.get("description"),
            "job_url": job_data.get("job_url"),
            "published_time": job_data.get("published_time"),
            "company_id": job_data.get("company_id"),
        }

    def _find_company_id(self, cursor, source_site, source_company_id, name):
        if source_company_id:
            cursor.execute(
                sql.select_company_id_by_source_id,
                {
                    "source_site": source_site,
                    "source_company_id": source_company_id,
                },
            )
        else:
            cursor.execute(
                sql.select_company_id_by_name,
                {
                    "source_site": source_site,
                    "name": name,
                },
            )
        result = cursor.fetchone()
        return result[0] if result else None

    def _find_job_id(self, cursor, source_site, source_job_id, title, company_id):
        if source_job_id:
            cursor.execute(
                sql.select_job_id_by_source_id,
                {
                    "source_site": source_site,
                    "source_job_id": source_job_id,
                },
            )
        else:
            cursor.execute(
                sql.select_job_id_by_title,
                {
                    "source_site": source_site,
                    "title": title,
                    "company_id": company_id,
                },
            )
        result = cursor.fetchone()
        return result[0] if result else None
