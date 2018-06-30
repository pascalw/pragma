module Main exposing (..)

import Html exposing (Html, text, div, h1, img, ul, li)
import Time exposing (Time)
import Date
import Task


---- MODEL ----


type alias NoteBook =
    { name : String
    , notes : List Note
    , createdAt : Time
    }


type alias Note =
    { title : String
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
    }


buildNote : String -> Time -> String -> Note
buildNote title createdAt contentString =
    let
        content =
            MarkdownContent { text = contentString }
    in
        Note title createdAt createdAt [ content ] []


buildSeedNoteBook : Time -> NoteBook
buildSeedNoteBook createdAt =
    let
        note =
            buildNote "My first note" createdAt "Hello *world*!"
    in
        NoteBook "Pascal" [ note ] createdAt


init : ( Model, Cmd Msg )
init =
    ( Model Nothing, Task.perform CurrentTime Time.now )



---- UPDATE ----


type Msg
    = CurrentTime Time


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        CurrentTime time ->
            let
                noteBook =
                    buildSeedNoteBook time
            in
                ( { model | notebooks = (Just [ noteBook ]) }, Cmd.none )



---- VIEW ----


renderNote : Note -> Html Msg
renderNote note =
    li [] [ text note.title, text (note.createdAt |> Date.fromTime |> toString) ]


renderNotes : List Note -> Html Msg
renderNotes notes =
    ul [] (List.map renderNote notes)


renderNoteBook : NoteBook -> Html Msg
renderNoteBook noteBook =
    li []
        [ (text noteBook.name)
        , renderNotes noteBook.notes
        ]


renderNoteBooks : Maybe (List NoteBook) -> Html Msg
renderNoteBooks noteBooks =
    case noteBooks of
        Nothing ->
            div []
                [ text "Loading..."
                ]

        Just noteBooks ->
            div []
                [ ul [] (List.map renderNoteBook noteBooks)
                ]

layout : Model -> Html Msg
layout model =
    div [] [ ]

view : Model -> Html Msg
view model =
    div [] [
        renderNoteBooks model.notebooks
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
