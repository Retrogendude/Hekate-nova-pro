"""Local Chinese draft translation; preserve format fields and machine tokens."""
from pathlib import Path
import sys, json, re, time
W=Path(__file__).resolve().parent
sys.path.insert(0,str(W/'mt-libs'))
import ctranslate2, sentencepiece
P=W/'hekate-nova-pro-v33/nyx/nyx_gui'
catalog=json.loads((P/'localization_catalog.json').read_text(encoding='utf-8'))
cache_path=W/'nova33-zh-cache.json'
cache=json.loads(cache_path.read_text(encoding='utf-8')) if cache_path.exists() else {}
fmt=r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn%])'
# Keep every placeholder, path, LVGL recolor marker, register and acronym intact.
token=re.compile(fmt+r'|#[0-9A-Fa-f]{6}\b|#|\b\S*[/\\]\S*|\b0x[0-9A-Fa-f]+\b|\b[A-Z][A-Z0-9_]{1,}\b|\b\w*[0-9_]\w*\b|\b(?:hekate|Hekate|Nyx|Nova|atmosphere|Atmosphere|Nintendo|Switch|Joy-Con|Tegra|Mariko|Erista|Linux|Android|Minerva|Horizon|Bluetooth|FAT32|emuMMC|sysMMC|Payload|payload)\b|[\x00-\x1f]')
def parts(text):
 out=[];at=0
 for m in token.finditer(text):
  if m.start()>at:out.append((text[at:m.start()],False))
  out.append((m.group(),True));at=m.end()
 if at<len(text):out.append((text[at:],False))
 return out
def natural(s):return bool(re.search('[A-Za-z]{2}',s))
fragments=sorted({s.strip() for v in catalog.values() for s,protected in parts(v[0]) if not protected and natural(s)})
model=W/'nova33-language-resources/en_zh'
sp=sentencepiece.SentencePieceProcessor(model_file=str(next(model.rglob('sentencepiece.model'))))
engine=ctranslate2.Translator(str(next(model.rglob('model.bin')).parent),device='cpu',compute_type='int8',intra_threads=4,inter_threads=1)
pending=[s for s in fragments if s not in cache]
print('Chinese fragments',len(fragments),'pending',len(pending),flush=True)
start=time.time()
for i in range(0,len(pending),24):
 batch=pending[i:i+24]
 results=engine.translate_batch([sp.encode(x,out_type=str) for x in batch],beam_size=3,max_decoding_length=240,repetition_penalty=1.15)
 for src,result in zip(batch,results):
  target=sp.decode(result.hypotheses[0]).strip()
  if not target or any(ord(c)<32 for c in target):raise ValueError((src,target))
  cache[src]=target.replace('%','百分比').replace('▁',' ')
 cache_path.write_text(json.dumps(cache,ensure_ascii=False,indent=2),encoding='utf-8')
 print('translated',min(i+len(batch),len(pending)),'/',len(pending),'seconds',round(time.time()-start),flush=True)
result={}
for key,values in catalog.items():
 value=''
 for s,protected in parts(values[0]):
  core=s.strip()
  if not protected and natural(s):
   value+=s[:len(s)-len(s.lstrip())]+cache[core].replace('▁',' ')+s[len(s.rstrip()):]
  else:value+=s
 assert re.findall(fmt,value)==re.findall(fmt,values[0]),(key,value)
 result[key]=value
(P/'localization_zh_draft.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
print('Chinese draft ready:',len(result),'entries',flush=True)
