// For Diesel 1.3 on Rust >= 1.29
#![allow(proc_macro_derive_resolution_fallback)]

use chrono::prelude::*;
use data;
use diesel;
use diesel::prelude::*;
use diesel::sql_query;
use repo_id;
use serde_json;
use std;

use schema::content_blocks;
use schema::notebooks;
use schema::notes;

embed_migrations!("./migrations");
static WELCOME_TEXT: &str = include_str!("./static/welcome.html");

#[derive(Queryable)]
struct Notebook {
    id: String,
    title: String,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Insertable)]
#[table_name = "notebooks"]
struct NewNotebook {
    id: String,
    title: String,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Queryable, AsChangeset)]
struct Note {
    id: String,
    title: String,
    tags: Option<String>,
    notebook_id: String,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Insertable)]
#[table_name = "notes"]
struct NewNote {
    id: String,
    title: String,
    tags: Option<String>,
    notebook_id: String,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Queryable)]
struct ContentBlock {
    id: String,
    #[allow(dead_code)]
    type_: String,
    content: String,
    note_id: String,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
}

#[derive(Insertable)]
#[table_name = "content_blocks"]
struct NewContentBlock {
    id: String,
    type_: String,
    content: String,
    created_at: NaiveDateTime,
    updated_at: NaiveDateTime,
    system_updated_at: NaiveDateTime,
    note_id: String,
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(untagged)]
pub enum Content {
    Text { text: String },
    Code { language: String, code: String },
}

#[derive(Queryable)]
struct Deletion {
    type_: String,
    resource_id: String,
    system_updated_at: NaiveDateTime,
}

fn is_first_run(connection: &SqliteConnection) -> bool {
    type BoolSql = ::diesel::sql_types::Bool;
    #[derive(QueryableByName, Debug)]
    struct TableExistsResult {
        #[sql_type = "BoolSql"]
        exists_: bool,
    }

    let result: QueryResult<TableExistsResult> = sql_query(
        "SELECT 1 as exists_ FROM sqlite_master WHERE name='__diesel_schema_migrations';",
    ).get_result(connection);

    match result {
        Ok(_) => false,
        Err(diesel::NotFound) => true,
        Err(_) => unreachable!(),
    }
}

fn seed(connection: &SqliteConnection) {
    let now = Utc::now();

    let notebook_id = repo_id::generate();
    let notebook = data::NewNotebook {
        id: Some(notebook_id.clone()),
        title: "Example notebook".into(),
        created_at: now,
        updated_at: now,
    };

    let note_id = repo_id::generate();
    let note = data::NewNote {
        id: Some(note_id.clone()),
        title: "Welcome to Pragma!".into(),
        tags: vec![],
        created_at: now,
        updated_at: now,
        notebook_id,
    };

    let content_block_id = repo_id::generate();
    let content_block = data::NewContentBlock {
        id: Some(content_block_id.clone()),
        content: data::Content::Text {
            text: WELCOME_TEXT.into(),
        },
        created_at: now,
        updated_at: now,
        note_id,
    };

    create_notebook(notebook, connection).unwrap();
    create_note(note, connection).unwrap();
    create_content_block(content_block, connection).unwrap();
}

pub fn setup(connection: &SqliteConnection) {
    let is_first_run = is_first_run(connection);

    embedded_migrations::run_with_output(connection, &mut std::io::stdout()).unwrap();

    if is_first_run {
        seed(connection);
    }
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
        .map(map_notebooks)
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

    query_result.map_err(|e| format!("{}", e)).map(map_notes)
}

fn map_notebooks(notebooks: Vec<Notebook>) -> Vec<data::Notebook> {
    notebooks.into_iter().map(map_notebook).collect()
}

fn map_notebook(notebook: Notebook) -> data::Notebook {
    data::Notebook {
        id: notebook.id,
        title: notebook.title,
        created_at: to_utc(notebook.created_at),
        updated_at: to_utc(notebook.updated_at),
        revision: to_utc(notebook.system_updated_at),
    }
}

fn map_notes(notes: Vec<Note>) -> Vec<data::Note> {
    notes.into_iter().map(map_note).collect()
}

fn map_note(note: Note) -> data::Note {
    let tags: Vec<String> = match &note.tags {
        None => vec![],
        Some(tags) => tags_from_string(tags),
    };

    data::Note {
        id: note.id,
        title: note.title,
        tags,
        notebook_id: note.notebook_id,
        created_at: to_utc(note.created_at),
        updated_at: to_utc(note.updated_at),
        revision: to_utc(note.system_updated_at),
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
        .map(map_content_blocks)
}

fn map_content_blocks(content_blocks: Vec<ContentBlock>) -> Vec<data::ContentBlock> {
    content_blocks.into_iter().map(map_content_block).collect()
}

fn map_content_block(content_block: ContentBlock) -> data::ContentBlock {
    let content = map_content(&content_block);

    data::ContentBlock {
        id: content_block.id,
        note_id: content_block.note_id,
        content,
        revision: to_utc(content_block.system_updated_at),
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
        .map(map_deletions)
}

fn map_deletions(deletions: Vec<Deletion>) -> Vec<data::Deletion> {
    deletions.into_iter().map(map_deletion).collect()
}

fn map_deletion(deletion: Deletion) -> data::Deletion {
    data::Deletion {
        type_: deletion.type_,
        resource_id: deletion.resource_id,
        system_updated_at: to_utc(deletion.system_updated_at),
    }
}

pub fn create_notebook(
    notebook: data::NewNotebook,
    conn: &SqliteConnection,
) -> Result<data::Notebook, String> {
    use schema::notebooks::dsl::*;

    let now = Utc::now();

    let new_notebook = NewNotebook {
        id: notebook.id.unwrap_or_else(repo_id::generate),
        title: notebook.title,
        created_at: to_naive(notebook.created_at),
        updated_at: to_naive(notebook.updated_at),
        system_updated_at: to_naive(now),
    };

    let result = conn.transaction::<Notebook, _, _>(|| {
        diesel::insert_into(notebooks)
            .values(&new_notebook)
            .execute(conn)?;

        notebooks.order(system_updated_at.desc()).first(conn)
    });

    match result {
        Ok(notebook) => Ok(map_notebook(notebook)),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn update_notebook(
    notebook_id: &str,
    update: data::NotebookUpdate,
    connection: &SqliteConnection,
) -> Result<data::Notebook, String> {
    use schema::notebooks::dsl::*;

    let result = diesel::update(notebooks.filter(id.eq(&notebook_id)))
        .set((
            title.eq(update.title),
            system_updated_at.eq(to_naive(Utc::now())),
        )).execute(connection)
        .and_then(|_num_rows| notebooks.find(&notebook_id).first(connection));

    match result {
        Ok(notebook) => Ok(map_notebook(notebook)),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn delete_notebook(notebook_id: String, connection: &SqliteConnection) -> Result<(), String> {
    use schema::notebooks::dsl::*;

    let result = diesel::delete(notebooks.filter(id.eq(notebook_id))).execute(connection);

    match result {
        Ok(_num_rows) => Ok(()),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn create_note(note: data::NewNote, conn: &SqliteConnection) -> Result<data::Note, String> {
    use schema::notes::dsl::*;

    let now = Utc::now();

    let new_note = NewNote {
        id: note.id.unwrap_or_else(repo_id::generate),
        title: note.title,
        tags: Some(tags_to_string(&note.tags)),
        notebook_id: note.notebook_id,
        created_at: to_naive(note.created_at),
        updated_at: to_naive(note.updated_at),
        system_updated_at: to_naive(now),
    };

    let result = conn.transaction::<Note, _, _>(|| {
        diesel::insert_into(notes).values(&new_note).execute(conn)?;

        notes.order(system_updated_at.desc()).first(conn)
    });

    match result {
        Ok(note) => Ok(map_note(note)),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn update_note(
    note_id: &str,
    update: data::NoteUpdate,
    connection: &SqliteConnection,
) -> Result<data::Note, String> {
    use schema::notes::dsl::*;

    let result = diesel::update(notes.filter(id.eq(&note_id)))
        .set((
            title.eq(update.title),
            tags.eq(tags_to_string(&update.tags)),
            updated_at.eq(to_naive(update.updated_at)),
            system_updated_at.eq(to_naive(Utc::now())),
        )).execute(connection)
        .and_then(|_num_rows| notes.find(&note_id).first(connection));

    match result {
        Ok(note) => Ok(map_note(note)),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn delete_note(note_id: String, connection: &SqliteConnection) -> Result<(), String> {
    use schema::notes::dsl::*;

    let result = diesel::delete(notes.filter(id.eq(note_id))).execute(connection);

    match result {
        Ok(_num_rows) => Ok(()),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn create_content_block(
    content_block: data::NewContentBlock,
    conn: &SqliteConnection,
) -> Result<data::ContentBlock, String> {
    use schema::content_blocks::dsl::*;

    let now = Utc::now();

    let (content_string, content_type) = content_to_string(content_block.content);

    let new_content_block = NewContentBlock {
        id: content_block.id.unwrap_or_else(repo_id::generate),
        type_: content_type,
        content: content_string,
        created_at: to_naive(content_block.created_at),
        updated_at: to_naive(content_block.updated_at),
        system_updated_at: to_naive(now),
        note_id: content_block.note_id,
    };

    let result = conn.transaction::<ContentBlock, _, _>(|| {
        diesel::insert_into(content_blocks)
            .values(&new_content_block)
            .execute(conn)?;

        content_blocks.order(system_updated_at.desc()).first(conn)
    });

    match result {
        Ok(content_block) => Ok(map_content_block(content_block)),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn update_content_block(
    content_block_id: &str,
    update: data::ContentBlockUpdate,
    connection: &SqliteConnection,
) -> Result<data::ContentBlock, String> {
    use schema::content_blocks::dsl::*;

    let (content_string, content_type) = content_to_string(update.content);

    let result = diesel::update(content_blocks.filter(id.eq(&content_block_id)))
        .set((
            content.eq(content_string),
            type_.eq(content_type.to_string()),
            system_updated_at.eq(to_naive(Utc::now())),
        )).execute(connection)
        .and_then(|_num_rows| content_blocks.find(&content_block_id).first(connection));

    match result {
        Ok(content_block) => Ok(map_content_block(content_block)),
        Err(err) => Err(format!("{}", err)),
    }
}

pub fn delete_contentblock(
    content_block_id: String,
    connection: &SqliteConnection,
) -> Result<(), String> {
    use schema::content_blocks::dsl::*;

    let result = diesel::delete(content_blocks.filter(id.eq(content_block_id))).execute(connection);

    match result {
        Ok(_num_rows) => Ok(()),
        Err(err) => Err(format!("{}", err)),
    }
}

fn content_to_string(content: data::Content) -> (String, String) {
    let (content_, content_type) = match content {
        data::Content::Text { text } => (Content::Text { text }, "text"),
        data::Content::Code { language, code } => (Content::Code { language, code }, "code"),
    };
    let content_string = serde_json::to_string(&content_).unwrap(); // FIXME

    (content_string, content_type.to_string())
}

fn tags_to_string(tags: &[data::Tag]) -> String {
    tags.join(",")
}

fn tags_from_string(tags: &str) -> Vec<data::Tag> {
    tags.split(',').map(String::from).collect()
}

fn to_naive(date_time: DateTime<Utc>) -> NaiveDateTime {
    date_time.naive_utc()
}

// Convert to UTC DateTime. This is assuming that the
// NaiveDateTimes stored in the DB are actually UTC!
fn to_utc(date_time: NaiveDateTime) -> DateTime<Utc> {
    DateTime::<Utc>::from_utc(date_time, Utc)
}
