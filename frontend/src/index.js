import './styles/main.scss';
import { Main } from './Main.elm';
import pell from 'pell';
import registerServiceWorker from './registerServiceWorker';

const buildCustomEvent = (eventName, detail) => {
    if (typeof(CustomEvent) === 'function') {
        return new CustomEvent(eventName, {
            detail: detail,
            bubbles: true
        });
    } else {
        var event = document.createEvent('CustomEvent');
        event.initCustomEvent(eventName, true, false, detail);
        return event;
    }
}

window.initPell = () => {
    const target = document.currentScript.previousSibling;

    const contentTarget = target.previousSibling;
    const content = contentTarget.innerHTML;

    const observer = new MutationObserver(() => {
        editor.content.innerHTML = contentTarget.innerHTML;
    });
    observer.observe(contentTarget, { characterData: true, childList: true, subtree: true }); 

    const editor = pell.init({
        element: target,
        onChange: (html) => {
            const event  = buildCustomEvent("change", { value: html });
            target.dispatchEvent(event);
        }
    });

    editor.content.innerHTML = content;
};

document.addEventListener("keydown", (e) => {
    if(e.target.classList.contains("pell-content")) {
        if(e.key == "b" && e.metaKey) {
            pell.exec("bold");
            e.preventDefault();
            return;
        }

        if(e.key == "u" && e.metaKey) {
            pell.exec("underline");
            e.preventDefault();
            return;
        }

        if(e.key == "i" && e.metaKey) {
            pell.exec("italic");
            
            e.preventDefault();
            return;
        }
    }

}, false);

Main.embed(document.getElementById('root'));

registerServiceWorker();