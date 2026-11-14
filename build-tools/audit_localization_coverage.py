"""Compose authored fragments and report source coverage without word guessing.

The audit is an inventory of string literals, not a claim that every literal
is a visible UI element. Technical identifiers and filenames are left intact.
"""
from pathlib import Path
import argparse
import ast
import json
import re

WORK = Path(__file__).resolve().parent
ROOT = WORK / 'hekate-6.5.4'
TOKEN = re.compile(r'"(?:[^"\\]|\\.)*"|\'(?:[^\'\\]|\\.)*\'|/\*.*?\*/|//[^\n]*', re.S)
FORMATS = re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[diuoxXfFeEgGaAcspn]|%)')
COLOR = re.compile(r'#[0-9a-fA-F]{6} ')
IDENTIFIERS = set('eMMC emuMMC microSD SD NAND Hekate hekate Nyx Android Linux FAT32 exFAT EXT4 GPT MBR HOS OFW CFW RCM AutoRCM FSS0 Package1 Package2 pkg1 pkg2 KIP1 kip1 INI1 ini1 BOOT0 BOOT1 GPP CAL0 SYSTEM USER PRODINFO HDCP KFuses Ipatches I2C PCIe USB USB-PD UART_B SoC DRAM SDRAM GPIO OLED LCD PWM RTC PMIC DSI ADMAIF MAX77620 MAX77621 MAX77812 BQ24193 MAX17050 BM92T36 BH1730 TMP451 FTS4 FPC ITO T210 T210B01 Erista Mariko Hynix Samsung Micron Toshiba SanDisk SanDisk AUO JDI InnoLux Nintendo Switch Joy-Con POWER VOL VOL- VOL+ HOME L3 UMS LBA CID CSD EXT_CSD OEM RPMB EUDA MLC SLC KiB MiB GiB GB MB MHz kHz Hz mAh mA mV uV V C oC IOPS CPU GPU UART USB2 USB3 BootROM bootloader Lockpick Lockpick_RCM'.split())

def literals(path):
    source = path.read_text(encoding='utf-8', errors='replace')
    previous_end = None
    parts = []
    start = 0
    for token in TOKEN.finditer(source):
        raw = token.group()
        if not raw.startswith('"'): continue
        try: text = ast.literal_eval(raw)
        except (ValueError, SyntaxError): continue
        if previous_end is not None:
            between = source[previous_end:token.start()]
            between = re.sub(r'/\*.*?\*/|//[^\n]*', '', between, flags=re.S)
            if between.strip() and parts:
                yield ''.join(parts), source.count('\n', 0, start) + 1
                parts=[]
        if not parts: start=token.start()
        parts.append(text); previous_end=token.end()
    if parts: yield ''.join(parts), source.count('\n', 0, start) + 1

def technical(text):
    """Only machine-shaped tokens; never whitelist ordinary English prose."""
    text=FORMATS.sub('',text)
    text=COLOR.sub('',text).replace('#','').strip()
    if not text or not re.search(r'[A-Za-z\u00C0-\u024F]',text): return True
    if text in IDENTIFIERS: return True
    if re.fullmatch(r'[0-9 .:%+\-]*[A-Za-z]+[) ]*',text) and re.sub(r'[^A-Za-z]','',text) in IDENTIFIERS:return True
    if text.startswith('Copyright (c)') or re.match(r'^\(c\) \d{4}',text):return True
    # Source keys, filenames, paths and peripheral model/format identifiers.
    if re.fullmatch(r'[A-Za-z0-9_.:/\\*+\-\[\] ]+',text):
        if not ' ' in text and ('/' in text or '\\' in text): return True
        if re.search(r'\.(?:h|c|bin|ini|bso|elf|kip1|img|ext4|bmp|nyxf|ttf|otf)$',text): return True
        if text.isupper() and len(text.split()) <= 4: return True
        if re.search(r'\d',text) and not re.search(r'\b(?:read|write|error|size|clock|init|fail|found|level|total|open|boot|reset|power|volt|current)\b',text,re.I):
            if all(re.fullmatch(r'[A-Z][A-Z0-9_.:+\-]*|[0-9][0-9A-Za-z_.:+\-]*|[()|@]+',word) for word in text.split()): return True
    if re.fullmatch(r'[\w]+=(?:.*)',text): return True
    return False

def compose(text,catalog,depth=0):
    if text in catalog: return catalog[text]
    if depth>12: return None
    if not text or technical(text): return [text]*7
    if text.endswith('#') and text[:-1] in catalog:
        return [part+'#' for part in catalog[text[:-1]]]
    # Translate complete authored sentences, preserving exact highlighted
    # translated fragments or invariant numbers/identifiers where possible.
    if COLOR.search(text):
        plain=COLOR.sub('',text).replace('#','')
        translated=catalog.get(plain)
        if translated:
            spans=re.findall(r'(#[0-9a-fA-F]{6} )([^#]*)#',text)
            result=list(translated)
            for command,fragment in spans:
                translated_fragment=catalog.get(fragment,[fragment]*7)
                for i,value in enumerate(translated_fragment):
                    if value and value in result[i]:
                        result[i]=result[i].replace(value,command+value+'#',1)
            return result
    lead=len(text)-len(text.lstrip(' \t\r'))
    tail=len(text)-len(text.rstrip(' \t\r'))
    if lead or tail:
        core=text[lead:len(text)-tail if tail else len(text)]
        result=compose(core,catalog,depth+1)
        if result is not None:return [text[:lead]+part+(text[len(text)-tail:] if tail else '') for part in result]
    if '\n' in text:
        result=[compose(part,catalog,depth+1) for part in text.split('\n')]
        if all(part is not None for part in result): return ['\n'.join(part[language] for part in result) for language in range(7)]
        return None
    # Hekate color commands wrap already authored visible phrases.
    match=COLOR.search(text)
    if match:
        end=text.find('#',match.end())
        if end>=0:
            parts=[compose(text[:match.start()],catalog,depth+1),
                   compose(text[match.end():end],catalog,depth+1),
                   compose(text[end+1:],catalog,depth+1)]
            if all(part is not None for part in parts):
                return [parts[0][i]+match.group()+parts[1][i]+'#'+parts[2][i] for i in range(7)]
    return None

def source_sets(root=ROOT):
    return {
        'frontend': sorted((root/'nyx/nyx_gui/frontend').glob('*.c')),
        'diagnostics': sorted((root/'nyx/nyx_gui/diagnostics/vendor').glob('*.c')),
        'ram_service': sorted((WORK/'HWTest-RCM-0.15/ramtest/source').glob('*.c')),
        'bootloader': sorted((root/'bootloader').glob('*.c')),
    }

def expand_catalog(catalog,root=ROOT):
    """Add only fully composed translations, never guessed prose or words."""
    additions={}
    for files in source_sets(root).values():
        for path in files:
            for text,_ in literals(path):
                if text in catalog or not text or any(ord(c)<32 and c not in '\n\r\t' for c in text):continue
                if any(0x80<=ord(c)<0xC0 for c in text):continue # raw LVGL control bytes
                result=compose(text,catalog)
                if result and any(value != text for value in result):additions[text]=result
    catalog.update(additions)
    return len(additions)

def audit(catalog,root=ROOT):
    report={'note':'Inventory of source literals, not all are visible UI. Catalog includes authored UI text and offline translated report prose with reviewed overrides; coverage does not establish linguistic quality.', 'scopes':{}}
    for name,files in source_sets(root).items():
        texts={}
        for path in files:
            for text,line in literals(path):
                if not re.search(r'[A-Za-z]{3}',text): continue
                texts.setdefault(text,[]).append({'path':str(path.relative_to(WORK)),'line':line})
        counts={'catalog_or_composed':0,'technical_or_path':0,'untranslated_candidates':0}
        missing=[]
        for text,locations in texts.items():
            if text in catalog:counts['catalog_or_composed']+=1
            elif technical(text):counts['technical_or_path']+=1
            else:
                counts['untranslated_candidates']+=1
                missing.append({'text':text,'locations':locations})
        report['scopes'][name]={'counts':counts,'missing_candidates':missing}
    return report

def main():
    catalog=json.loads((ROOT/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
    report=audit(catalog)
    target=WORK/'localization-coverage.json'
    target.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    for scope,data in report['scopes'].items(): print(scope,data['counts'])
    print('Audit:',target)

if __name__=='__main__': main()
