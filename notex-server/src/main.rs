extern crate listenfd;

extern crate actix;
extern crate actix_web;

extern crate chrono;
extern crate env_logger;

#[macro_use]
extern crate log;
extern crate num_cpus;
extern crate rand;

#[macro_use]
extern crate lazy_static;

#[macro_use]
extern crate serde_derive;
extern crate serde_json;

#[macro_use]
extern crate diesel;
#[macro_use]
extern crate diesel_migrations;
extern crate r2d2;

extern crate nanoid;

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
use listenfd::ListenFd;
use std::env;

mod actix_state;
mod api;
mod auth;
mod build_info;
mod data;
mod repo;
mod repo_actor;
mod repo_connection;
mod repo_id;
mod schema;

fn main() {
    configure_logger();
    auth::init();
    let pool = init_repo();

    let sys = actix::System::new("notex-server");
    let mut server = server::HttpServer::new(move || build_actix_app(pool.clone()));

    let mut listenfd = ListenFd::from_env();
    server = if let Ok(Some(listener)) = listenfd.take_tcp_listener(0) {
        server.listen(listener)
    } else {
        let port = port();
        server
            .bind(format!("127.0.0.1:{}", port))
            .unwrap_or_else(|_| panic!("Can not bind to port {}", port))
    };

    server.start();
    let _ = sys.run();
}

fn build_actix_app(pool: repo_connection::Pool) -> App<State> {
    let addr = SyncArbiter::start(1, move || repo_actor::DbExecutor(pool.clone()));

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
    assets::mount(app)
}

fn configure_logger() {
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .init();
}

fn init_repo() -> repo_connection::Pool {
    let database_url = env::var("DATABASE_URL").unwrap_or_else(|_| "notex.sqlite".to_owned());
    let pool = repo_connection::create_pool(&database_url);

    let connection = pool.get().unwrap();
    repo::run_migrations(&connection);

    pool
}

fn port() -> String {
    env::var("PORT").unwrap_or_else(|_| "8000".to_string())
}
