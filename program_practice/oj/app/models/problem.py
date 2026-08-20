from __future__ import annotations

import re

from pydantic import BaseModel, Field, field_validator, model_validator

from app.models.enums import Difficulty


PROBLEM_ID_RE = re.compile(r"^[A-Za-z0-9_-]{1,32}$")


class Sample(BaseModel):
    input: str
    output: str


class TestCase(BaseModel):
    case_id: str = Field(min_length=1)
    input: str
    output: str
    score: int = Field(ge=0)
    is_hidden: bool

    @model_validator(mode="after")
    def validate_non_empty_case(self) -> "TestCase":
        if not self.input.strip() and not self.output.strip():
            raise ValueError("test case input and output cannot both be empty")
        return self


class ProblemBase(BaseModel):
    id: str
    title: str = Field(min_length=1, max_length=100)
    description: str = Field(min_length=1)
    input_description: str = Field(min_length=1)
    output_description: str = Field(min_length=1)
    samples: list[Sample]
    constraints: str = ""
    time_limit: float = Field(gt=0)
    memory_limit: int = Field(gt=0)
    difficulty: Difficulty
    tags: list[str] = Field(default_factory=list)
    test_cases: list[TestCase]

    @field_validator("id")
    @classmethod
    def validate_problem_id(cls, value: str) -> str:
        if not PROBLEM_ID_RE.fullmatch(value):
            raise ValueError("problem id must contain only letters, digits, underscore or hyphen")
        return value

    @field_validator("samples")
    @classmethod
    def validate_samples(cls, value: list[Sample]) -> list[Sample]:
        if not value:
            raise ValueError("at least one sample is required")
        return value

    @field_validator("test_cases")
    @classmethod
    def validate_test_cases(cls, value: list[TestCase]) -> list[TestCase]:
        if not value:
            raise ValueError("at least one test case is required")
        return value

    @model_validator(mode="after")
    def validate_case_ids_and_scores(self) -> "ProblemBase":
        case_ids = [case.case_id for case in self.test_cases]
        if len(case_ids) != len(set(case_ids)):
            raise ValueError("case_id must be unique in the same problem")
        if abs(sum(case.score for case in self.test_cases) - 100) > 0.01:
            raise ValueError("test case scores must sum to 100")
        return self


class ProblemCreateRequest(ProblemBase):
    pass


class ProblemUpdateRequest(ProblemBase):
    pass


class ProblemListItem(BaseModel):
    id: str
    title: str
    difficulty: Difficulty
    tags: list[str]
    time_limit: float
    memory_limit: int


class ProblemPublicDetail(BaseModel):
    id: str
    title: str
    description: str
    input_description: str
    output_description: str
    samples: list[Sample]
    constraints: str
    time_limit: float
    memory_limit: int
    difficulty: Difficulty
    tags: list[str]


class ProblemPrivateDetail(ProblemBase):
    pass
