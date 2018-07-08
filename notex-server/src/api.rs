use actix_web::{http, App, HttpRequest, Json, Responder};
use chrono::prelude::*;
use chrono::serde::ts_nanoseconds;
use serde_json::Value;

use notebooks::data::TypeIdentifiable;
use notebooks::repo;

pub fn mount(app: App) -> App {
    app.route("/api/data", http::Method::GET, get_data)
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct DataResponse {
    #[serde(with = "ts_nanoseconds")]
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
    id: u64,

    #[serde(rename = "type")]
    _type: String,
}

fn get_data(_req: HttpRequest) -> impl Responder {
    let now = Utc::now();

    let changed_notebooks = repo::notebooks(now);
    let changed_notes = repo::notes(now);
    let changed_content = repo::content_blocks(now);
    let deleted_records = repo::deletions(now);

    let notebook_changes: Vec<DataChange> = changed_notebooks
        .unwrap()
        .iter()
        .map(|n| DataChange {
            data: json!(n),
            resource: Resource {
                id: n.id,
                _type: n.type_name().into(),
            },
        })
        .collect();

    let mut note_changes: Vec<DataChange> = changed_notes
        .unwrap()
        .iter()
        .map(|n| DataChange {
            data: json!(n),
            resource: Resource {
                id: n.id,
                _type: n.type_name().into(),
            },
        })
        .collect();

    let mut content_changes: Vec<DataChange> = changed_content
        .unwrap()
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
        .unwrap()
        .iter()
        .map(|d| Resource {
            id: d.id,
            _type: d._type.clone(),
        })
        .collect();

    Json(DataResponse {
        revision: now,
        deletions: deletions,
        changes: changes,
    })
}
