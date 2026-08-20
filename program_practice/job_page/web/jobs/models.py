from django.db import models
import re


class Company(models.Model):
    id = models.BigAutoField(primary_key=True)
    source_site = models.CharField(max_length=50)
    source_company_id = models.CharField(max_length=100, blank=True, null=True)

    name = models.CharField(max_length=255)
    summary = models.TextField(blank=True, null=True)
    logo_url = models.CharField(max_length=500, blank=True, null=True)
    industry = models.CharField(max_length=100, blank=True, null=True)
    scale = models.CharField(max_length=100, blank=True, null=True)

    created_at = models.DateTimeField()

    class Meta:
        managed = False     # 仅 query 不 edit
        db_table = "companies"

    def __str__(self):
        return self.name


class Job(models.Model):
    id = models.BigAutoField(primary_key=True)
    source_site = models.CharField(max_length=50)
    source_job_id = models.CharField(max_length=100, blank=True, null=True)

    title = models.CharField(max_length=255)
    description = models.TextField(blank=True, null=True)
    job_url = models.CharField(max_length=500, blank=True, null=True)
    city = models.CharField(max_length=100, blank=True, null=True)
    salary = models.CharField(max_length=100, blank=True, null=True)
    experience = models.CharField(max_length=100, blank=True, null=True)
    education = models.CharField(max_length=100, blank=True, null=True)
    published_time = models.CharField(max_length=100, blank=True, null=True)
    tags = models.TextField(blank=True, null=True)
    company = models.ForeignKey(
        Company,
        db_column="company_id",
        related_name="jobs",
        on_delete=models.SET_NULL,
        blank=True,
        null=True,
    )

    created_at = models.DateTimeField()

    class Meta:
        managed = False     # 仅 query 不 edit
        db_table = "jobs"

    @property
    def tag_list(self):
        if not self.tags:
            return []
        return [tag for tag in re.split(r"[;；,\s]+", self.tags) if tag]

    def __str__(self):
        return self.title
