use actix::prelude::*;
use chrono::prelude::*;
use data::*;
use diesel::prelude::*;
use repo;

pub struct DbExecutor(pub SqliteConnection);

impl Actor for DbExecutor {
    type Context = SyncContext<Self>;
}

// Start GetNoteBooks

pub struct GetNoteBooksMessage {
    pub since_revision: DateTime<Utc>,
}

impl Message for GetNoteBooksMessage {
    type Result = Result<Vec<Notebook>, String>;
}

impl Handler<GetNoteBooksMessage> for DbExecutor {
    type Result = Result<Vec<Notebook>, String>;

    fn handle(&mut self, msg: GetNoteBooksMessage, _: &mut Self::Context) -> Self::Result {
        let connection = &self.0;
        repo::notebooks(msg.since_revision, connection)
    }
}

// End GetNoteBooks

// Start GetNotes

pub struct GetNotesMessage {
    pub since_revision: DateTime<Utc>,
}

impl Message for GetNotesMessage {
    type Result = Result<Vec<Note>, String>;
}

impl Handler<GetNotesMessage> for DbExecutor {
    type Result = Result<Vec<Note>, String>;

    fn handle(&mut self, msg: GetNotesMessage, _: &mut Self::Context) -> Self::Result {
        let connection = &self.0;
        repo::notes(msg.since_revision, connection)
    }
}

// End GetNotes

// Start GetContentBlocks

pub struct GetContentBlocksMessage {
    pub since_revision: DateTime<Utc>,
}

impl Message for GetContentBlocksMessage {
    type Result = Result<Vec<ContentBlock>, String>;
}

impl Handler<GetContentBlocksMessage> for DbExecutor {
    type Result = Result<Vec<ContentBlock>, String>;

    fn handle(&mut self, msg: GetContentBlocksMessage, _: &mut Self::Context) -> Self::Result {
        let connection = &self.0;
        repo::content_blocks(msg.since_revision, &connection)
    }
}

// End GetContentBlocks

// Start GetDeletions

pub struct GetDeletionsMessage {
    pub since_revision: DateTime<Utc>,
}

impl Message for GetDeletionsMessage {
    type Result = Result<Vec<Deletion>, String>;
}

impl Handler<GetDeletionsMessage> for DbExecutor {
    type Result = Result<Vec<Deletion>, String>;

    fn handle(&mut self, msg: GetDeletionsMessage, _: &mut Self::Context) -> Self::Result {
        // let connection = &self.0;
        repo::deletions(msg.since_revision)
    }
}

// End GetDeletions
