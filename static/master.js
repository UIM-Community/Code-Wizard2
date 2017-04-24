const fs    = require('fs');
const path  = require('path');

const samplesDir = path.join( __dirname, '..' , 'samples' );

document.addEventListener('DOMContentLoaded',function(e) {
    console.log('DOM Loaded');
    console.log(samplesDir);

    let codeBlock = document.getElementById('code_block');

    let codeElem = document.createElement('code');
    codeElem.className = 'java'; 

    let buf = fs.readFileSync( path.join( samplesDir, 'sample.java' ),'utf8');
    codeElem.innerText = buf.toString();
    codeBlock.appendChild(codeElem);
    hljs.initHighlightingOnLoad();
});