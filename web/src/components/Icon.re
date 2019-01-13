type iconType =
  | Close
  | Add
  | FormatBold
  | FormatItalic
  | FormatUnderline
  | FormatStrikethrough
  | FormatQuote
  | BulletList
  | NumberedList
  | CheckList
  | Spellcheck
  | Sync
  | Recent
  | HeadingOne
  | HeadingTwo
  | HeadingThree;

let component = ReasonReact.statelessComponent("Icon");
let make = (~icon: iconType, _children) => {
  ...component,
  render: _self =>
    switch (icon) {
    | Close =>
      <svg
        xmlns="http://www.w3.org/2000/svg"
        width="24"
        height="24"
        viewBox="0 0 24 24"
        preserveAspectRatio="xMidYMid meet">
        <path
          d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"
        />
      </svg>
    | Add =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z" />
      </svg>
    | FormatBold =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path
          d="M15.6 10.79c.97-.67 1.65-1.77 1.65-2.79 0-2.26-1.75-4-4-4H7v14h7.04c2.09 0 3.71-1.7 3.71-3.79 0-1.52-.86-2.82-2.15-3.42zM10 6.5h3c.83 0 1.5.67 1.5 1.5s-.67 1.5-1.5 1.5h-3v-3zm3.5 9H10v-3h3.5c.83 0 1.5.67 1.5 1.5s-.67 1.5-1.5 1.5z"
        />
      </svg>
    | FormatItalic =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path d="M10 4v3h2.21l-3.42 8H6v3h8v-3h-2.21l3.42-8H18V4z" />
      </svg>
    | FormatUnderline =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path
          d="M12 17c3.31 0 6-2.69 6-6V3h-2.5v8c0 1.93-1.57 3.5-3.5 3.5S8.5 12.93 8.5 11V3H6v8c0 3.31 2.69 6 6 6zm-7 2v2h14v-2H5z"
        />
      </svg>
    | FormatStrikethrough =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path d="M10 19h4v-3h-4v3zM5 4v3h5v3h4V7h5V4H5zM3 14h18v-2H3v2z" />
      </svg>
    | FormatQuote =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path d="M6 17h3l2-4V7H5v6h3zm8 0h3l2-4V7h-6v6h3z" />
      </svg>
    | BulletList =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path
          d="M4 10.5c-.83 0-1.5.67-1.5 1.5s.67 1.5 1.5 1.5 1.5-.67 1.5-1.5-.67-1.5-1.5-1.5zm0-6c-.83 0-1.5.67-1.5 1.5S3.17 7.5 4 7.5 5.5 6.83 5.5 6 4.83 4.5 4 4.5zm0 12c-.83 0-1.5.68-1.5 1.5s.68 1.5 1.5 1.5 1.5-.68 1.5-1.5-.67-1.5-1.5-1.5zM7 19h14v-2H7v2zm0-6h14v-2H7v2zm0-8v2h14V5H7z"
        />
      </svg>
    | NumberedList =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path
          d="M2 17h2v.5H3v1h1v.5H2v1h3v-4H2v1zm1-9h1V4H2v1h1v3zm-1 3h1.8L2 13.1v.9h3v-1H3.2L5 10.9V10H2v1zm5-6v2h14V5H7zm0 14h14v-2H7v2zm0-6h14v-2H7v2z"
        />
      </svg>
    | CheckList =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24">
        <path
          d="M6.99 16.266L3.657 20 1.99 18.137l.743-.79.924 1.042 2.57-2.889.763.766zm0-6L3.657 14 1.99 12.137l.743-.79.924 1.042L6.227 9.5l.763.766zm0-6L3.657 8 1.99 6.137l.743-.79.924 1.042L6.227 3.5l.763.766zM8 19h13v-2H8v2zm0-6h13v-2H8v2zm0-8v2h13V5H8z"
        />
      </svg>
    | Spellcheck =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24">
        <path
          d="M12.45 16h2.09L9.43 3H7.57L2.46 16h2.09l1.12-3h5.64l1.14 3zm-6.02-5L8.5 5.48 10.57 11H6.43zm15.16.59l-8.09 8.09L9.83 16l-1.41 1.41 5.09 5.09L23 13l-1.41-1.41z"
        />
      </svg>
    | Sync =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24">
        <path
          d="M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 1.01-.25 1.97-.7 2.8l1.46 1.46A7.93 7.93 0 0 0 20 12c0-4.42-3.58-8-8-8zm0 14c-3.31 0-6-2.69-6-6 0-1.01.25-1.97.7-2.8L5.24 7.74A7.93 7.93 0 0 0 4 12c0 4.42 3.58 8 8 8v3l4-4-4-4v3z"
        />
      </svg>
    | Recent =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24">
        <path
          d="M11.99 2C6.47 2 2 6.48 2 12s4.47 10 9.99 10C17.52 22 22 17.52 22 12S17.52 2 11.99 2zM12 20c-4.42 0-8-3.58-8-8s3.58-8 8-8 8 3.58 8 8-3.58 8-8 8z"
        />
        <path d="M12.5 7H11v6l5.25 3.15.75-1.23-4.5-2.67z" />
      </svg>
    | HeadingOne =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24">
        <path
          d="M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm0 16H5V5h14v14zm-7-2h2V7h-4v2h2z"
        />
      </svg>
    | HeadingTwo =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24">
        <path
          d="M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm0 16H5V5h14v14zm-4-4h-4v-2h2a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2H9v2h4v2h-2a2 2 0 0 0-2 2v4h6v-2z"
        />
      </svg>
    | HeadingThree =>
      <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24">
        <path
          d="M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm0 16H5V5h14v14zm-4-4v-1.5c0-.83-.67-1.5-1.5-1.5.83 0 1.5-.67 1.5-1.5V9a2 2 0 0 0-2-2H9v2h4v2h-2v2h2v2H9v2h4a2 2 0 0 0 2-2z"
        />
      </svg>
    },
};
