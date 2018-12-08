# Hosting Pragma on Linux

Pragma provides pre-compiled binaries for Linux. These binaries are fully statically compiled, so they should run on any `x86_64` Linux machine.

Below you'll find instructions to setup Pragma on a Linux machine with `systemd`. The instructions assume you're running as `root`.

1. Download the latest pre-compiled binary from the [GitHub releases page](https://github.com/pascalw/pragma/releases/latest).

2. Place it somewhere on your file system and make the binary executable:

   ```sh
   mkdir -p /srv/pragma/data
   mv pragma-x86_64-unknown-linux-musl /srv/pragma/pragma
   chmod +x /srv/pragma/pragma
   chown nobody:nogroup /srv/pragma/data
   ```

3. Now let's setup a `systemd` service:

   ```sh
   cat << EOF > /etc/systemd/system/pragma.service
   [Unit]
   Description=pragma
   After=syslog.target network.target
   
   [Service]
   User=nobody
   Group=nogroup
   Type=simple
   Environment="AUTH_TOKEN=changeme"
   Environment="PORT=8000"
   WorkingDirectory=/srv/pragma/data
   ExecStart=/srv/pragma/pragma
   
   # if we crash, restart
   RestartSec=1
   Restart=on-failure
   
   # use syslog for logging
   StandardOutput=syslog
   StandardError=syslog
   SyslogIdentifier=pragma
   
   [Install]
   WantedBy=multi-user.target
   EOF
   ```

4. Activate the service:

   ```sh
   systemctl daemon-reload
   systemctl start pragma.service
   systemctl enable pragma.service # start at boot
   ```

That's it! At this point you have a Pragma instance running on your machine, listening on port `8000` with data stored in `/srv/pragma/data`.

## Enabling HTTPS

If your Pragma instance is internet facing you should enable HTTPS. Pragma supports SSL via the following environment variables:

* `SSL=true` to enable the SSL listener
* `SSL_KEY=path/to/ssl/key.pem`
* `SSL_CERT=path/to/ssl/cert.pem`

If you're using Letsencrypt this would be `SSL=true SSL_KEY=/etc/letsencrypt/live/example.org/privkey.pem SSL_CERT=/etc/letsencrypt/live/example.org/fullchain.pem`. Note that in this case you should run Pragma as `root` so it can access the certificates, or copy the certificates somewhere else so the `nobody` user can read them.

Alternatively you could have a proxy handle SSL. Pragma automatically handles `gzip` compression and caching headers, so you don't have to configure this in your proxy.