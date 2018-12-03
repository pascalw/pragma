extern crate futures;

use actix_web::http::Method;
use actix_web::{App, AsyncResponder, Error, HttpRequest, HttpResponse, Json, Path, Query};
use chrono::prelude::*;

use self::futures::future::Future;
use actix_state::State;
use auth;
use build_info;
use data::*;
use repo_actor::*;

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct DataResponse {
    revision: DateTime<Utc>,
    deletions: Vec<Resource>,
    changes: Changes,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Changes {
    notebooks: Vec<Notebook>,
    notes: Vec<Note>,
    content_blocks: Vec<ContentBlock>,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Resource {
    id: String,

    #[serde(rename = "type")]
    type_: String,
}

#[derive(Deserialize)]
struct GetDataQuery {
    since_revision: Option<DateTime<Utc>>,
}

pub fn mount(app: App<State>) -> App<State> {
    #[cfg_attr(rustfmt, rustfmt_skip)]
    app.scope("/api", |scope| {
       scope.middleware(auth::AuthMiddleware)
            .route("/auth", Method::POST, auth::check_token)
            .route("/data", Method::GET, get_data)
            .route("/notes", Method::POST, create_note)
            .route("/notes/{id}", Method::PUT, update_note)
            .route("/notes/{id}", Method::DELETE, delete_note)
            .route("/notebooks", Method::POST, create_notebook)
            .route("/notebooks/{id}", Method::PUT, update_notebook)
            .route("/notebooks/{id}", Method::DELETE, delete_notebook)
            .route("/content_blocks", Method::POST, create_content_block)
            .route("/content_blocks/{id}", Method::PUT, update_content_block)
            .route("/content_blocks/{id}", Method::DELETE, delete_content_block)
    })
    .route("/version", Method::GET, |_: HttpRequest<State>|
        build_info::build_version()
    )
}

fn create_notebook(
    (req, new_notebook): (HttpRequest<State>, Json<NewNotebook>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let db = &req.state().db;

    db.send(CreateNotebookMessage {
        new_notebook: new_notebook.into_inner(),
    }).from_err()
    .and_then(move |res| match res {
        Ok(notebook) => Ok(HttpResponse::Ok().json(notebook)),
        Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
    }).responder()
}

fn create_note(
    (req, new_note): (HttpRequest<State>, Json<NewNote>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let db = &req.state().db;

    db.send(CreateNoteMessage {
        new_note: new_note.into_inner(),
    }).from_err()
    .and_then(move |res| match res {
        Ok(note) => Ok(HttpResponse::Ok().json(note)),
        Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
    }).responder()
}

fn create_content_block(
    (req, new_content_block): (HttpRequest<State>, Json<NewContentBlock>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let db = &req.state().db;

    db.send(CreateContentBlockMessage {
        new_content_block: new_content_block.into_inner(),
    }).from_err()
    .and_then(move |res| match res {
        Ok(content_block) => Ok(HttpResponse::Ok().json(content_block)),
        Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
    }).responder()
}

fn update_content_block(
    (req, params, content_block_update): (
        HttpRequest<State>,
        Path<String>,
        Json<ContentBlockUpdate>,
    ),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let id = params.into_inner();

    let db = &req.state().db;

    db.send(UpdateContentBlockMessage {
        id,
        update: content_block_update.into_inner(),
    }).from_err()
    .and_then(move |res| match res {
        Ok(updated_content_block) => Ok(HttpResponse::Ok().json(updated_content_block)),
        Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
    }).responder()
}

fn delete_content_block(
    (req, params): (HttpRequest<State>, Path<String>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let id = params.into_inner();

    let db = &req.state().db;

    db.send(DeleteContentBlockMessage { id })
        .from_err()
        .and_then(move |res| match res {
            Ok(_) => Ok(HttpResponse::Ok().finish()),
            Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
        }).responder()
}

fn update_note(
    (req, params, note_update): (HttpRequest<State>, Path<String>, Json<NoteUpdate>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let id = params.into_inner();

    let db = &req.state().db;

    db.send(UpdateNoteMessage {
        id,
        update: note_update.into_inner(),
    }).from_err()
    .and_then(move |res| match res {
        Ok(updated_note) => Ok(HttpResponse::Ok().json(updated_note)),
        Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
    }).responder()
}

fn delete_note(
    (req, params): (HttpRequest<State>, Path<String>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let id = params.into_inner();

    let db = &req.state().db;

    db.send(DeleteNoteMessage { id })
        .from_err()
        .and_then(move |res| match res {
            Ok(_) => Ok(HttpResponse::Ok().finish()),
            Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
        }).responder()
}

fn update_notebook(
    (req, params, notebook_update): (HttpRequest<State>, Path<String>, Json<NotebookUpdate>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let id = params.into_inner();

    let db = &req.state().db;

    db.send(UpdateNotebookMessage {
        id,
        update: notebook_update.into_inner(),
    }).from_err()
    .and_then(move |res| match res {
        Ok(updated_notebook) => Ok(HttpResponse::Ok().json(updated_notebook)),
        Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
    }).responder()
}

fn delete_notebook(
    (req, params): (HttpRequest<State>, Path<String>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let id = params.into_inner();

    let db = &req.state().db;

    db.send(DeleteNotebookMessage { id })
        .from_err()
        .and_then(move |res| match res {
            Ok(_) => Ok(HttpResponse::Ok().finish()),
            Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
        }).responder()
}

fn get_data(
    (req, query): (HttpRequest<State>, Query<GetDataQuery>),
) -> Box<Future<Item = HttpResponse, Error = Error>> {
    let db = &req.state().db;

    let req_1 = db.send(GetNoteBooksMessage {
        since_revision: query.since_revision,
    });
    let req_2 = db.send(GetNotesMessage {
        since_revision: query.since_revision,
    });
    let req_3 = db.send(GetContentBlocksMessage {
        since_revision: query.since_revision,
    });
    let req_4 = db.send(GetDeletionsMessage {
        since_revision: query.since_revision,
    });

    req_1
        .join4(req_2, req_3, req_4)
        .from_err()
        .and_then(move |res| match res {
            (Ok(notebooks), Ok(notes), Ok(content_blocks), Ok(deleted_records)) => {
                let data_response =
                    build_response(notebooks, notes, content_blocks, deleted_records);
                Ok(HttpResponse::Ok().json(data_response))
            }
            _ => Ok(HttpResponse::InternalServerError().into()),
        }).responder()
}

fn build_response(
    notebooks: Vec<Notebook>,
    notes: Vec<Note>,
    content_blocks: Vec<ContentBlock>,
    deleted_records: Vec<Deletion>,
) -> DataResponse {
    let deletions: Vec<Resource> = deleted_records
        .into_iter()
        .map(|d| Resource {
            id: d.resource_id,
            type_: d.type_,
        }).collect();

    let revision = revision(&notebooks, &notes, &content_blocks);

    DataResponse {
        revision,
        deletions,
        changes: Changes {
            notebooks,
            notes,
            content_blocks,
        },
    }
}

fn revision(
    notebooks: &[Notebook],
    notes: &[Note],
    content_blocks: &[ContentBlock],
) -> DateTime<Utc> {
    let iter1 = notebooks.iter().map(|n| n.revision);
    let iter2 = notes.iter().map(|n| n.revision);
    let iter3 = content_blocks.iter().map(|c| c.revision);

    iter1
        .chain(iter2)
        .chain(iter3)
        .max()
        .unwrap_or_else(Utc::now)
}
