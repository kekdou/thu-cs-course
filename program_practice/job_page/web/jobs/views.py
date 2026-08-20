from django.shortcuts import render

from .services import (
    get_company_detail,
    get_company_jobs,
    get_job_detail,
    paginate_companies,
    paginate_jobs,
    search_items,
)


def job_list(request):
    page_obj = paginate_jobs(request)
    context = {
        "page_obj": page_obj,
        "total_jobs": page_obj.paginator.count,
        "page_base_url": "?",
    }
    return render(request, "jobs/job_list.html", context)


def job_detail(request, job_id):
    job = get_job_detail(job_id)
    return render(request, "jobs/job_detail.html", {"job": job})


def company_list(request):
    page_obj = paginate_companies(request)
    context = {
        "page_obj": page_obj,
        "total_companies": page_obj.paginator.count,
        "page_base_url": "?",
    }
    return render(request, "jobs/company_list.html", context)


def company_detail(request, company_id):
    company = get_company_detail(company_id)
    company_jobs = get_company_jobs(company)
    context = {
        "company": company,
        "company_jobs": company_jobs,
    }
    return render(request, "jobs/company_detail.html", context)


def search(request):
    context = search_items(request)
    context["page_base_url"] = ""
    return render(request, "jobs/search_result.html", context)
