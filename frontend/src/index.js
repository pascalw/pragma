import './styles/main.scss';
import { Main } from './Main.elm';

import 'trix';
import 'trix/dist/trix.css';

import registerServiceWorker from './registerServiceWorker';

window.initTrix = () => {
    const editor = document.currentScript.previousSibling;
    const input = document.getElementById(editor.getAttribute("input"));

    const observer = new MutationObserver((e) => {
        editor.value = input.value;
        editor.blur();
    });
    observer.observe(input, { attributeFilter: ["id"] });
};

Main.embed(document.getElementById('root'));

registerServiceWorker();