CREATE TABLE `messages` (
  `message_id`  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `sender` VARCHAR(64) NOT NULL,
  `receiver` VARCHAR(64) NOT NULL,
  `creationdate` DATETIME NOT NULL,
  `fetcheddate` DATETIME DEFAULT NULL,
  `message` BLOB NOT NULL
);