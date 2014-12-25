CREATE TABLE `publickeys` (
  `publickey_id`  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `publickey`     VARCHAR(2048)  NOT NULL,
  `fingerprint`   VARCHAR(64)    UNIQUE NOT NULL,
  `name`          VARCHAR(64),
  `email`         VARCHAR(64),
  `phone`         VARCHAR(64)
);