use lazy_static::lazy_static;

lazy_static! {
    static ref BUILD_VERSION: String =
        format!("{}-{}", env!("CARGO_PKG_VERSION"), env!("VERGEN_SHA_SHORT"));
}

pub fn build_version() -> String {
    BUILD_VERSION.to_string()
}
