use chrono::prelude::*;
use data;
use diesel::prelude::*;
use serde_json;

#[derive(Queryable)]
struct Notebook {
    id: i32,
    name: String,
    created_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Queryable)]
struct Note {
    id: i32,
    title: String,
    tags: Option<String>,
    notebook_id: i32,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Queryable)]
struct ContentBlock {
    id: i32,
    #[allow(dead_code)]
    type_: String,
    content: String,
    note_id: i32,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(untagged)]
pub enum Content {
    Text { text: String },
    Code { language: String, code: String },
}

#[derive(Queryable)]
struct Deletion {
    _type: String,
    id: i32,
    system_updated_at: NaiveDateTime,
}

pub fn establish_connection(database_url: String) -> SqliteConnection {
    let connection = SqliteConnection::establish(&database_url)
        .expect(&format!("Error connecting to {}", database_url));

    connection
        .execute("PRAGMA foreign_keys = ON")
        .expect("Failed to enable forgein_keys.");
    connection
}

pub fn notebooks(
    since_revision: NaiveDateTime,
    connection: &SqliteConnection,
) -> Result<Vec<data::Notebook>, String> {
    use schema::notebooks::dsl::*;

    notebooks
        .filter(system_updated_at.gt(since_revision))
        .load::<Notebook>(connection)
        .map_err(|e| format!("{}", e))
        .map(map_notebooks)
}

pub fn notes(
    since_revision: NaiveDateTime,
    connection: &SqliteConnection,
) -> Result<Vec<data::Note>, String> {
    use schema::notes::dsl::*;

    notes
        .filter(system_updated_at.gt(since_revision))
        .load::<Note>(connection)
        .map_err(|e| format!("{}", e))
        .map(map_notes)
}

fn map_notebooks(notebooks: Vec<Notebook>) -> Vec<data::Notebook> {
    notebooks.iter().map(map_notebook).collect()
}

fn map_notebook(notebook: &Notebook) -> data::Notebook {
    data::Notebook {
        id: notebook.id,
        name: notebook.name.to_owned(),
        created_at: to_utc(notebook.created_at),
        system_updated_at: to_utc(notebook.system_updated_at),
    }
}

fn map_notes(notes: Vec<Note>) -> Vec<data::Note> {
    notes.iter().map(map_note).collect()
}

fn map_note(note: &Note) -> data::Note {
    let tags: Vec<String> = match &note.tags {
        None => vec![],
        Some(tags) => tags.split(",").map(String::from).collect(),
    };

    data::Note {
        id: note.id,
        title: note.title.to_owned(),
        tags: tags,
        notebook_id: note.notebook_id,
        created_at: to_utc(note.created_at),
        updated_at: to_utc(note.updated_at),
        system_updated_at: to_utc(note.system_updated_at),
    }
}

pub fn content_blocks(
    since_revision: NaiveDateTime,
    connection: &SqliteConnection,
) -> Result<Vec<data::ContentBlock>, String> {
    use schema::content_blocks::dsl::*;

    content_blocks
        .filter(system_updated_at.gt(since_revision))
        .load::<ContentBlock>(connection)
        .map_err(|e| format!("{}", e))
        .map(map_content_blocks)
}

fn map_content_blocks(content_blocks: Vec<ContentBlock>) -> Vec<data::ContentBlock> {
    content_blocks.iter().map(map_content_block).collect()
}

fn map_content_block(content_block: &ContentBlock) -> data::ContentBlock {
    data::ContentBlock {
        id: content_block.id,
        note_id: content_block.note_id,
        content: map_content(content_block),
        system_updated_at: to_utc(content_block.system_updated_at),
        created_at: to_utc(content_block.created_at),
        updated_at: to_utc(content_block.updated_at),
    }
}

fn map_content(content_block: &ContentBlock) -> data::Content {
    let content: Content = serde_json::from_str(&content_block.content).unwrap(); // FIXME

    match content {
        Content::Text { text } => data::Content::Text { text: text },
        Content::Code { language, code } => data::Content::Code {
            language: language,
            code: code,
        },
    }
}

pub fn deletions(since_revision: NaiveDateTime) -> Result<Vec<data::Deletion>, String> {
    Ok(vec![])
}

// Convert to UTC DateTime. This is assuming that the
// NaiveDateTimes stored in the DB are actually UTC!
fn to_utc(date_time: NaiveDateTime) -> DateTime<Utc> {
    DateTime::<Utc>::from_utc(date_time, Utc)
}
