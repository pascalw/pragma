extern crate futures;

use actix_web::http::Method;
use actix_web::{App, AsyncResponder, Error, HttpRequest, HttpResponse, Json, Path, Query};
use chrono::prelude::*;

use self::futures::future::Future;
use actix_state::State;
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
    app.route("/api/data", Method::GET, get_data)
       .route("/api/notes", Method::POST, create_note)
       .route("/api/notes/{id}", Method::PUT, update_note)
       .route("/api/notebooks", Method::POST, create_notebook)
       .route("/api/notebooks/{id}", Method::PUT, update_notebook)
       .route("/api/content_blocks", Method::POST, create_content_block)
       .route("/api/content_blocks/{id}", Method::PUT, update_content_block)
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
        })
        .responder()
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
        })
        .responder()
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
        })
        .responder()
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
            Ok(_) => Ok(HttpResponse::Ok().finish()),
            Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
        })
        .responder()
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
            Ok(_) => Ok(HttpResponse::Ok().finish()),
            Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
        })
        .responder()
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
            Ok(_) => Ok(HttpResponse::Ok().finish()),
            Err(reason) => Ok(HttpResponse::InternalServerError().body(reason)),
        })
        .responder()
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
                    build_response(notebooks, notes, content_blocks, &deleted_records);
                Ok(HttpResponse::Ok().json(data_response))
            }
            _ => Ok(HttpResponse::InternalServerError().into()),
        })
        .responder()
}

fn build_response(
    notebooks: Vec<Notebook>,
    notes: Vec<Note>,
    content_blocks: Vec<ContentBlock>,
    deleted_records: &[Deletion],
) -> DataResponse {
    let deletions: Vec<Resource> = deleted_records
        .iter()
        .map(|d| Resource {
            id: d.resource_id.to_owned(),
            type_: d.type_.to_owned(),
        })
        .collect();

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
    let iter1 = notebooks.iter().map(|n| n.system_updated_at);
    let iter2 = notes.iter().map(|n| n.system_updated_at);
    let iter3 = content_blocks.iter().map(|c| c.system_updated_at);

    iter1
        .chain(iter2)
        .chain(iter3)
        .max()
        .unwrap_or_else(Utc::now)
}
