extern crate mime_guess;

use self::mime_guess::guess_mime_type;
use actix_state::State;
use actix_web::http::{header, StatusCode};
use actix_web::{Error, HttpRequest, HttpResponse};

#[derive(RustEmbed)]
#[folder = "assets/"]
struct Asset;

pub fn handler(req: HttpRequest<State>) -> Result<HttpResponse<State>, Error> {
    let asset_path = asset_path(&req);

    match Asset::get(asset_path) {
        Some(asset) => {
            let mime = guess_mime_type(asset_path);
            Ok(HttpResponse::Ok()
                .header(header::CONTENT_TYPE, mime)
                .body(asset))
        }
        None => Ok(HttpResponse::Ok().status(StatusCode::NOT_FOUND).finish()),
    }
}

fn asset_path<'a>(req: &'a HttpRequest) -> &'a str {
    let path = req.match_info().get("path").unwrap_or("index.html");

    match path {
        "" => "index.html",
        _ => path,
    }
}
