const WebpackPwaManifest = require("webpack-pwa-manifest");
module.exports = new WebpackPwaManifest({
  ios: true,
  start_url: "/",
  name: "Notex",
  short_name: "Notex",
  description: "Open source personal note taking.",
  orientation: "any",
  display: "standalone",
  background_color: "#fbfbfb",
  theme_color: "#3667E2",
  crossorigin: null,
  icons: [
    {
      src: "public/icons/favicon-32.png",
      sizes: "32x32",
      type: "image/png",
    },
    {
      src: "public/icons/favicon-72.png",
      sizes: "72x72",
      type: "image/png",
    },
    {
      src: "public/icons/favicon-72-precomposed.png",
      sizes: "72x72",
      type: "image/png",
      ios: true
    },
    {
      src: "public/icons/favicon-114-precomposed.png",
      sizes: "114x114",
      type: "image/png",
      ios: true
    },
    {
      src: "public/icons/favicon-144-precomposed.png",
      sizes: "144x144",
      type: "image/png",
      ios: true
    },
    {
      src: "public/icons/favicon-180-precomposed.png",
      sizes: "180x180",
      type: "image/png",
      ios: true
    },
    {
      src: "public/icons/favicon-192.png",
      sizes: "192x192",
      type: "image/png"
    },
    {
      src: "public/icons/favicon-512.png",
      sizes: "512x512",
      type: "image/png"
    },
  ]
});
