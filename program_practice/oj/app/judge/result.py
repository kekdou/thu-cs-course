from __future__ import annotations

from app.models.enums import JudgeResult


RESULT_PRIORITY = [
    JudgeResult.SE.value,
    JudgeResult.MLE.value,
    JudgeResult.TLE.value,
    JudgeResult.RE.value,
    JudgeResult.WA.value,
]


def summarize_cases(cases: list[dict]) -> dict:
    if cases and all(case["result"] == JudgeResult.AC.value for case in cases):
        result = JudgeResult.AC.value
    else:
        results = {case["result"] for case in cases}
        result = next((item for item in RESULT_PRIORITY if item in results), JudgeResult.WA.value)
    return {
        "result": result,
        "score": sum(case["score"] for case in cases if case["result"] == JudgeResult.AC.value),
        "total_time": round(sum(case["time_used"] for case in cases), 6),
        "cases": cases,
    }
