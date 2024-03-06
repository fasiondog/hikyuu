BEGIN TRANSACTION;

CREATE TABLE
    IF NOT EXISTS `Block` (
        "id" INTEGER,
        `category` VARCHAR(100) NOT NULL,
        `name` VARCHAR(100) NOT NULL,
        `market_code` VARCHAR(30) NOT NULL,
        PRIMARY KEY("id" AUTOINCREMENT)
    );

CREATE INDEX "ix_block" ON "block" (category, name);

UPDATE `version` set `version` = 11;

COMMIT;