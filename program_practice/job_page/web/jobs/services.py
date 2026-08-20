import time

from django.core.paginator import Paginator
from django.db.models import Q
from django.shortcuts import get_object_or_404

from .models import Company, Job


PAGE_SIZE = 20
SEARCH_KEYWORD_MAX_LENGTH = 50


def paginate_jobs(request):
    """
    job 列表页分页
    """
    jobs = Job.objects.select_related("company").order_by("-id")
    paginator = Paginator(jobs, PAGE_SIZE)
    page_number = request.GET.get("page", 1)
    page_obj = paginator.get_page(page_number)
    page_obj.page_window = build_page_window(page_obj)
    return page_obj


def get_job_detail(job_id):
    jobs = Job.objects.select_related("company")
    return get_object_or_404(jobs, id=job_id)


def paginate_companies(request):
    """
    company 列表页分页
    """
    companies = Company.objects.order_by("-id")
    paginator = Paginator(companies, PAGE_SIZE)
    page_number = request.GET.get("page", 1)
    page_obj = paginator.get_page(page_number)
    page_obj.page_window = build_page_window(page_obj)
    return page_obj


def get_company_detail(company_id):
    return get_object_or_404(Company, id=company_id)


def get_company_jobs(company):
    return company.jobs.order_by("-id")


def search_items(request):
    """
    搜索职位或公司，并记录搜索耗时
    """
    keyword = request.GET.get("q", "").strip()[:SEARCH_KEYWORD_MAX_LENGTH]
    search_type = request.GET.get("type", "job")
    if search_type not in ("job", "company"):
        search_type = "job"

    start_time = time.perf_counter()

    if not keyword:
        results = Job.objects.none()
    elif search_type == "company":
        results = Company.objects.filter(
            Q(name__icontains=keyword)
            | Q(industry__icontains=keyword)
            | Q(summary__icontains=keyword)
        ).order_by("-id")
    else:
        results = Job.objects.select_related("company").filter(
            Q(title__icontains=keyword)
            | Q(description__icontains=keyword)
            | Q(tags__icontains=keyword)
            | Q(city__icontains=keyword)
        ).order_by("-id")

    paginator = Paginator(results, PAGE_SIZE)
    page_obj = paginator.get_page(request.GET.get("page", 1))
    list(page_obj.object_list)
    page_obj.page_window = build_page_window(page_obj)

    elapsed_time = time.perf_counter() - start_time
    return {
        "keyword": keyword,
        "search_type": search_type,
        "search_type_text": "公司" if search_type == "company" else "职位",
        "page_obj": page_obj,
        "total_results": page_obj.paginator.count,
        "elapsed_time": elapsed_time,
        "has_searched": bool(keyword),
    }


def build_page_window(page_obj):
    """
    构造类似 1 ... 4 5 6 7 8 ... 的页码窗口。
    """
    current = page_obj.number
    total = page_obj.paginator.num_pages

    if total <= 8:
        return list(range(1, total + 1))

    if current <= 4:
        pages = list(range(1, 6))
        return pages + ["..."]

    if current >= total - 3:
        pages = list(range(total - 4, total + 1))
        return [1, "..."] + pages

    pages = list(range(current - 2, current + 3))
    return [1, "..."] + pages + ["..."]
