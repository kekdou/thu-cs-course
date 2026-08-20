from __future__ import annotations


def normalize_output(text: str) -> str:
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    lines = [line.rstrip(" \t") for line in text.split("\n")]
    while lines and lines[-1] == "":
        lines.pop()
    return "\n".join(lines)


def is_standard_match(actual: str, expected: str) -> bool:
    return normalize_output(actual) == normalize_output(expected)
