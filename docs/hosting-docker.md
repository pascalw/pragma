# Hosting Pragma on Docker

Pragma provides a Docker image on Docker Hub: `pascalw/pragma`.
Data will be written in `/data` so make sure you mount a volume at this location.

```sh
docker run -p 127.0.0.1:8000:8000 \
		   -v /srv/pragma/data:/data \
		   -e AUTH_TOKEN=changeme \
		   -it pascalw/pragma
```

This will expose Pragma on localhost port `8000` on your host.

### Enabling HTTPS

If your Pragma instance is internet facing you should enable HTTPS. Pragma supports SSL via the following environment variables:

- `SSL=true` to enable the SSL listener
- `SSL_KEY=path/to/ssl/key.pem`
- `SSL_CERT=path/to/ssl/cert.pem`

If you're using Letsencrypt for example that could look like this:

```sh
docker run -p 127.0.0.1:8000:8000 \
		   -v /srv/pragma/data:/data \
		   -v /etc/letsencrypt/:/etc/letsencrypt \
		   -e AUTH_TOKEN=changeme \
		   -e SSL=true \
		   -e SSL_KEY=/etc/letsencrypt/live/example.org/privkey.pem \
		   -e SSL_CERT=/etc/letsencrypt/live/example.org/fullchain.pem \
		   -it pascalw/pragma
```