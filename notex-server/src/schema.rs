table! {
    content_blocks (id) {
        id -> Integer,
        #[sql_name = "type"]
        type_ -> Text,
        content -> Text,
        note_id -> Integer,
        created_at -> Timestamp,
        updated_at -> Timestamp,
        system_updated_at -> Timestamp,
    }
}

table! {
    notebooks (id) {
        id -> Integer,
        name -> Text,
        created_at -> Timestamp,
        system_updated_at -> Timestamp,
    }
}

table! {
    notes (id) {
        id -> Integer,
        title -> Text,
        tags -> Nullable<Text>,
        notebook_id -> Integer,
        created_at -> Timestamp,
        updated_at -> Timestamp,
        system_updated_at -> Timestamp,
    }
}

joinable!(content_blocks -> notes (note_id));
joinable!(notes -> notebooks (notebook_id));

allow_tables_to_appear_in_same_query!(content_blocks, notebooks, notes,);
