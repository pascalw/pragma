use log::warn;
use std::env;

fn main() {
    configure_logger();

    let config = pragma::Config {
        port: port(),
        auth_token: token(),
        database_url: database_url(),
    };

    let sys = pragma::build(config);
    let _ = sys.run();
}

fn configure_logger() {
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .init();
}

fn token() -> String {
    env::var("AUTH_TOKEN").unwrap_or_else(|_| {
        let token = pragma::auth::random_token();

        warn!("No AUTH_TOKEN was specified, using random token {}", token);

        token
    })
}

fn port() -> String {
    env::var("PORT").unwrap_or_else(|_| "8000".to_string())
}

fn database_url() -> String {
    env::var("DATABASE_URL").unwrap_or_else(|_| "pragma.sqlite".to_owned())
}
