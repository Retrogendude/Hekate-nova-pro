from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import json, hashlib, re
w=Path(__file__).resolve().parent; o=w.parent/'outputs'; d=w/'nova-installer-1.2'
sha=lambda b:hashlib.sha256(b).hexdigest()
tested=json.loads((d/'build/test-result.json').read_text()); assert tested['passed']
package=json.loads((w/'nova34a5-package-check.json').read_text()); assert package['passed']
nyx=(w/'hekate-nova-pro-v34-alpha5/output/nyx.bin').read_bytes()
assert b'Hekate Nova Pro 34-alpha5 - automatic dock output' in nyx
assert b'video stream synchronization confirmed' in nyx
for suffix in ('','-8GB'):
    name='hekate-nova-pro-v34-alpha5'+suffix+'.zip'; p=o/name
    assert sha(p.read_bytes())==package['archives'][name]['sha256']
    with ZipFile(p) as z:
        manifest=json.loads(z.read('BUNDLE-MANIFEST.json'))
        assert len(z.namelist())==167
        for n,h in manifest.items(): assert sha(z.read(n))==h,n
        assert z.read('bootloader/sys/nyx.bin')==nyx
        payload=z.read('bootloader/update.bin')
        alias=next(n for n in z.namelist() if n.startswith('bootloader/payloads/hekate_nova_pro_6.5.5_v34_alpha5'))
        assert z.read(alias)==payload and bool(payload[0x122]&1)==bool(suffix)
exe=o/'Hekate-Nova-Pro-v34-alpha5-Installation-v1.2.exe'
report={'passed':True,'nova_version':'34-alpha5','installer_version':'1.2','files_per_variant':167,
        'installer_sha256':sha(exe.read_bytes()),'installer_bytes':exe.stat().st_size,
        'native_tests':tested,'physical_sd_written':False,'hdmi_hardware_validation_pending':True,
        'system_dll_imports':re.findall(r'DLL Name: (.+)',(d/'build/pe-imports.txt').read_text()),
        'source_sha256':{f.name:sha(f.read_bytes()) for f in d.iterdir() if f.is_file()}}
(o/'INSTALLATEUR-NOVA-v1.2-VERIFICATION.json').write_text(json.dumps(report,indent=2))
note='''HEKATE NOVA PRO v34 alpha 5 — INSTALLATEUR WINDOWS 1.2

1. Lancez Hekate-Nova-Pro-v34-alpha5-Installation-v1.2.exe sous Windows 64 bits.
2. Choisissez la lettre de la carte SD et vérifiez sa capacité.
3. Gardez Standard pour une console normale ; 8 Go expérimental est réservé à une console physiquement modifiée en RAM.
4. Lancez l’installation, attendez la confirmation puis éjectez la carte.

Le programme copie et vérifie les 167 fichiers du pack. Il ne partitionne pas la carte et n’injecte pas de payload. La carte doit être en FAT32/exFAT. Les configurations, appairages et personnalisations existants sont conservés ; les composants remplacés sont sauvegardés dans nova-backups. Le payload Nova est également présent dans bootloader/payloads. Les deux variantes sont intégrées, aucun téléchargement supplémentaire n’est nécessaire. Exécutable non signé.

Cette alpha maintient le service TV pendant les opérations longues et diffère ses rapports pour éviter des accès SD concurrents. Les échecs de connexion peuvent entraîner jusqu’à trois tentatives après nettoyage. Le flux vidéo doit désormais confirmer sa synchronisation : l’entraînement réussi des voies ne suffit plus à annoncer le succès. Le journal indique 34-alpha5 au lieu de 20-Dock.

Contrôles PC réussis : 54 scénarios du service TV, 9 de synchronisation, intégration réelle aux boucles longues, compilation ARM/en-têtes et installation native des deux vrais packs. Aucune écriture sur SD physique ni validation HDMI effectuée ici.

Essai conseillé : vérifier d’abord l’image au démarrage, puis après plusieurs retraits/remises de la console directement sur le dock officiel. Tester ensuite l’affichage pendant le partitionnement et la copie emuMMC sur une carte dont les données sont sauvegardées. En cas d’échec, transmettre bootloader/tv-test/last-run.txt ; les fichiers d’historique conservent les tentatives précédentes. VOL− arrête la sortie TV et annule les relances.
'''
(o/'INSTALLATEUR-NOVA-v1.2-LIRE-MOI.txt').write_text(note,encoding='utf-8')
zip_path=o/'Hekate-Nova-Pro-v34-alpha5-Installateur-v1.2.zip'
with ZipFile(zip_path,'w',ZIP_DEFLATED) as z:
    for f in (exe,o/'INSTALLATEUR-NOVA-v1.2-LIRE-MOI.txt',o/'INSTALLATEUR-NOVA-v1.2-VERIFICATION.json'):
        z.write(f,f.name)
source_path=o/'Hekate-Nova-Pro-Installateur-v1.2-sources.zip'
with ZipFile(source_path,'w',ZIP_DEFLATED) as z:
    for f in d.iterdir():
        if f.is_file(): z.write(f,'nova-installer-1.2/'+f.name)
    for f in (w/'package_nova34a5.py',w/'finish_nova34a5.py'):
        z.write(f,'build-tools/'+f.name)
(o/'INSTALLATEUR-NOVA-v1.2-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n' for f in (exe,zip_path,source_path)))
state=json.loads((w/'nova34a5-status.json').read_text()); state.update({'packages_verified':True,'installer_native_tests':tested['result'],'installer_version':'1.2'})
(w/'nova34a5-status.json').write_text(json.dumps(state,indent=2))
print(json.dumps(report,indent=2))
