CREATE TABLE notebooks (
  id VARCHAR(10) NOT NULL PRIMARY KEY,
  title VARCHAR NOT NULL,
  created_at DATETIME NOT NULL,
  updated_at DATETIME NOT NULL,
  system_updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);
