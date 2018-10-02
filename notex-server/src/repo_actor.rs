use actix::prelude::*;
use chrono::prelude::*;
use data::*;
use repo;
use repo_connection;

pub struct DbExecutor(pub repo_connection::Pool);

impl Actor for DbExecutor {
    type Context = SyncContext<Self>;
}

// Start GetNoteBooks

pub struct GetNoteBooksMessage {
    pub since_revision: Option<DateTime<Utc>>,
}

impl Message for GetNoteBooksMessage {
    type Result = Result<Vec<Notebook>, String>;
}

impl Handler<GetNoteBooksMessage> for DbExecutor {
    type Result = Result<Vec<Notebook>, String>;

    fn handle(&mut self, msg: GetNoteBooksMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::notebooks(msg.since_revision, &connection)
    }
}

// End GetNoteBooks

// Start CreateNotebook

pub struct CreateNotebookMessage {
    pub new_notebook: NewNotebook,
}

impl Message for CreateNotebookMessage {
    type Result = Result<Notebook, String>;
}

impl Handler<CreateNotebookMessage> for DbExecutor {
    type Result = Result<Notebook, String>;

    fn handle(&mut self, msg: CreateNotebookMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::create_notebook(msg.new_notebook, &connection)
    }
}

// End CreateNotebook

// Start UpdateNotebook

pub struct UpdateNotebookMessage {
    pub id: String,
    pub update: NotebookUpdate,
}

impl Message for UpdateNotebookMessage {
    type Result = Result<(), String>;
}

impl Handler<UpdateNotebookMessage> for DbExecutor {
    type Result = Result<(), String>;

    fn handle(&mut self, msg: UpdateNotebookMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::update_notebook(msg.id, msg.update, &connection)
    }
}

// End UpdateNotebook

// Start GetNotes

pub struct GetNotesMessage {
    pub since_revision: Option<DateTime<Utc>>,
}

impl Message for GetNotesMessage {
    type Result = Result<Vec<Note>, String>;
}

impl Handler<GetNotesMessage> for DbExecutor {
    type Result = Result<Vec<Note>, String>;

    fn handle(&mut self, msg: GetNotesMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::notes(msg.since_revision, &connection)
    }
}

// End GetNotes

// Start CreateNote

pub struct CreateNoteMessage {
    pub new_note: NewNote,
}

impl Message for CreateNoteMessage {
    type Result = Result<Note, String>;
}

impl Handler<CreateNoteMessage> for DbExecutor {
    type Result = Result<Note, String>;

    fn handle(&mut self, msg: CreateNoteMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::create_note(msg.new_note, &connection)
    }
}

// End CreateNote

// Start UpdateNote

pub struct UpdateNoteMessage {
    pub id: String,
    pub update: NoteUpdate,
}

impl Message for UpdateNoteMessage {
    type Result = Result<(), String>;
}

impl Handler<UpdateNoteMessage> for DbExecutor {
    type Result = Result<(), String>;

    fn handle(&mut self, msg: UpdateNoteMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::update_note(msg.id, msg.update, &connection)
    }
}

// End UpdateNote

// Start CreateContentBlocks

pub struct CreateContentBlockMessage {
    pub new_content_block: NewContentBlock,
}

impl Message for CreateContentBlockMessage {
    type Result = Result<ContentBlock, String>;
}

impl Handler<CreateContentBlockMessage> for DbExecutor {
    type Result = Result<ContentBlock, String>;

    fn handle(&mut self, msg: CreateContentBlockMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::create_content_block(msg.new_content_block, &connection)
    }
}

// End CreateContentBlocks

// Start UpdateContentBlock

pub struct UpdateContentBlockMessage {
    pub id: String,
    pub update: ContentBlockUpdate,
}

impl Message for UpdateContentBlockMessage {
    type Result = Result<(), String>;
}

impl Handler<UpdateContentBlockMessage> for DbExecutor {
    type Result = Result<(), String>;

    fn handle(&mut self, msg: UpdateContentBlockMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::update_content_block(msg.id, msg.update, &connection)
    }
}

// End UpdateContentBlock

// Start GetContentBlocks

pub struct GetContentBlocksMessage {
    pub since_revision: Option<DateTime<Utc>>,
}

impl Message for GetContentBlocksMessage {
    type Result = Result<Vec<ContentBlock>, String>;
}

impl Handler<GetContentBlocksMessage> for DbExecutor {
    type Result = Result<Vec<ContentBlock>, String>;

    fn handle(&mut self, msg: GetContentBlocksMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::content_blocks(msg.since_revision, &connection)
    }
}

// End GetContentBlocks

// Start GetDeletions

pub struct GetDeletionsMessage {
    pub since_revision: Option<DateTime<Utc>>,
}

impl Message for GetDeletionsMessage {
    type Result = Result<Vec<Deletion>, String>;
}

impl Handler<GetDeletionsMessage> for DbExecutor {
    type Result = Result<Vec<Deletion>, String>;

    fn handle(&mut self, msg: GetDeletionsMessage, _: &mut Self::Context) -> Self::Result {
        let pool = &self.0;
        let connection = pool.get().unwrap();
        repo::deletions(msg.since_revision, &connection)
    }
}

// End GetDeletions
