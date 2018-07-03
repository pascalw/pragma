module Main exposing (..)

import Debug
import Json.Encode
import Json.Decode
import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick, on)
import Time exposing (Time)
import Date
import Task
import Date.Format


---- MODEL ----


type alias NoteBook =
    { id : Int
    , name : String
    , notes : List Note
    , createdAt : Time
    }


allNotes : List NoteBook -> List Note
allNotes =
    List.concatMap (.notes)


type alias Note =
    { id : Int
    , title : String
    , createdAt : Time
    , updatedAt : Time
    , contentBlocks : List Content
    , tags : List Tag
    }


type Content
    = TextContent String
    | CodeContent
        { text : String
        , language : String
        }


type alias Tag =
    String


type alias Model =
    { notebooks : Maybe (List NoteBook)
    , selectedNoteBook : Maybe Int
    , editingNote : Maybe Int
    }


noteById : Model -> Int -> Maybe Note
noteById model noteId =
    case model.notebooks of
        Nothing ->
            Nothing

        Just noteBooks ->
            noteBooks
                |> allNotes
                |> List.filter (\note -> note.id == noteId)
                |> List.head


noteBookById : Model -> Int -> Maybe NoteBook
noteBookById model noteBookId =
    case model.notebooks of
        Nothing ->
            Nothing

        Just noteBooks ->
            noteBooks
                |> List.filter (\noteBook -> noteBook.id == noteBookId)
                |> List.head


buildNote : Int -> String -> Time -> String -> Note
buildNote id title createdAt contentString =
    let
        content =
            TextContent contentString
    in
        Note id title createdAt createdAt [ content ] []


init : ( Model, Cmd Msg )
init =
    ( (Model Nothing Nothing Nothing), Task.perform CurrentTime Time.now )



---- UPDATE ----


type Msg
    = CurrentTime Time
    | EditNote Note
    | SelectNoteBook NoteBook
    | NoteContentChanged Note Content String


updateNote : Note -> NoteBook -> NoteBook
updateNote note noteBook =
    let
        newNotes =
            noteBook.notes
                |> List.map
                    (\listNote ->
                        if listNote.id == note.id then
                            note
                        else
                            listNote
                    )
    in
        { noteBook | notes = newNotes }

updateNoteContent : Note -> Content -> String -> Note
updateNoteContent note content text =
    let
        updatedContentBlocks =
        note.contentBlocks
        |> List.map (\c -> if c == content then (TextContent text) else c)
    in
        { note | contentBlocks = updatedContentBlocks }


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        CurrentTime time ->
            let
                noteBooks =
                    [ (NoteBook 1
                        "Pascal"
                        [ (buildNote 1 "Note 1 by Pascal" time "Hello *world*!")
                        , (buildNote 2 "Note 2 by Pascal" time "Hello *world*!")
                        ]
                        time
                      )
                    , (NoteBook 2
                        "Testing"
                        [ (buildNote 3 "Note 1 for Testing" time "Hello **Testing**!")
                        , (buildNote 4 "Note 2 for Testing" time "Hello **Testing**!")
                        ]
                        time
                      )
                    ]
            in
                ( { model | notebooks = (Just noteBooks), selectedNoteBook = (List.head noteBooks) |> Maybe.map (\n -> n.id) }, Cmd.none )

        EditNote note ->
            ( { model | editingNote = (Just note.id) }, Cmd.none )

        SelectNoteBook noteBook ->
            ( { model | selectedNoteBook = (Just noteBook.id), editingNote = Nothing }, Cmd.none )

        NoteContentChanged note content text ->
            let
                updatedNote = updateNoteContent note content text

                updatedNoteBooks =
                    Maybe.map (\notebooks -> List.map (updateNote updatedNote) notebooks) model.notebooks
            in
                ( { model | editingNote = (Just updatedNote.id), notebooks = updatedNoteBooks }, Cmd.none )



---- VIEW ----


script : String -> Html any
script code =
    node "script"
        []
        [ text code ]


timeToDateString : Time -> String
timeToDateString time =
    time
        |> Date.fromTime
        |> Date.Format.format "%d %A %Y"


noteClass : Note -> Model -> String
noteClass note model =
    if model.editingNote == (Just note.id) then
        "selected"
    else
        ""


renderNote : Model -> Note -> Html Msg
renderNote model note =
    li [ onClick (EditNote note), class (noteClass note model) ] [ text note.title, br [] [], small [] [ text (timeToDateString note.createdAt) ] ]


renderNotes : Model -> List Note -> Html Msg
renderNotes model notes =
    ul [] (List.map (renderNote model) notes)


noteBookClass : NoteBook -> Model -> String
noteBookClass noteBook model =
    if model.selectedNoteBook == (Just noteBook.id) then
        "selected"
    else
        ""


renderNoteBook : Model -> NoteBook -> Html Msg
renderNoteBook model noteBook =
    li [ onClick (SelectNoteBook noteBook), class <| noteBookClass noteBook model ]
        [ span [ class "notebooks__list_name" ] [ text noteBook.name ]
        , span [ class "notebooks__list__note-count" ] [ text (noteBook.notes |> List.length |> toString) ]
        ]


noteBooksContainer : Html Msg -> Html Msg
noteBooksContainer child =
    div [ class "notebooks" ] [ child ]


renderNoteBooks : Model -> Html Msg
renderNoteBooks model =
    let
        content =
            case model.notebooks of
                Nothing ->
                    div []
                        [ text "Loading..."
                        ]

                Just noteBooks ->
                    div []
                        [ ul [] (List.map (renderNoteBook model) noteBooks)
                        ]
    in
        noteBooksContainer content


renderNotesContainer : Model -> Html Msg
renderNotesContainer model =
    div [ class "notes" ]
        [ case model.selectedNoteBook |> Maybe.andThen (noteBookById model) of
            Nothing ->
                div [] []

            Just noteBook ->
                div []
                    [ noteBook.notes |> renderNotes (model)
                    ]
        ]


decodeNoteContentChanged : Json.Decode.Decoder String
decodeNoteContentChanged =
    Json.Decode.at [ "detail", "value" ] Json.Decode.string


noteContentInputId : Note -> String
noteContentInputId note =
    "note-" ++ (note.id |> toString) ++ "-content"


renderNoteContentBlocks : Note -> Content -> Html Msg
renderNoteContentBlocks note content =
    case content of
        TextContent contentText ->
            div [ class "content" ]
                [ input [ id <| noteContentInputId note, type_ "hidden", value contentText ] []
                , node "trix-editor" [ class "trix-content", attribute "input" <| noteContentInputId note, on "trix-change" (Json.Decode.map (NoteContentChanged note content) Html.Events.targetValue) ] []
                , script "initTrix()"
                ]

        _ ->
            div [] [ text "unsupported" ]


renderNoteEditor : Model -> Html Msg
renderNoteEditor model =
    let
        contents =
            case model.editingNote |> Maybe.andThen (noteById model) of
                Nothing ->
                    [ text "No note selected." ]

                Just note ->
                    (h2 [] [ text note.title ])
                        :: List.map (renderNoteContentBlocks note) note.contentBlocks
    in
        div [ class "editor" ] contents


layout : List (Html Msg) -> Html Msg
layout children =
    main_ [] children


columns : List (Html Msg) -> Html Msg
columns children =
    div [ class "columns" ] children


view : Model -> Html Msg
view model =
    layout
        [ columns
            [ renderNoteBooks model
            , renderNotesContainer model
            , renderNoteEditor model
            ]
        ]



---- PROGRAM ----


main : Program Never Model Msg
main =
    Html.program
        { view = view
        , init = init
        , update = update
        , subscriptions = always Sub.none
        }
