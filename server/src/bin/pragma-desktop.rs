use directories::ProjectDirs;
use std::{fs, thread};

fn main() {
    configure_logger();

    /* We have to use a fixed port :(. If we'd use an ephemeral port the webview will lose
    the data stored in localStorage and IndexedDB because this is scoped by origin (which includes port). */
    let port = "60773";

    thread::spawn(move || {
        let dirs = ProjectDirs::from("me", "pascalw", "pragma")
            .expect("Could not determine data directory.");
        fs::create_dir_all(dirs.data_dir()).expect("Failed to create data directory.");

        let database_path = dirs
            .data_dir()
            .join("pragma.sqlite")
            .to_string_lossy()
            .into_owned();

        let config = pragma::Config {
            port: port.to_string(),
            auth_token: "desktop".to_string(),
            database_url: database_path,
        };

        let sys = pragma::build(config);
        let _ = sys.run();
    });

    tether::builder()
        .size(1200, 800)
        .html(& format!(r#"
        <script>
            const tryConnect = function() {{
                fetch('http://localhost:{0}/version').then(function() {{ window.location.href = 'http://localhost:{0}'; }}).catch(function(e) {{
                    setTimeout(tryConnect, 50);
                }});
            }};
            tryConnect();
        </script>
        "#, port))
        .start();
}

fn configure_logger() {
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .init();
}
