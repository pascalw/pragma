CREATE TABLE content_blocks (
  id VARCHAR(10) NOT NULL PRIMARY KEY,
  type VARCHAR NOT NULL,
  content VARCHAR NOT NULL,
  note_id VARCHAR(10) NOT NULL,
  created_at DATETIME NOT NULL,
  updated_at DATETIME NOT NULL,
  system_updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY(note_id) REFERENCES notes(id) ON DELETE CASCADE
);