"""Offline completion of remaining probe prose, preserving machine tokens.

Uses locally downloaded Argos models with CTranslate2/SentencePiece. The
firmware consumes only the resulting UTF-8 JSON; no model is shipped on SD.
Curated translations take precedence over these additional report strings.
"""
from pathlib import Path
import gc,hashlib,json,re,sys,unicodedata
WORK=Path(__file__).resolve().parent
sys.path.insert(0,str(WORK/'mt-libs'))
import ctranslate2
import sentencepiece
from audit_localization_coverage import literals,technical,FORMATS

ROOT=WORK/'hekate-6.5.4/nyx/nyx_gui'
catalog=json.loads((ROOT/'localization_catalog.json').read_text(encoding='utf-8'))
previous_path=ROOT/'localization_probe_extra.json'
previous=json.loads(previous_path.read_text(encoding='utf-8')) if previous_path.exists() else {}
texts=sorted(set(previous) | {text for p in (ROOT/'diagnostics/vendor').glob('*.c') for text,line in literals(p) if text and text not in catalog})
codes=['en','fr','es','it','ru','de','pt']
# Protect printf fields, hexadecimal values, register names, bit masks,
# peripheral models, units and paths. Only natural-language gaps are sent to
# the local model; reconstruction cannot reorder or modify these fields.
token=re.compile(FORMATS.pattern+r'|0x[0-9A-Fa-f]+|\b[A-Za-z0-9]*[0-9_][A-Za-z0-9_.]*\b|\b[A-Z][A-Z0-9_]{1,}\b|\b(?:MHz|kHz|Hz|mV|mA|mAh|MiB|GiB|KiB|ms|ns|us|MB|GB|dB|fs|pinmux|padctl|pull-up|pull-down|tristate|POR|Realtek|Broadcom|Cypress|Mariko|Erista|Switch|Lite|Nintendo|Lockpick|PRODINFO|HOS|UART|I2C|PCIe|Tegra|JEDEC)\b|\b[0-9]+(?:\.[0-9]+)?\b')
def exempt(text):
    core=text.strip()
    return technical(text) or bool(re.fullmatch(r'[a-z][a-z0-9]*_[a-z0-9_]+',core)) or '.h must match ' in text or core.startswith(('N_PAGES in ','g_page_groups must '))
def pieces(text):
    result=[];start=0
    for found in token.finditer(text):
        if found.start()>start: result.append((False,text[start:found.start()]))
        result.append((True,found.group()));start=found.end()
    if start<len(text):result.append((False,text[start:]))
    return result
parts={text:pieces(text) for text in texts if not exempt(text)}
gaps=sorted({value.strip() for row in parts.values() for immutable,value in row if not immutable and re.search(r'[A-Za-z]{2}',value)})
cache_dir=WORK/'mt-cache';cache_dir.mkdir(exist_ok=True)
translations={}
for code in codes[1:]:
    cache_file=cache_dir/(code+'.json')
    cache=json.loads(cache_file.read_text(encoding='utf-8')) if cache_file.exists() else {}
    pending=[gap for gap in gaps if gap not in cache and gap not in catalog]
    print(code,'pending',len(pending),'phrases',flush=True)
    model_root=WORK/'mt-models'/code
    tokenizer=sentencepiece.SentencePieceProcessor(model_file=str(next(model_root.rglob('sentencepiece.model'))))
    engine=ctranslate2.Translator(str(next(model_root.rglob('model.bin')).parent),device='cpu',compute_type='int8',inter_threads=1,intra_threads=2)
    for at in range(0,len(pending),48):
        batch=pending[at:at+48]
        results=engine.translate_batch([tokenizer.encode(x,out_type=str) for x in batch],beam_size=3,max_decoding_length=180,repetition_penalty=1.1)
        for phrase,result in zip(batch,results):
            translated=''.join(result.hypotheses[0]).replace('▁',' ').strip()
            translated=unicodedata.normalize('NFC',translated).replace('\u202f',' ').replace('\u03bc','µ')
            assert translated and not any(ord(c)<32 for c in translated),(code,phrase,translated)
            # Literal percent symbols belong exclusively to protected fields.
            translated=translated.replace('%','pour cent' if code=='fr' else 'percent')
            cache[phrase]=translated
        cache_file.write_text(json.dumps(cache,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
        print(code,min(at+len(batch),len(pending)),'/',len(pending),flush=True)
    translations[code]=cache
    del engine,tokenizer;gc.collect()

extra={}
for source in texts:
    values=[source]
    for language,code in enumerate(codes[1:],1):
        if exempt(source): values.append(source);continue
        output=[]
        for immutable,value in parts[source]:
            gap=value.strip()
            if immutable or not re.search(r'[A-Za-z]{2}',value): output.append(value);continue
            tr=catalog[gap][language] if gap in catalog else translations[code][gap]
            # Preserve punctuation-only wrappers and indentation/clearing spaces.
            before=value[:len(value)-len(value.lstrip())];after=value[len(value.rstrip()):]
            output.append(before+tr+after)
        values.append(''.join(output))
    assert all(FORMATS.findall(source)==FORMATS.findall(value) for value in values),source
    extra[source]=values
(ROOT/'localization_probe_extra.json').write_text(json.dumps(extra,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
metadata={'method':'Offline Argos/CTranslate2; protected printf/machine tokens; curated entries take precedence',
          'source':'https://github.com/argosopentech/argos-translate',
          'model_source':'https://data.argosopentech.com/argospm/v1/',
          'keys':len(extra),'natural_language_keys':len(parts),'unique_prose_fragments':len(gaps),
          'model_sha256':{code:hashlib.sha256((WORK/'mt-models'/(code+'.argosmodel')).read_bytes()).hexdigest() for code in codes[1:]}}
(ROOT/'localization_probe_provenance.json').write_text(json.dumps(metadata,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('COMPLETE',len(extra),'keys',len(parts),'prose keys',len(gaps),'unique fragments',flush=True)
