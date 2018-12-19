#[macro_use]
extern crate serde_derive;

#[macro_use]
extern crate diesel;
#[macro_use]
extern crate diesel_migrations;

// <EMBEDDED ASSETS>

#[cfg(feature = "embedded_assets")]
#[macro_use]
extern crate rust_embed;

#[cfg(feature = "embedded_assets")]
pub mod assets;

// </EMBEDDED ASSETS>

mod actix_state;
mod api;
pub mod auth;
mod build_info;
mod data;
mod repo;
mod repo_actor;
mod repo_connection;
mod repo_id;
mod schema;

use self::actix_state::State;
use ::actix::{prelude::*, SystemRunner};
use actix_web::{server, App};
use listenfd::ListenFd;
use openssl::ssl::{SslAcceptor, SslAcceptorBuilder, SslFiletype, SslMethod};
use std::env;

pub struct Config {
    pub port: String,
    pub auth_token: String,
    pub database_url: String,
}

pub fn build(config: Config) -> SystemRunner {
    let pool = init_repo(&config.database_url);

    let port = config.port;
    let auth_token = config.auth_token;

    let sys = actix::System::new("pragma");
    let mut server =
        server::HttpServer::new(move || build_actix_app(pool.clone(), auth_token.clone()));

    let mut listenfd = ListenFd::from_env();

    server = if let Ok(Some(listener)) = listenfd.take_tcp_listener(0) {
        match env::var("SSL") {
            Ok(_) => server.listen_ssl(listener, ssl_acceptor()).unwrap(),
            Err(_) => server.listen(listener),
        }
    } else {
        let host = host();

        match env::var("SSL") {
            Ok(_) => server.bind_ssl(format!("{}:{}", host, port), ssl_acceptor()),
            Err(_) => server.bind(format!("{}:{}", host, port)),
        }
        .unwrap_or_else(|_| panic!("Can not bind to {}:{}", host, port))
    };

    server.start();
    sys
}

fn build_actix_app(pool: repo_connection::Pool, auth_token: String) -> App<State> {
    let addr = SyncArbiter::start(1, move || repo_actor::DbExecutor(pool.clone()));

    let mut app = App::with_state(State { db: addr.clone() });

    app = api::mount(app, auth_token);
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

fn init_repo(database_url: &str) -> repo_connection::Pool {
    let pool = repo_connection::create_pool(database_url);

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

fn host() -> String {
    env::var("LISTEN_HOST").unwrap_or_else(|_| "127.0.0.1".to_string())
}
