/* Original vector brand, also used by the native boot screen. */
const fs=require('fs'),path=require('path');
const sharp=require('C:/Users/Retrogen/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/sharp');
const svg=fs.readFileSync(path.join(__dirname,'hekate-6.5.4/nyx/nyx_gui/frontend/nova_assets/brand-mark.svg'));
sharp(svg,{density:384}).resize(256,256).png().toFile(path.join(__dirname,'nova-brand.png')).then(()=>console.log('Exported original Nova Pro monogram.')).catch(e=>{console.error(e);process.exitCode=1;});
