use chrono::prelude::*;

#[derive(Serialize, Queryable, Debug)]
#[serde(rename_all = "camelCase")]
pub struct Notebook {
    pub id: i32,
    pub name: String,
    pub created_at: DateTime<Utc>,
    pub system_updated_at: DateTime<Utc>,
}

impl TypeIdentifiable for Notebook {
    fn type_name(&self) -> &'static str {
        "Notebook"
    }
}

#[derive(Serialize, Queryable)]
#[serde(rename_all = "camelCase")]
pub struct Note {
    pub id: i32,
    pub title: String,
    pub tags: Vec<Tag>,
    pub notebook_id: i32,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
    pub system_updated_at: DateTime<Utc>,
}

impl TypeIdentifiable for Note {
    fn type_name(&self) -> &'static str {
        "Note"
    }
}

#[derive(Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
#[serde(tag = "type", content = "data")]
pub enum Content {
    Text { text: String },
    Code { language: String, code: String },
}

pub type Tag = String;

#[derive(Serialize, Queryable)]
#[serde(rename_all = "camelCase")]
pub struct ContentBlock {
    pub id: i32,
    pub content: Content,
    pub system_updated_at: DateTime<Utc>,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
    pub note_id: i32,
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
    pub id: i32,
    pub system_updated_at: DateTime<Utc>,
}

pub trait TypeIdentifiable {
    fn type_name(&self) -> &'static str;
}
