extern crate actix_web;
use actix_web::{server, App};

#[macro_use]
extern crate rust_embed;
extern crate mime_guess;

mod assets;

fn main() {
    server::new(|| {
        App::new()
            .resource("/{path:.*}", |r| r.f(assets::handler))
    })
    .bind("127.0.0.1:8000")
    .expect("Can not bind to port 8000")
    .run();
}
