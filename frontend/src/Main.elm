module Main exposing (..)

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick)
import Time exposing (Time)
import Date
import Task
import Date.Format


---- MODEL ----


type alias NoteBook =
    { name : String
    , notes : List Note
    , createdAt : Time
    }


allNotes : List NoteBook -> List Note
allNotes =
    List.concatMap (.notes)


type alias Note =
    { 
      id: Int
    , title : String
    , createdAt : Time
    , updatedAt : Time
    , contentBlocks : List Content
    , tags : List Tag
    }


type Content
    = MarkdownContent
        { text : String
        }
    | CodeContent
        { text : String
        , language : String
        }


type alias Tag =
    String


type alias Model =
    { notebooks : Maybe (List NoteBook)
    , selectedNoteBook : Maybe NoteBook
    , editingNote : Maybe Note
    }


buildNote : Int -> String -> Time -> String -> Note
buildNote id title createdAt contentString =
    let
        content =
            MarkdownContent { text = contentString }
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


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        CurrentTime time ->
            let
                noteBooks =
                    [ (NoteBook "Pascal"
                        [ (buildNote 1 "Note 1 by Pascal" time "Hello *world*!")
                        , (buildNote 2 "Note 2 by Pascal" time "Hello *world*!")
                        ]
                        time
                      )
                    , (NoteBook "Testing"
                        [ (buildNote 3 "Note 1 for Testing" time "Hello **Testing**!")
                        , (buildNote 4 "Note 2 for Testing" time "Hello **Testing**!")
                        ]
                        time
                      )
                    ]
            in
                ( { model | notebooks = (Just noteBooks), selectedNoteBook = (List.head noteBooks) }, Cmd.none )

        EditNote note ->
            ( { model | editingNote = (Just note) }, Cmd.none )

        SelectNoteBook noteBook ->
            ( { model | selectedNoteBook = (Just noteBook), editingNote = Nothing }, Cmd.none )



---- VIEW ----


timeToDateString : Time -> String
timeToDateString time =
    time
        |> Date.fromTime
        |> Date.Format.format "%d %A %Y"

noteClass : Note -> Model -> String
noteClass note model =
    if model.editingNote == (Just note) then
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
    if model.selectedNoteBook == (Just noteBook) then
        "selected"
    else
        ""

renderNoteBook : Model -> NoteBook -> Html Msg
renderNoteBook model noteBook =
    li [ onClick (SelectNoteBook noteBook), class <| noteBookClass noteBook model ]
        [ span [] [ text noteBook.name ],
          span [ class "notebooks__list__note-count" ] [ text (noteBook.notes |> List.length |> toString ) ]
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
                    div [ class "content" ]
                        [ ul [] (List.map (renderNoteBook model) noteBooks)
                        ]
    in
        noteBooksContainer content


renderNotesContainer : Model -> Html Msg
renderNotesContainer model =
    div [ class "notes" ]
        [ case model.selectedNoteBook of
            Nothing ->
                div [] []

            Just noteBook ->
                div []
                    [ noteBook.notes |> renderNotes(model)
                    ]
        ]


renderNoteEditor : Model -> Html Msg
renderNoteEditor model =
    div [ class "editor" ]
        [ case model.editingNote of
            Nothing ->
                text "No note selected."

            Just note ->
                div [ style [("display", "none")] ] [
                    text note.title
                ]
        ]


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
