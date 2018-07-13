use chrono::prelude::*;
use data;
use diesel::prelude::*;
use serde_json;
use std;

embed_migrations!("./migrations");

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
    id: i32,
    type_: String,
    resource_id: i32,
    system_updated_at: NaiveDateTime,
}

pub fn run_migrations(connection: &SqliteConnection) {
    embedded_migrations::run_with_output(connection, &mut std::io::stdout()).unwrap();
}

pub fn establish_connection(database_url: &str) -> SqliteConnection {
    let connection = SqliteConnection::establish(database_url)
        .unwrap_or_else(|_| panic!("Error connecting to {}", database_url));

    connection
        .execute("PRAGMA foreign_keys = ON")
        .expect("Failed to enable forgein_keys.");

    connection
}

pub fn notebooks(
    since_revision: Option<DateTime<Utc>>,
    connection: &SqliteConnection,
) -> Result<Vec<data::Notebook>, String> {
    use schema::notebooks::dsl::*;

    let query_result = match since_revision {
        None => notebooks.load::<Notebook>(connection),
        Some(since_revision) => notebooks
            .filter(system_updated_at.gt(since_revision.naive_utc()))
            .load::<Notebook>(connection),
    };

    query_result
        .map_err(|e| format!("{}", e))
        .map(|n| map_notebooks(&n))
}

pub fn notes(
    since_revision: Option<DateTime<Utc>>,
    connection: &SqliteConnection,
) -> Result<Vec<data::Note>, String> {
    use schema::notes::dsl::*;

    let query_result = match since_revision {
        None => notes.load::<Note>(connection),
        Some(since_revision) => notes
            .filter(system_updated_at.gt(since_revision.naive_utc()))
            .load::<Note>(connection),
    };

    query_result
        .map_err(|e| format!("{}", e))
        .map(|n| map_notes(&n))
}

fn map_notebooks(notebooks: &[Notebook]) -> Vec<data::Notebook> {
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

fn map_notes(notes: &[Note]) -> Vec<data::Note> {
    notes.iter().map(map_note).collect()
}

fn map_note(note: &Note) -> data::Note {
    let tags: Vec<String> = match &note.tags {
        None => vec![],
        Some(tags) => tags.split(',').map(String::from).collect(),
    };

    data::Note {
        id: note.id,
        title: note.title.to_owned(),
        tags,
        notebook_id: note.notebook_id,
        created_at: to_utc(note.created_at),
        updated_at: to_utc(note.updated_at),
        system_updated_at: to_utc(note.system_updated_at),
    }
}

pub fn content_blocks(
    since_revision: Option<DateTime<Utc>>,
    connection: &SqliteConnection,
) -> Result<Vec<data::ContentBlock>, String> {
    use schema::content_blocks::dsl::*;

    let query_result = match since_revision {
        None => content_blocks.load::<ContentBlock>(connection),
        Some(since_revision) => content_blocks
            .filter(system_updated_at.gt(since_revision.naive_utc()))
            .load::<ContentBlock>(connection),
    };

    query_result
        .map_err(|e| format!("{}", e))
        .map(|c| map_content_blocks(&c))
}

fn map_content_blocks(content_blocks: &[ContentBlock]) -> Vec<data::ContentBlock> {
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
        Content::Text { text } => data::Content::Text { text },
        Content::Code { language, code } => data::Content::Code { language, code },
    }
}

pub fn deletions(
    since_revision: Option<DateTime<Utc>>,
    connection: &SqliteConnection,
) -> Result<Vec<data::Deletion>, String> {
    use schema::deletions::dsl::*;

    let query_result = match since_revision {
        None => deletions.load::<Deletion>(connection),
        Some(since_revision) => deletions
            .filter(system_updated_at.gt(since_revision.naive_utc()))
            .load::<Deletion>(connection),
    };

    query_result
        .map_err(|e| format!("{}", e))
        .map(|d| map_deletions(&d))
}

fn map_deletions(deletions: &[Deletion]) -> Vec<data::Deletion> {
    deletions.iter().map(map_deletion).collect()
}

fn map_deletion(deletion: &Deletion) -> data::Deletion {
    data::Deletion {
        id: deletion.id,
        type_: deletion.type_.to_owned(),
        resource_id: deletion.resource_id,
        system_updated_at: to_utc(deletion.system_updated_at),
    }
}

// Convert to UTC DateTime. This is assuming that the
// NaiveDateTimes stored in the DB are actually UTC!
fn to_utc(date_time: NaiveDateTime) -> DateTime<Utc> {
    DateTime::<Utc>::from_utc(date_time, Utc)
}
