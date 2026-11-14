const fs=require('fs'),path=require('path'),vm=require('vm');
const source=process.argv[2]||'C:/Users/Retrogen/.codex/visualizations/2026/11/04/01a1a7ec-1604-7c61-bf1b-3c2207a02f83/hekate-directions-pro.html';
const html=fs.readFileSync(source,'utf8');
function literal(pattern){const found=html.match(pattern);if(!found)throw Error('Missing reference data');return vm.runInNewContext('('+found[1]+')',Object.create(null),{timeout:1000,codeGeneration:{strings:false,wasm:false}});}
const items=literal(/const items=(\{[\s\S]*?\n \});/);
const titles=literal(/const titles=(\{[^\n]+\});/);
const navigation=literal(/const navItems=(\[[^\n]+\]);/);
const reference={source,canvas:{width:1280,height:720,header:76,bottom:656,padding:34,contentTop:140},
 colors:{background:'#10151d',panel:'#19212d',raised:'#202c3b',line:'#334050',text:'#f2f5fa',dim:'#9baabd',accent:'#55c9f1',ink:'#0c1721'},radius:8,halo:18,items,titles,navigation};
fs.writeFileSync(path.join(__dirname,'nova-reference.json'),JSON.stringify(reference,null,2));
let c='/* Text and icons extracted verbatim from the selected Nova mockup. */\n';
for(const [key,values] of Object.entries(items))c+=`static const nova_item nova_reference_${key}[] = {\n`+values.map(([id,title,description,icon])=>`{${[id,title,description,icon].map(JSON.stringify).join(',')}},`).join('\n')+'\n};\n';
fs.writeFileSync(path.join(__dirname,'hekate-6.5.4/nyx/nyx_gui/frontend/nova_assets/nova_reference.inc'),c);
console.log('Extracted Nova navigation, headings and 32 action cards from the reference.');
