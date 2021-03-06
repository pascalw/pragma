const WebpackPwaManifest = require("webpack-pwa-manifest");
module.exports = new WebpackPwaManifest({
  ios: true,
  start_url: "/",
  name: "Pragma",
  short_name: "Pragma",
  description: "Open source personal note taking.",
  orientation: "any",
  display: "standalone",
  background_color: "#fbfbfb",
  theme_color: "#5b88ef",
  crossorigin: null,
  icons: [
    {
      src: "src/assets/favicons/favicon-32.png",
      sizes: "32x32",
      type: "image/png",
    },
    {
      src: "src/assets/favicons/favicon-72.png",
      sizes: "72x72",
      type: "image/png",
    },
    {
      src: "src/assets/favicons/favicon-72.png",
      sizes: "72x72",
      type: "image/png",
      ios: true
    },
    {
      src: "src/assets/favicons/favicon-114.png",
      sizes: "114x114",
      type: "image/png",
      ios: true
    },
    {
      src: "src/assets/favicons/favicon-144.png",
      sizes: "144x144",
      type: "image/png",
      ios: true
    },
    {
      src: "src/assets/favicons/favicon-152.png",
      sizes: "152x152",
      type: "image/png",
      ios: true
    },
    {
      src: "src/assets/favicons/favicon-180.png",
      sizes: "180x180",
      type: "image/png",
      ios: true
    },
    {
      src: "src/assets/favicons/favicon-192.png",
      sizes: "192x192",
      type: "image/png"
    },
    {
      src: "src/assets/favicons/favicon-512.png",
      sizes: "512x512",
      type: "image/png"
    },
  ]
});
