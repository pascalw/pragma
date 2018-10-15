use actix::prelude::*;
use repo_actor;

pub struct State {
    pub db: Addr<repo_actor::DbExecutor>,
}
