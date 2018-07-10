use actix::prelude::*;
use repo_actor;

pub struct State {
    pub db: Addr<Syn, repo_actor::DbExecutor>,
}
