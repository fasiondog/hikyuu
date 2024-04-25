BEGIN TRANSACTION;

CREATE TABLE
    IF NOT EXISTS `BlockIndex` (
        "id" INTEGER,
        `category` VARCHAR(100) NOT NULL,
        `name` VARCHAR(100) NOT NULL,
        `market_code` VARCHAR(30) NOT NULL,
        PRIMARY KEY("id" AUTOINCREMENT)
    );

CREATE INDEX "ix_blockindex_category_name" ON "block" (category, name);

UPDATE `version` set `version` = 17;

COMMIT;