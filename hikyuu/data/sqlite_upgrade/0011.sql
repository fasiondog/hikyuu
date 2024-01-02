BEGIN TRANSACTION;

CREATE TABLE
    IF NOT EXISTS `Block` (
        "id" INTEGER,
        `category` VARCHAR(100) NOT NULL UNIQUE,
        `content` TEXT NOT NULL,
        PRIMARY KEY("id" AUTOINCREMENT)
    );

CREATE INDEX "ix_block" ON "block" (category);

UPDATE `version` set `version` = 11;

COMMIT;