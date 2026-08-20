from analysis_utils import (
    add_to_group,
    build_stats,
    fetch_jobs,
    get_education_group,
    parse_salary,
    print_table,
)


def main():
    jobs = fetch_jobs()
    education_salary = {
        "Junior College": [],
        "Bachelor": [],
        "Master": [],
        "No Limit": [],
    }

    for job in jobs:
        salary = parse_salary(job.get("salary"))
        if salary is None:
            continue

        education = get_education_group(job.get("education"))
        add_to_group(education_salary, education, salary)

    rows = []
    for education in ["Junior College", "Bachelor", "Master", "No Limit"]:
        item = build_stats(education_salary[education])
        rows.append(
            [
                education,
                item["count"],
                f"{item['average']:.2f}",
                f"{item['median']:.2f}",
            ]
        )

    print_table(
        "Salary Level by Education",
        rows,
        ["Education", "Job Count", "Average(k/month)", "Median(k/month)"],
    )


if __name__ == "__main__":
    main()
