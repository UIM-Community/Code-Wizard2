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
        console.log('min clicked');
        if (window.isMaximized() === false) {
            window.maximize();          
        } 
        else {
            window.unmaximize();
        }
    });

    document.getElementById('win_close').addEventListener('click', () => {
        console.log('closed clicked!');
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
    const codeBlock         = document.getElementById('code_block');
    const generateBTN       = document.getElementById('generate');
    const copyToClipboard   = document.getElementById('copy');

    function selectText(containerid) {
        if (document.selection) {
            var range = document.body.createTextRange();
            range.moveToElementText(codeBlock);
            range.select();
        } 
        else if (window.getSelection) {
            var range = document.createRange();
            range.selectNode(codeBlock);
            window.getSelection().addRange(range);
        }
    }

    if(document.queryCommandSupported('copy') === false) {
        copyToClipboard.style.display = 'none';
    }
    else {
        copyToClipboard.addEventListener('click',() => {
            selectText();

            try {
                var successful = document.execCommand('copy');
                var msg = successful ? 'successful' : 'unsuccessful';
                console.log('Copying text command was ' + msg);
            } catch (err) {
                console.log('Oops, unable to copy');
            }
        });
    }

    /*
     * Add listener
     */
    generateBTN.addEventListener('click', e => {
        const ext = document.getElementById('language').value;

        loadSample(ext);
    });
});