import os
import matplotlib.pyplot as plt

from analysis_utils import (
    add_to_group,
    build_stats,
    chinese_region_to_pinyin,
    fetch_jobs,
    get_region,
    parse_salary,
    print_table,
)

PRO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMG_DIR = os.path.join(PRO_DIR, "image")

def main():
    jobs = fetch_jobs()
    region_salary = {}

    for job in jobs:
        salary = parse_salary(job.get("salary"))
        region = get_region(job.get("city"))

        if salary is None or region is None:
            continue

        region_name = chinese_region_to_pinyin(region)
        if region_name is None:
            continue

        add_to_group(region_salary, region_name, salary)

    stats = []
    for region, values in region_salary.items():
        item = build_stats(values)
        stats.append(
            {
                "region": region,
                "count": item["count"],
                "average": item["average"],
                "median": item["median"],
            }
        )

    stats.sort(key=lambda item: item["average"], reverse=True)

    rows = [
        [
            item["region"],
            item["count"],
            f"{item['average']:.2f}",
            f"{item['median']:.2f}",
        ]
        for item in stats
    ]
    print_table("Salary Level by Region", rows, ["Region", "Job Count", "Average(k/month)", "Median(k/month)"])

    draw_line_chart(stats)


def draw_line_chart(stats):
    output_path = os.path.join(IMG_DIR, "salary_by_region.png")

    regions = [item["region"] for item in stats]
    averages = [item["average"] for item in stats]
    medians = [item["median"] for item in stats]

    plt.figure(figsize=(14, 6))
    plt.plot(regions, averages, marker="o", label="Average Salary")
    plt.plot(regions, medians, marker="s", label="Median Salary")
    plt.title("Salary Level by Region")
    plt.xlabel("Region")
    plt.ylabel("Salary(k/month)")
    plt.xticks(rotation=45)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_path, dpi=200)
    plt.close()

    print(f"\nLine chart saved to: {output_path}")


if __name__ == "__main__":
    main()
