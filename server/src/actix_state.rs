use crate::repo_actor;
use ::actix::prelude::*;

pub struct State {
    pub db: Addr<repo_actor::DbExecutor>,
}
