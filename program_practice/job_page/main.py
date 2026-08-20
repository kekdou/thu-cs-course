import argparse
import os
import traceback

PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))

from config.ncss_config import AREA_CODES

from crawler import RequestFailedTooManyTimes
from crawler import NCSSDownloader, NCSSParser
from database import DBClient

DB_CONFIG_PATH = os.path.join(PROJECT_DIR, "config", "db_config.json")

MAX_PAGES_PER_AREA = 50
REST_EVERY_N_JOBS = 80


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--city", default="all")
    return parser.parse_args()


def choose_area_items(area_choice):
    if area_choice.lower() == "all":
        return list(AREA_CODES.items())
    if area_choice in AREA_CODES:
        return [(area_choice, AREA_CODES[area_choice])]

    available_text = " / ".join(AREA_CODES.keys())
    raise RuntimeError(
        f"unknown city: {area_choice}, available areas: {available_text}"
    )


def crawl(downloader, parser, db_client, area_items):
    stats = {
        "list_pages": 0,
        "detail_pages": 0,
        "saved_jobs": 0,
        "failed_jobs": 0,
        "skipped_jobs": 0,
    }

    for area_name, area_code in area_items:
        print(f"start crawl city: {area_name} ({area_code})")

        for page_num in range(1, MAX_PAGES_PER_AREA + 1):
            print(f"fetch list page: {area_name} - page {page_num}")
            try:
                list_path = downloader.fetch_list_json(area_name, area_code, page_num)
                job_items = parser.parse_list_json(list_path)
            except RequestFailedTooManyTimes:
                print(
                    f"same list request failed too many times: {area_name} - page {page_num}"
                )
                raise
            except Exception as error:
                stats["failed_jobs"] += 1
                print(f"fetch list page failed: {area_name} - page {page_num}")
                print(error)
                traceback.print_exc()
                continue

            if not job_items:
                print(
                    f"stop city because list page is empty: {area_name} - page {page_num}"
                )
                break

            stats["list_pages"] += 1
            print(f"found {len(job_items)} jobs")

            for job_item in job_items:
                job_id = job_item.get("jobId", "")
                if not job_id:
                    print("skip job because jobId is empty")
                    stats["skipped_jobs"] += 1
                    continue

                try:
                    detail_path = downloader.fetch_detail_html(job_id)
                    detail_data = parser.parse_detail_html(detail_path)

                    company_summary = ""
                    company_id = detail_data.get("company_id")
                    company_href = detail_data.get("company_href")
                    if company_id:
                        company_path = downloader.fetch_company_html(
                            company_id, company_href
                        )
                        if company_path:
                            company_summary = parser.parse_company_summary(company_path)

                    company_data, job_data = parser.parse_job(
                        job_item, detail_data, company_summary
                    )
                    db_client.save_job_with_company(company_data, job_data)
                    stats["detail_pages"] += 1
                    stats["saved_jobs"] += 1
                    print(f"saved job: {job_data.get('title', '')}")
                except RequestFailedTooManyTimes:
                    print(f"same detail request failed too many times: {job_id}")
                    raise
                except Exception as error:
                    stats["failed_jobs"] += 1
                    print(f"process job failed: {job_id}")
                    print(error)
                    traceback.print_exc()

                if (
                    stats["detail_pages"] > 0
                    and stats["detail_pages"] % REST_EVERY_N_JOBS == 0
                ):
                    print(f"take a rest after {stats['detail_pages']} detail pages")
                    downloader.rest()

    return stats


def main():
    db_client = None
    try:
        args = parse_args()
        area_items = choose_area_items(args.city)

        cookie = os.environ.get("NCSS_COOKIE", "")
        if not cookie:
            from crawler.login import NCSSLoginManager

            cookie = NCSSLoginManager().get_cookie()

        downloader = NCSSDownloader(cookie)
        parser = NCSSParser()
        db_client = DBClient(DB_CONFIG_PATH)
        db_client.init_table()

        company_count, job_count = db_client.count_data()
        print(f"database is ready, companies={company_count}, jobs={job_count}")

        stats = crawl(downloader, parser, db_client, area_items)
        company_count, job_count = db_client.count_data()

        print("crawl finished")
        print(f"list pages: {stats['list_pages']}")
        print(f"detail pages: {stats['detail_pages']}")
        print(f"saved jobs: {stats['saved_jobs']}")
        print(f"failed jobs: {stats['failed_jobs']}")
        print(f"skipped jobs: {stats['skipped_jobs']}")
        print(f"database companies: {company_count}")
        print(f"database jobs: {job_count}")
    except Exception as error:
        print("main process failed")
        print(error)
        traceback.print_exc()
    finally:
        if db_client is not None:
            db_client.close()


if __name__ == "__main__":
    main()
