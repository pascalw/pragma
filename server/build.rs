use vergen::*;

fn main() {
    let mut flags = OutputFns::all();
    flags.toggle(TARGET);
    flags.toggle(SEMVER);

    assert!(vergen(flags).is_ok());
}
