use chrono::prelude::*;
use notebooks::data::*;

pub fn notebooks<'a>(since_revision: DateTime<Utc>) -> Result<Vec<Notebook>, &'a str> {
    let now = Utc::now();

    let notebook = Notebook {
        id: 1,
        name: "Pascal".into(),
        created_at: now,
        system_updated_at: now,
    };

    Ok(vec![notebook])
}

pub fn notes<'a>(since_revision: DateTime<Utc>) -> Result<Vec<Note>, &'a str> {
    let now = Utc::now();

    let note = Note {
        id: 1,
        title: "Test note 1".into(),
        created_at: now,
        updated_at: now,
        tags: vec![],
        system_updated_at: now,
        notebook_id: 1,
    };

    Ok(vec![note])
}

pub fn content_blocks<'a>(since_revision: DateTime<Utc>) -> Result<Vec<ContentBlock>, &'a str> {
    let now = Utc::now();

    let content1 = ContentBlock {
        id: 1,
        content: Content::Text {
            text: "Hello <strong> world</strong>!".into(),
        },
        note_id: 1,
        system_updated_at: now,
    };

    let content2 = ContentBlock {
        id: 2,
        content: Content::Code {
            language: "javascript".into(),
            code: "alert('hello world!');".into(),
        },
        note_id: 1,
        system_updated_at: now,
    };

    Ok(vec![content1, content2])
}

pub fn deletions<'a>(since_revision: DateTime<Utc>) -> Result<Vec<Deletion>, &'a str> {
    let now = Utc::now();

    let deletion = Deletion {
        _type: "Note".into(),
        id: 1,
        system_updated_at: now,
    };

    Ok(vec![deletion])
}
