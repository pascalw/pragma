table! {
    content_blocks (id) {
        id -> Text,
        #[sql_name = "type"]
        type_ -> Text,
        content -> Text,
        note_id -> Text,
        created_at -> Timestamp,
        updated_at -> Timestamp,
        system_updated_at -> Timestamp,
    }
}

table! {
    deletions (type_, resource_id) {
        #[sql_name = "type"]
        type_ -> Text,
        resource_id -> Text,
        system_updated_at -> Timestamp,
    }
}

table! {
    notebooks (id) {
        id -> Text,
        name -> Text,
        created_at -> Timestamp,
        system_updated_at -> Timestamp,
    }
}

table! {
    notes (id) {
        id -> Text,
        title -> Text,
        tags -> Nullable<Text>,
        notebook_id -> Text,
        created_at -> Timestamp,
        updated_at -> Timestamp,
        system_updated_at -> Timestamp,
    }
}

joinable!(content_blocks -> notes (note_id));
joinable!(notes -> notebooks (notebook_id));

allow_tables_to_appear_in_same_query!(content_blocks, deletions, notebooks, notes,);
