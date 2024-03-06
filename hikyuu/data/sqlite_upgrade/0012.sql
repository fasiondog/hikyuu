BEGIN TRANSACTION;

CREATE TABLE
    IF NOT EXISTS `zh_bond10` (
        "id" INTEGER,
        `date` INT NOT NULL,
        `value` INT NOT NULL,
        PRIMARY KEY("id" AUTOINCREMENT)
    );

CREATE INDEX "ix_date_on_zh_bond10" ON "zh_bond10" (date);

UPDATE `version` set `version` = 12;

COMMIT;