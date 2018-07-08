use chrono::prelude::*;
use notebooks::data::*;

pub fn seed_data() -> (Vec<Notebook>, Vec<Note>, Vec<ContentBlock>) {
    let now = Utc::now();

    let notebook1 = Notebook {
        id: 1,
        name: "Pascal".into(),
        created_at: now,
        system_updated_at: now,
    };

    let note1 = Note {
        id: 1,
        title: "Test note 1".into(),
        created_at: now,
        updated_at: now,
        tags: vec![],
        system_updated_at: now,
        notebook_id: 1,
    };

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

    (vec![notebook1], vec![note1], vec![content1, content2])
}
