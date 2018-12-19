use rand::distributions::Alphanumeric;
use rand::{thread_rng, Rng};

use actix_web::http::StatusCode;
use actix_web::middleware::{Middleware, Response, Started};
use actix_web::{error, HttpRequest, HttpResponse, Result};

pub struct AuthMiddleware {
    token: String,
}

pub fn random_token() -> String {
    thread_rng().sample_iter(&Alphanumeric).take(32).collect()
}

pub fn middleware(token: String) -> AuthMiddleware {
    AuthMiddleware { token }
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
            Some(ref token) if token == &self.token => Ok(Response::Done(resp)),

            _ => Err(error::ErrorUnauthorized("Unauthorized")),
        }
    }
}
