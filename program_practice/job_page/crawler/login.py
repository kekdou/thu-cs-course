import os
from playwright.sync_api import sync_playwright

from config.ncss_config import SEARCH_PAGE_URL

PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BROWSER_DATA_DIR = os.path.join(PROJECT_DIR, "output", "browser_data")
COOKIE_PATH = os.path.join(BROWSER_DATA_DIR, "cookie.txt")


class NCSSLoginManager:
    def __init__(self, headless=False):
        self.headless = headless

    def get_cookie(self):
        os.makedirs(BROWSER_DATA_DIR, exist_ok=True)
        saved_cookie = self._load_cookie()
        if saved_cookie:
            print(f"use saved cookie: {COOKIE_PATH}")
            return saved_cookie

        with sync_playwright() as playwright:
            context = playwright.chromium.launch_persistent_context(
                user_data_dir=BROWSER_DATA_DIR,
                headless=self.headless,
                viewport={"width": 1440, "height": 900},
                locale="zh-CN",
            )
            page = context.pages[0] if context.pages else context.new_page()
            page.goto(SEARCH_PAGE_URL, wait_until="domcontentloaded")

            print("please finish login in browser")
            print("after login success, return to terminal and press Enter")
            input()

            self._wait_page_stable(page)
            cookies = context.cookies()
            context.close()

        cookie_text = self._format_cookie(cookies)
        self._save_cookie(cookie_text)
        return cookie_text

    def _load_cookie(self):
        if not os.path.exists(COOKIE_PATH):
            return ""
        with open(COOKIE_PATH, "r", encoding="utf-8") as f:
            return f.read().strip()

    def _save_cookie(self, cookie_text):
        if not cookie_text:
            return
        with open(COOKIE_PATH, "w", encoding="utf-8") as f:
            f.write(cookie_text)
        print(f"cookie saved: {COOKIE_PATH}")

    def _wait_page_stable(self, page):
        try:
            page.wait_for_load_state("networkidle", timeout=5000)
        except Exception:
            pass

    def _format_cookie(self, cookies):
        cookie_parts = []
        for cookie in cookies:
            name = cookie.get("name")
            value = cookie.get("value")
            if name and value:
                cookie_parts.append(f"{name}={value}")
        return "; ".join(cookie_parts)
