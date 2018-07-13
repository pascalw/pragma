extern crate futures;

use actix_web::http::Method;
use actix_web::{App, AsyncResponder, Error, HttpRequest, HttpResponse, Query};
use chrono::prelude::*;
use serde_json::Value;

use self::futures::Future;
use actix_state::State;
use build_info;
use data::TypeIdentifiable;
use data::*;
use repo_actor::*;

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct DataResponse {
    revision: DateTime<Utc>,
    deletions: Vec<Resource>,
    changes: Vec<DataChange>,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct DataChange {
    resource: Resource,
    data: Value,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Resource {
    id: i32,

    #[serde(rename = "type")]
    _type: String,
}

#[derive(Deserialize)]
struct GetDataQuery {
    since_revision: Option<DateTime<Utc>>,
}

pub fn mount(app: App<State>) -> App<State> {
    #[cfg_attr(rustfmt, rustfmt_skip)]
    app.route("/api/data", Method::GET, get_data)
       .route("/version", Method::GET, |_: HttpRequest<State>|
           build_info::build_version()
       )
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
                    build_response(&notebooks, &notes, &content_blocks, &deleted_records);
                Ok(HttpResponse::Ok().json(data_response))
            }
            _ => Ok(HttpResponse::InternalServerError().into()),
        })
        .responder()
}

fn build_response(
    notebooks: &[Notebook],
    notes: &[Note],
    content_blocks: &[ContentBlock],
    deleted_records: &[Deletion],
) -> DataResponse {
    let notebook_changes: Vec<DataChange> = notebooks
        .iter()
        .map(|n| DataChange {
            data: json!(n),
            resource: Resource {
                id: n.id,
                _type: n.type_name().into(),
            },
        })
        .collect();

    let mut note_changes: Vec<DataChange> = notes
        .iter()
        .map(|n| DataChange {
            data: json!(n),
            resource: Resource {
                id: n.id,
                _type: n.type_name().into(),
            },
        })
        .collect();

    let mut content_changes: Vec<DataChange> = content_blocks
        .iter()
        .map(|c| DataChange {
            data: json!(c),
            resource: Resource {
                id: c.id,
                _type: c.type_name().into(),
            },
        })
        .collect();

    let mut changes = notebook_changes;
    changes.append(&mut note_changes);
    changes.append(&mut content_changes);

    let deletions: Vec<Resource> = deleted_records
        .iter()
        .map(|d| Resource {
            id: d.id,
            _type: d._type.clone(),
        })
        .collect();

    let iter1 = notebooks.iter().map(|n| n.system_updated_at);
    let iter2 = notes.iter().map(|n| n.system_updated_at);
    let iter3 = content_blocks.iter().map(|c| c.system_updated_at);

    // FIXME include deletions
    let revision = iter1
        .chain(iter2)
        .chain(iter3)
        .max()
        .unwrap_or_else(Utc::now);

    DataResponse {
        revision,
        deletions,
        changes,
    }
}
