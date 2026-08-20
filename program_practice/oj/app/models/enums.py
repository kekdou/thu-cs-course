from __future__ import annotations

from enum import Enum


class UserRole(str, Enum):
    student = "student"
    teacher = "teacher"
    admin = "admin"


class SubmissionStatus(str, Enum):
    pending = "pending"
    running = "running"
    finished = "finished"
    failed = "failed"


class JudgeResult(str, Enum):
    AC = "AC"
    WA = "WA"
    RE = "RE"
    TLE = "TLE"
    MLE = "MLE"
    SE = "SE"


class Difficulty(str, Enum):
    easy = "easy"
    medium = "medium"
    hard = "hard"
