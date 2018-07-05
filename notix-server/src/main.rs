extern crate actix_web;
use actix_web::{http, server, App, HttpRequest};

extern crate mime_guess;

extern crate env_logger;
extern crate log;
#[macro_use]
extern crate lazy_static;

use std::env;

#[cfg(feature = "embedded_assets")]
#[macro_use]
extern crate rust_embed;
#[cfg(feature = "embedded_assets")]
mod assets;

mod build_info;

fn main() {
    configure_logger();

    let port = port();

    server::new(|| build_actix_app())
        .bind(format!("127.0.0.1:{}", port))
        .expect(&format!("Can not bind to port {}", port))
        .run();
}

fn build_actix_app() -> App {
    let app = App::new().route("/version", http::Method::GET, |_: HttpRequest| {
        build_info::build_version()
    });

    maybe_serve_embedded_assets(app)
}

#[cfg(not(feature = "embedded_assets"))]
fn maybe_serve_embedded_assets(app: App) -> App {
    app
}

#[cfg(feature = "embedded_assets")]
fn maybe_serve_embedded_assets(app: App) -> App {
    app.route("/{path:.*}", http::Method::GET, assets::handler)
}

fn configure_logger() {
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .init();
}

fn port() -> String {
    env::var("PORT").unwrap_or("8000".to_string())
}
