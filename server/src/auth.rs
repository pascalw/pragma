use lazy_static::lazy_static;
use log::warn;
use rand::distributions::Alphanumeric;
use rand::{thread_rng, Rng};
use std::env;

use actix_web::http::StatusCode;
use actix_web::middleware::{Middleware, Response, Started};
use actix_web::{error, HttpRequest, HttpResponse, Result};

lazy_static! {
    static ref AUTH_TOKEN: String = env::var("AUTH_TOKEN").unwrap_or_else(|_| {
        let token = thread_rng().sample_iter(&Alphanumeric).take(32).collect();

        warn!("No AUTH_TOKEN was specified, using random token {}", token);

        token
    });
}

pub struct AuthMiddleware;

pub fn init() {
    // Force avaluation of the AUTH_TOKEN, so it's printed on the console
    // if no value was specified.
    let _ = *AUTH_TOKEN;
}

#[allow(clippy::needless_pass_by_value)]
/* Dummy route that will return OK, and unauthorized by below middleware. */
pub fn check_token<S>(_req: HttpRequest<S>) -> HttpResponse {
    HttpResponse::Ok().status(StatusCode::NO_CONTENT).finish()
}

fn extract_bearer_token<S>(req: &HttpRequest<S>) -> Option<String> {
    req.headers()
        .get("Authorization")
        .and_then(|header| header.to_str().ok())
        .and_then(|header_value| header_value.split(char::is_whitespace).last())
        .map(|string| string.to_owned())
}

impl<S> Middleware<S> for AuthMiddleware {
    fn start(&self, _req: &HttpRequest<S>) -> Result<Started> {
        Ok(Started::Done)
    }

    fn response(&self, req: &HttpRequest<S>, resp: HttpResponse) -> Result<Response> {
        match extract_bearer_token(req) {
            Some(ref token) if token == &*AUTH_TOKEN => Ok(Response::Done(resp)),

            _ => Err(error::ErrorUnauthorized("Unauthorized")),
        }
    }
}
