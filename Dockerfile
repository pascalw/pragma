FROM scratch

ADD ./server/target/x86_64-unknown-linux-musl/release/pragma-x86_64-unknown-linux-musl /usr/bin/pragma
CMD ["/usr/bin/pragma"]

WORKDIR /data

ENV LISTEN_HOST=0.0.0.0
EXPOSE 8000
