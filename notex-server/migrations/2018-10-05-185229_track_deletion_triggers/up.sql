CREATE TRIGGER track_notebook_deletes AFTER DELETE ON notebooks
BEGIN
 INSERT INTO deletions (
  type,
  resource_id
 )
 VALUES
 (
   "notebook",
   old.id
 );
END;

CREATE TRIGGER track_note_deletes AFTER DELETE ON notes
BEGIN
 INSERT INTO deletions (
   type,
   resource_id
 )
 VALUES
 (
   "note",
   old.id
 );
END;

CREATE TRIGGER track_content_block_deletes AFTER DELETE ON content_blocks
BEGIN
 INSERT INTO deletions (
   type,
   resource_id
 )
 VALUES
 (
   "contentBlock",
   old.id
 );
END;
