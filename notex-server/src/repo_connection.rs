use diesel::prelude::*;
use diesel::r2d2::{ConnectionManager, Error};
use diesel::SqliteConnection;
use r2d2;

pub type Pool = r2d2::Pool<ConnectionManager<SqliteConnection>>;

#[derive(Debug)]
struct ConnectionCustomizer;

impl r2d2::CustomizeConnection<SqliteConnection, Error> for ConnectionCustomizer {
    fn on_acquire(&self, conn: &mut SqliteConnection) -> Result<(), Error> {
        conn.execute("PRAGMA foreign_keys = ON")
            .expect("Failed to enable forgein_keys.");

        Ok(())
    }

    fn on_release(&self, _: SqliteConnection) {}
}

pub fn create_pool(database_url: &str) -> Pool {
    let manager = ConnectionManager::<SqliteConnection>::new(database_url);

    r2d2::Pool::builder()
        .max_size(1)
        .connection_customizer(Box::new(ConnectionCustomizer))
        .build(manager)
        .expect("Failed to create database pool.")
}
