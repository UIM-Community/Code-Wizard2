const fs        = require('fs');
const path      = require('path');
const remote    = require('electron').remote;

const samplesDir = path.join( __dirname, '..' , 'samples' );

// When dom is ready!
document.addEventListener('DOMContentLoaded',function(e) {

    /*
     * Window events
     */
    const window    = remote.getCurrentWindow();

    document.getElementById('win_maximize').addEventListener('click', () => {
        if (window.isMaximized() === false) {
            window.maximize();          
        } 
        else {
            window.unmaximize();
        }
    });

    document.getElementById('win_close').addEventListener('click', () => {
        window.close();
    });

    /*
     * loadSample
     */
    function loadSample(ext) {
        if(ext === undefined) {
            throw new Error("Ext name is invalid!");
        }
        if(typeof ext !== "string") {
            throw new TypeError("Ext have to be string");
        }
        codeBlock.innerHTML = "";
        const codeElem = document.createElement('code');
        codeElem.className = ext; 

        const buf = fs.readFileSync( path.join( samplesDir, 'sample.'+ext ),'utf8');
        codeElem.innerText = buf.toString();
        codeBlock.appendChild(codeElem);
        console.log('start highlight!');
        hljs.highlightBlock(codeElem);
    }

    /*
     * Require generic block!
     */
    const codeBlock     = document.getElementById('code_block');
    const generateBTN   = document.getElementById('generate');

    /*
     * Add listener
     */
    generateBTN.addEventListener('click', e => {
        const ext = document.getElementById('language').value;
        loadSample(ext);
    });
});