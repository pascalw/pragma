extern crate actix;
extern crate actix_web;
extern crate chrono;
extern crate env_logger;
extern crate log;
extern crate num_cpus;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate serde_derive;
#[macro_use]
extern crate serde_json;

#[macro_use]
extern crate diesel;
#[macro_use]
extern crate diesel_migrations;

// <EMBEDDED ASSETS>

#[cfg(feature = "embedded_assets")]
#[macro_use]
extern crate rust_embed;
#[cfg(feature = "embedded_assets")]
mod assets;

// </EMBEDDED ASSETS>

use actix::prelude::*;
use actix_state::State;
use actix_web::{server, App};
use diesel::sqlite::SqliteConnection;
use std::env;

mod actix_state;
mod api;
mod build_info;
mod data;
mod repo;
mod repo_actor;
mod schema;

fn main() {
    configure_logger();
    let port = port();

    let sys = actix::System::new("notex-server");
    init_repo();

    server::HttpServer::new(|| build_actix_app())
        .bind(format!("127.0.0.1:{}", port))
        .expect(&format!("Can not bind to port {}", port))
        .start();

    let _ = sys.run();
}

fn build_actix_app() -> App<State> {
    let addr = SyncArbiter::start(num_cpus::get(), || {
        let connection = establish_repo_connection();
        repo_actor::DbExecutor(connection)
    });

    let mut app = App::with_state(State { db: addr.clone() });

    app = api::mount(app);
    maybe_serve_embedded_assets(app)
}

#[cfg(not(feature = "embedded_assets"))]
fn maybe_serve_embedded_assets(app: App<State>) -> App<State> {
    app
}

#[cfg(feature = "embedded_assets")]
fn maybe_serve_embedded_assets(app: App<State>) -> App<State> {
    app.route("/{path:.*}", http::Method::GET, assets::handler)
}

fn configure_logger() {
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .init();
}

fn init_repo() {
    let connection = establish_repo_connection();
    repo::run_migrations(&connection);
}

fn establish_repo_connection() -> SqliteConnection {
    let database_url = env::var("DATABASE_URL").expect("Missing required variable DATABASE_URL");
    repo::establish_connection(database_url)
}

fn port() -> String {
    env::var("PORT").unwrap_or("8000".to_string())
}
