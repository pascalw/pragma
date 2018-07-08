use chrono::prelude::*;

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Notebook {
    pub id: u64,
    pub name: String,
    pub created_at: DateTime<Utc>,
    pub system_updated_at: DateTime<Utc>,
}

impl TypeIdentifiable for Notebook {
    fn type_name(&self) -> &'static str {
        "Notebook"
    }
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Note {
    pub id: u64,
    pub title: String,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
    pub tags: Vec<Tag>,
    pub system_updated_at: DateTime<Utc>,
    pub notebook_id: u64,
}

impl TypeIdentifiable for Note {
    fn type_name(&self) -> &'static str {
        "Note"
    }
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
#[serde(tag = "type", content = "data")]
pub enum Content {
    Text { text: String },
    Code { language: String, code: String },
}

pub type Tag = String;

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ContentBlock {
    pub id: u64,
    pub content: Content,
    pub system_updated_at: DateTime<Utc>,
    pub note_id: u64,
}

impl TypeIdentifiable for ContentBlock {
    fn type_name(&self) -> &'static str {
        "ContentBlock"
    }
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Deletion {
    pub _type: String,
    pub id: u64,
    pub system_updated_at: DateTime<Utc>,
}

pub trait TypeIdentifiable {
    fn type_name(&self) -> &'static str;
}
