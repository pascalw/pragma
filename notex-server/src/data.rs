use chrono::prelude::*;

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct Notebook {
    pub id: String,
    pub name: String,
    pub created_at: DateTime<Utc>,
    pub system_updated_at: DateTime<Utc>,
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct NewNotebook {
    pub id: Option<String>,
    pub name: String,
    pub created_at: DateTime<Utc>,
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct NotebookUpdate {
    pub name: String,
}

impl TypeIdentifiable for Notebook {
    fn type_name(&self) -> &'static str {
        "Notebook"
    }
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct Note {
    pub id: String,
    pub title: String,
    pub tags: Vec<Tag>,
    pub notebook_id: String,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
    pub system_updated_at: DateTime<Utc>,
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct NewNote {
    pub id: Option<String>,
    pub title: String,
    pub tags: Vec<Tag>,
    pub created_at: DateTime<Utc>,
    pub notebook_id: String,
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct NoteUpdate {
    pub title: String,
    pub tags: Vec<Tag>,
    pub updated_at: DateTime<Utc>,
}

impl TypeIdentifiable for Note {
    fn type_name(&self) -> &'static str {
        "Note"
    }
}

#[derive(Serialize, Deserialize, Debug)]
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
    pub id: String,
    pub content: Content,
    pub system_updated_at: DateTime<Utc>,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
    pub note_id: String,
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct NewContentBlock {
    pub id: Option<String>,
    pub content: Content,
    pub created_at: DateTime<Utc>,
    pub note_id: String,
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
pub struct ContentBlockUpdate {
    pub content: Content,
    pub updated_at: DateTime<Utc>,
}

impl TypeIdentifiable for ContentBlock {
    fn type_name(&self) -> &'static str {
        "ContentBlock"
    }
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Deletion {
    pub id: String,
    #[serde(rename = "type")]
    pub type_: String,
    pub resource_id: String,
    pub system_updated_at: DateTime<Utc>,
}

pub trait TypeIdentifiable {
    fn type_name(&self) -> &'static str;
}
