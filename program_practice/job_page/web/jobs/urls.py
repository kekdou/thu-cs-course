from django.urls import path

from . import views


urlpatterns = [
    path("", views.job_list, name="home"),
    path("jobs/", views.job_list, name="job_list"),
    path("jobs/<int:job_id>/", views.job_detail, name="job_detail"),
    path("companies/", views.company_list, name="company_list"),
    path("companies/<int:company_id>/", views.company_detail, name="company_detail"),
    path("search/", views.search, name="search"),
]
