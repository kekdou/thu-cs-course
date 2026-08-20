create_company_table = """
CREATE TABLE IF NOT EXISTS `companies` (
    `id` BIGINT AUTO_INCREMENT PRIMARY KEY,
    `source_site` VARCHAR(50) NOT NULL,
    `source_company_id` VARCHAR(100),

    `name` VARCHAR(255) NOT NULL,
    `summary` TEXT,
    `logo_url` VARCHAR(500),
    `industry` VARCHAR(100),
    `scale` VARCHAR(100),

    `created_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY `uk_company` (`source_site`, `source_company_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"""

create_job_table = """
CREATE TABLE IF NOT EXISTS `jobs` (
    `id` BIGINT AUTO_INCREMENT PRIMARY KEY,
    `source_site` VARCHAR(50) NOT NULL,
    `source_job_id` VARCHAR(100),

    `title` VARCHAR(255) NOT NULL,
    `description` TEXT,
    `job_url` VARCHAR(500),
    `city` VARCHAR(100),
    `salary` VARCHAR(100),
    `experience` VARCHAR(100),
    `education` VARCHAR(100),
    `published_time` VARCHAR(100),
    `tags` TEXT,
    `company_id` BIGINT,

    `created_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY `uk_job` (`source_site`, `source_job_id`),
    KEY `idx_job_company_id` (`company_id`),
    CONSTRAINT `fk_jobs_company`
        FOREIGN KEY (`company_id`) REFERENCES `companies`(`id`)
        ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"""

# insert 都是若重复则更新
insert_company = """
INSERT INTO `companies` (
    `source_site`,
    `source_company_id`,
    `name`,
    `summary`,
    `logo_url`,
    `industry`,
    `scale`
) VALUES (
    %(source_site)s,
    %(source_company_id)s,
    %(name)s,
    %(summary)s,
    %(logo_url)s,
    %(industry)s,
    %(scale)s
)
ON DUPLICATE KEY UPDATE
    `name` = VALUES(`name`),
    `summary` = VALUES(`summary`),
    `logo_url` = VALUES(`logo_url`),
    `industry` = VALUES(`industry`),
    `scale` = VALUES(`scale`);
"""

insert_job = """
INSERT INTO `jobs` (
    `source_site`,
    `source_job_id`,
    `title`,
    `city`,
    `salary`,
    `experience`,
    `education`,
    `tags`,
    `description`,
    `job_url`,
    `published_time`,
    `company_id`
) VALUES (
    %(source_site)s,
    %(source_job_id)s,
    %(title)s,
    %(city)s,
    %(salary)s,
    %(experience)s,
    %(education)s,
    %(tags)s,
    %(description)s,
    %(job_url)s,
    %(published_time)s,
    %(company_id)s
)
ON DUPLICATE KEY UPDATE
    `title` = VALUES(`title`),
    `city` = VALUES(`city`),
    `salary` = VALUES(`salary`),
    `experience` = VALUES(`experience`),
    `education` = VALUES(`education`),
    `tags` = VALUES(`tags`),
    `description` = VALUES(`description`),
    `job_url` = VALUES(`job_url`),
    `published_time` = VALUES(`published_time`),
    `company_id` = VALUES(`company_id`);
"""

select_company_id_by_source_id = """
SELECT id FROM companies
WHERE source_site = %(source_site)s AND source_company_id = %(source_company_id)s
"""

select_company_id_by_name = """
SELECT id FROM companies
WHERE source_site = %(source_site)s AND name = %(name)s 
ORDER BY id DESC LIMIT 1
"""

select_job_id_by_source_id = """
SELECT id FROM jobs
WHERE source_site = %(source_site)s AND source_job_id = %(source_job_id)s
"""

select_job_id_by_title = """
SELECT id FROM jobs
WHERE source_site = %(source_site)s AND title = %(title)s AND company_id <=> %(company_id)s
ORDER BY id DESC LIMIT 1
"""

count_companies = "SELECT COUNT(*) FROM companies"

count_jobs = "SELECT COUNT(*) FROM jobs"
