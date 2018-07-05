extern crate actix_web;
use actix_web::{server, App};

#[macro_use]
extern crate rust_embed;
extern crate mime_guess;

#[macro_use]
extern crate log;
extern crate env_logger;

use std::env;

mod assets;

fn main() {
    configure_logger();

    let port = port();

    server::new(|| App::new().resource("/{path:.*}", |r| r.f(assets::handler)))
        .bind(format!("127.0.0.1:{}", port))
        .expect(&format!("Can not bind to port {}", port))
        .run();
}

fn configure_logger() {
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .init();
}

fn port() -> String {
    env::var("PORT").unwrap_or("8000".to_string())
}
