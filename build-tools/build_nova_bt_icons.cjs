/* Rasterize the same Lucide SVG paths used by the Nova reference. */
const fs = require('fs');
const path = require('path');
const root = 'C:/Users/Retrogen/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules';
const lucide = require(path.join(root, 'lucide/dist/cjs/lucide.js'));
const sharp = require(path.join(root, 'sharp'));
const target = path.join(__dirname, 'nova-bt-assets');
fs.mkdirSync(target, {recursive:true});
const names = ['download','bluetooth','square','trash-2'];
function pascal(name){return name.split('-').map(x=>x[0].toUpperCase()+x.slice(1)).join('');}
function nodeSvg(node){const [tag,attrs,children]=node;return `<${tag} ${Object.entries(attrs).map(([k,v])=>`${k}="${v}"`).join(' ')}>${(children||[]).map(nodeSvg).join('')}</${tag}>`;}
async function main(){
const entries=[];
for(const name of names){
 const nodes=name==='brand-mark'?[]:lucide.icons[pascal(name)];
 if(!nodes)throw Error('Missing Lucide icon: '+name);
 const svg=name==='brand-mark'?'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M16.5 3.8 12 1.7 2.8 6.8v10.4l9.2 5.1 9.2-5.1V9.3"/><path d="M7.3 16V8l9.4 8V8"/><path d="m20.2 1.4.8 2.4 2.4.8-2.4.8-.8 2.4-.8-2.4-2.4-.8 2.4-.8z" fill="white" stroke="none"/></svg>':`<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#ffffff" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round">${nodes.map(nodeSvg).join('')}</svg>`;
 fs.writeFileSync(path.join(target,name+'.svg'),svg);
 for(const size of [18,24,40]){
  const png=await sharp(Buffer.from(svg),{density:192}).resize(size,size).png().toBuffer();
  const {data,info}=await sharp(png).ensureAlpha().raw().toBuffer({resolveWithObject:true});
  const alpha=Buffer.alloc(size*size);for(let i=0;i<alpha.length;i++)alpha[i]=data[i*info.channels+3];
  const id=name.replaceAll('-','_')+'_'+size;
  entries.push(`static const uint8_t nova_${id}_pixels[] = {${[...alpha].join(',')}};\nstatic const lv_img_dsc_t nova_${id} = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=${size},.h=${size}},.data_size=${alpha.length},.data=nova_${id}_pixels};`);
 }
}
let output='/* Generated from Lucide 1.8.0 SVG paths, ISC license. Do not hand edit. */\n'+entries.join('\n')+'\n';
output+='static const struct {const char *name; const lv_img_dsc_t *small,*normal,*large;} nova_icon_assets[] = {\n';
for(const name of names){const id=name.replaceAll('-','_');output+=`{"${name}",&nova_${id}_18,&nova_${id}_24,&nova_${id}_40},\n`;}
output+='};\n';
fs.writeFileSync(path.join(target,'nova_icons.inc'),output);
fs.copyFileSync(path.join(root,'lucide/LICENSE'),path.join(target,'LUCIDE-LICENSE.txt'));
fs.writeFileSync(path.join(target,'manifest.json'),JSON.stringify({library:'lucide',version:'1.8.0',license:'ISC',strokeWidth:1.7,sizes:[18,24,40],icons:names,customIcons:['brand-mark']},null,2));
console.log(`Generated ${names.length} Lucide icons at three sizes, with antialiased alpha masks.`);
}
main().catch(error=>{console.error(error);process.exitCode=1;});
