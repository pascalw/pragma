use super::actix_state::State;
use actix_web::dev::HttpResponseBuilder;
use actix_web::http::Method;
use actix_web::http::{header, StatusCode};
use actix_web::{App, Error, HttpRequest, HttpResponse};
use lazy_static::lazy_static;
use mime_guess::guess_mime_type;
use regex::Regex;

lazy_static! {
    static ref RE_IS_CACHABLE_ASSET: Regex = Regex::new(r"\.(?:jpg|jpeg|png|svg|css|js)$").unwrap();
}

#[derive(RustEmbed)]
#[folder = "assets/"]
struct Asset;

pub fn mount(app: App<State>) -> App<State> {
    app.route("/{path:.*}", Method::GET, handler)
}

#[allow(clippy::needless_pass_by_value)]
pub fn handler(req: HttpRequest<State>) -> Result<HttpResponse, Error> {
    let asset_path = asset_path(&req);

    match Asset::get(asset_path) {
        Some(asset) => {
            let mime = guess_mime_type(asset_path);

            let mut response = HttpResponse::Ok();
            with_caching_headers(asset_path, &mut response);

            Ok(response.header(header::CONTENT_TYPE, mime).body(asset))
        }
        None => Ok(HttpResponse::Ok().status(StatusCode::NOT_FOUND).finish()),
    }
}

fn asset_path<'a>(req: &'a HttpRequest<State>) -> &'a str {
    let path = req.match_info().get("path").unwrap_or("index.html");

    match path {
        "" => "index.html",
        _ => path,
    }
}

fn with_caching_headers<'a>(
    path: &str,
    response: &'a mut HttpResponseBuilder,
) -> &'a mut HttpResponseBuilder {
    match path {
        "service-worker.js" => response
            .header(header::CACHE_CONTROL, "no-cache, no-store, must-revalidate")
            .header(header::EXPIRES, "0"),
        _ => {
            if RE_IS_CACHABLE_ASSET.is_match(path) {
                response.header(header::CACHE_CONTROL, "max-age=31536000, public")
            } else {
                response
            }
        }
    }
}
