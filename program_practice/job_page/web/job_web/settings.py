import json
import os


BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROJECT_DIR = os.path.dirname(BASE_DIR)
DB_CONFIG_PATH = os.path.join(PROJECT_DIR, "config", "db_config.json")

SECRET_KEY = "job-page-dev-secret-key"
DEBUG = True
ALLOWED_HOSTS = ["localhost", "127.0.0.1", "testserver"]

INSTALLED_APPS = [
    "django.contrib.contenttypes",
    "django.contrib.staticfiles",
    "jobs",
]

MIDDLEWARE = [
    "django.middleware.security.SecurityMiddleware",
    "django.middleware.common.CommonMiddleware",
]

ROOT_URLCONF = "job_web.urls"

TEMPLATES = [
    {
        "BACKEND": "django.template.backends.django.DjangoTemplates",
        "DIRS": [],
        "APP_DIRS": True,
        "OPTIONS": {
            "context_processors": [
                "django.template.context_processors.request",
            ],
        },
    }
]

WSGI_APPLICATION = "job_web.wsgi.application"

with open(DB_CONFIG_PATH, "r", encoding="utf-8") as f:
    db_config = json.load(f)

DATABASES = {
    "default": {
        "ENGINE": "django.db.backends.mysql",
        "HOST": db_config["host"],
        "PORT": str(db_config["port"]),
        "USER": db_config["user"],
        "PASSWORD": db_config["password"],
        "NAME": db_config["database"],
        "CHARSET": db_config.get("charset", "utf8mb4"),
        "OPTIONS": {
            "charset": db_config.get("charset", "utf8mb4"),
        },
    }
}

LANGUAGE_CODE = "zh-hans"
TIME_ZONE = "Asia/Shanghai"
USE_I18N = True
USE_TZ = False

STATIC_URL = "static/"
STATICFILES_DIRS = []

DEFAULT_AUTO_FIELD = "django.db.models.BigAutoField"
