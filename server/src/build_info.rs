#![allow(dead_code)]
include!(concat!(env!("OUT_DIR"), "/version.rs"));

use lazy_static::lazy_static;

lazy_static! {
    static ref BUILD_VERSION: String = format!("{}-{}", env!("CARGO_PKG_VERSION"), short_sha());
    static ref BUILD_VERSION_SHORT: String =
        format!("{}-{}", env!("CARGO_PKG_VERSION"), short_sha());
}

pub fn build_version() -> String {
    BUILD_VERSION.to_string()
}

pub fn build_version_short() -> String {
    BUILD_VERSION.to_string()
}
