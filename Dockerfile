FROM scratch

ADD ./server/target/x86_64-unknown-linux-musl/release/pragma.stripped /usr/bin/pragma
CMD ["/usr/bin/pragma"]

WORKDIR /data

ENV HOST=0.0.0.0
EXPOSE 8000
