extern crate listenfd;

extern crate actix;
extern crate actix_web;
extern crate openssl;

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
extern crate regex;
#[cfg(feature = "embedded_assets")]
mod assets;

// </EMBEDDED ASSETS>

use actix::prelude::*;
use actix_state::State;
use actix_web::{server, App};
use listenfd::ListenFd;
use openssl::ssl::{SslAcceptor, SslAcceptorBuilder, SslFiletype, SslMethod};
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

    let sys = actix::System::new("pragma");
    let mut server = server::HttpServer::new(move || build_actix_app(pool.clone()));

    let mut listenfd = ListenFd::from_env();

    server = if let Ok(Some(listener)) = listenfd.take_tcp_listener(0) {
        match env::var("SSL") {
            Ok(_) => server.listen_ssl(listener, ssl_acceptor()).unwrap(),
            Err(_) => server.listen(listener),
        }
    } else {
        let host = host();
        let port = port();

        match env::var("SSL") {
            Ok(_) => server.bind_ssl(format!("{}:{}", host, port), ssl_acceptor()),
            Err(_) => server.bind(format!("{}:{}", host, port)),
        }.unwrap_or_else(|_| panic!("Can not bind to {}:{}", host, port))
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
    let database_url = env::var("DATABASE_URL").unwrap_or_else(|_| "pragma.sqlite".to_owned());
    let pool = repo_connection::create_pool(&database_url);

    let connection = pool.get().unwrap();
    repo::setup(&connection);

    pool
}

fn ssl_acceptor() -> SslAcceptorBuilder {
    let key_file = env::var("SSL_KEY").expect("Missing SSL_KEY environment variable.");
    let cert_file = env::var("SSL_CERT").expect("Missing SSL_CERT environment variable");

    let mut builder = SslAcceptor::mozilla_intermediate(SslMethod::tls()).unwrap();
    builder
        .set_private_key_file(key_file, SslFiletype::PEM)
        .unwrap();
    builder.set_certificate_chain_file(cert_file).unwrap();

    builder
}

fn port() -> String {
    env::var("PORT").unwrap_or_else(|_| "8000".to_string())
}

fn host() -> String {
    env::var("LISTEN_HOST").unwrap_or_else(|_| "127.0.0.1".to_string())
}
