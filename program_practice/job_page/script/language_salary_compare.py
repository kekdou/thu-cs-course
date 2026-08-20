from analysis_utils import (
    add_to_group,
    build_stats,
    fetch_jobs,
    get_language_groups,
    parse_salary,
    print_table,
)


LANGUAGES = ["Python", "Java", "C++", "JavaScript", "Go", "PHP", "SQL"]


def main():
    jobs = fetch_jobs()
    language_salary = {language: [] for language in LANGUAGES}

    for job in jobs:
        salary = parse_salary(job.get("salary"))
        if salary is None:
            continue

        languages = get_language_groups(job)
        for language in languages:
            add_to_group(language_salary, language, salary)

    rows = []
    for language in LANGUAGES:
        item = build_stats(language_salary[language])
        rows.append(
            [
                language,
                item["count"],
                f"{item['average']:.2f}",
                f"{item['median']:.2f}",
            ]
        )

    rows.sort(key=lambda row: float(row[2]), reverse=True)
    print_table(
        "Salary Level by Programming Language",
        rows,
        ["Language", "Job Count", "Average(k/month)", "Median(k/month)"],
    )


if __name__ == "__main__":
    main()
