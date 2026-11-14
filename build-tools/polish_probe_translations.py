"""Prefer full-sentence offline translations when all machine tokens survive."""
from pathlib import Path
import collections,gc,json,re,sys,unicodedata
WORK=Path(__file__).resolve().parent
sys.path.insert(0,str(WORK/'mt-libs'))
import ctranslate2,sentencepiece
from audit_localization_coverage import FORMATS
ROOT=WORK/'hekate-6.5.4/nyx/nyx_gui'
source_file=ROOT/'localization_probe_extra.json'
fallback=json.loads(source_file.read_text(encoding='utf-8'))
backup=WORK/'mt-cache/probe-fragment-fallback.json'
if backup.exists(): fallback=json.loads(backup.read_text(encoding='utf-8'))
else: backup.write_text(json.dumps(fallback,ensure_ascii=False,indent=2),encoding='utf-8')
codes=['en','fr','es','it','ru','de','pt']
marker=re.compile(r'\[\s*\[\s*(\d+)\s*\]\s*\]')
machine=re.compile(r'0x[0-9A-Fa-f]+|\b[A-Za-z0-9]*[0-9_][A-Za-z0-9_.]*\b|\b[A-Z][A-Z0-9_]{1,}\b|\b(?:MHz|kHz|Hz|mV|mA|mAh|MiB|GiB|KiB|ms|ns|us|MB|GB|dB|Realtek|Broadcom|Cypress|Mariko|Erista|Nintendo|Lockpick|Tegra|JEDEC)\b|\b[0-9]+(?:\.[0-9]+)?\b')
def mask(line):
    fields=[]
    def replace(match): fields.append(match.group());return '[[%d]]'%(len(fields)-1)
    return FORMATS.sub(replace,line),fields
def identifiers(line): return collections.Counter(machine.findall(FORMATS.sub('',line)))
lines=sorted({line.strip() for source,values in fallback.items() if any(value!=source for value in values[1:]) for line in source.splitlines() if re.search(r'[a-z]{2}',line)})
result={key:list(values) for key,values in fallback.items()}
stats={}
for language,code in enumerate(codes[1:],1):
    path=WORK/'mt-cache'/('sentences-'+code+'.json')
    cache=json.loads(path.read_text(encoding='utf-8')) if path.exists() else {}
    pending=[line for line in lines if line not in cache]
    root=WORK/'mt-models'/code
    tokenizer=sentencepiece.SentencePieceProcessor(model_file=str(next(root.rglob('sentencepiece.model'))))
    engine=ctranslate2.Translator(str(next(root.rglob('model.bin')).parent),device='cpu',compute_type='int8',intra_threads=2,inter_threads=1)
    print(code,'full sentences',len(pending),flush=True)
    for at in range(0,len(pending),48):
        batch=pending[at:at+48];masked=[mask(line) for line in batch]
        outputs=engine.translate_batch([tokenizer.encode(line,out_type=str) for line,fields in masked],beam_size=3,max_decoding_length=200,repetition_penalty=1.1)
        for source,(encoded,fields),output in zip(batch,masked,outputs):
            value=''.join(output.hypotheses[0]).replace('▁',' ').strip()
            value=unicodedata.normalize('NFKC',value).replace('μ','µ').replace('→','->').replace('←','<-')
            found=[int(x) for x in marker.findall(value)]
            valid=found==list(range(len(fields)))
            if valid:
                value=marker.sub(lambda m:fields[int(m.group(1))],value)
                valid=FORMATS.findall(value)==FORMATS.findall(source) and identifiers(value)==identifiers(source)
            cache[source]=value if valid and value else None
        path.write_text(json.dumps(cache,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
        print(code,min(at+len(batch),len(pending)),'/',len(pending),flush=True)
    accepted=0
    for source,values in fallback.items():
        if all(value==source for value in values[1:]):continue
        original=source.splitlines(keepends=True);target=values[language].splitlines(keepends=True)
        if len(original)!=len(target):continue
        revised=[]
        for english,old in zip(original,target):
            core=english.strip();translated=cache.get(core)
            if translated:
                revised.append(english[:len(english)-len(english.lstrip())]+translated+english[len(english.rstrip()):]);accepted+=1
            else: revised.append(old)
        value=''.join(revised)
        assert FORMATS.findall(source)==FORMATS.findall(value)
        result[source][language]=value
    stats[code]={'accepted_full_lines':accepted,'candidates':len(lines)}
    del engine,tokenizer;gc.collect()
source_file.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(WORK/'mt-cache/sentence-validation.json').write_text(json.dumps(stats,indent=2),encoding='utf-8')
print('COMPLETE full-sentence validation',stats,flush=True)
