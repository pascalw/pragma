# Pragma

### A self-hosted, personal note taking app.

Pragma runs in your browser and syncs all your notes back to the server transparently so you'll never lose your data.
All your notes are also available offline in your browser, so always available to you.

![](./screenshots/text-note.png)

## Features

* Browser-based note taking, offline.
* Self hosted; you own all your data.
* Lightweight. The sync server is written in Rust and takes only ~ 5-8 MB of memory.
* Super easy to host, it's just a single binary!
* WYSIWYG editor for text notes.
* Code editor with syntax highlighting for code snippets.

## Public demo

A demo instance is available at [pragma-demo.pascalw.me](https://pragma-demo.pascalw.me). Data in this instance is reset every 24 hours.

## Hosting

Pragma is designed to be hosted on a server and accessed via your webbrowser. Instructions are available for hosting on [Linux](./docs/hosting-linux.md) and [Docker](./docs/hosting-docker.md).

## Roadmap

- [ ] Keyboard shortcuts.
- [ ] Checkboxes in text editor.
- [ ] Easy access to recent notes.
- [ ] Quick open / search.
- [ ] Sync conflict resolution.
- [ ] Mobile friendly.
- [ ] Anonymous note sharing.

## Known issues

- Only tested on Chrome and Firefox.
- Currently uses `localstorage` in the browser.
  - Some browsers like Safari have very small limits on `localstorage` so you'll start hitting quota issues around 500 notes.
  - Chrome and Firefox have much bigger quota limits.
  - Will be fixed by switching to IndexedDB.
- No sync conflict resolution yet (see roadmap).
  - Currently, last write wins. You'll only run into this if you edit the same note from multiple devices at the same time.

![](./screenshots/login.png)

![](./screenshots/code-note.png)