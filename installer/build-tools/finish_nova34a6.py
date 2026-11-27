from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import json, hashlib, re
w=Path(__file__).resolve().parent; o=w.parent/'outputs'; d=w/'nova-installer-1.3'
sha=lambda b:hashlib.sha256(b).hexdigest()
tested=json.loads((d/'build/test-result.json').read_text()); assert tested['passed']
package=json.loads((w/'nova34a6-package-check.json').read_text()); assert package['passed']
nyx=(w/'hekate-nova-pro-v34-alpha6/output/nyx.bin').read_bytes()
assert b'Hekate Nova Pro 34-alpha6 - automatic dock output' in nyx
assert b'video stream synchronization confirmed' in nyx
for suffix in ('','-8GB'):
    name='hekate-nova-pro-v34-alpha6'+suffix+'.zip'; p=o/name
    assert sha(p.read_bytes())==package['archives'][name]['sha256']
    with ZipFile(p) as z:
        manifest=json.loads(z.read('BUNDLE-MANIFEST.json'))
        assert len(z.namelist())==167
        for n,h in manifest.items(): assert sha(z.read(n))==h,n
        assert z.read('bootloader/sys/nyx.bin')==nyx
        payload=z.read('bootloader/update.bin')
        alias=next(n for n in z.namelist() if n.startswith('bootloader/payloads/hekate_nova_pro_6.5.5_v34_alpha6'))
        assert z.read(alias)==payload and bool(payload[0x122]&1)==bool(suffix)
exe=o/'Hekate-Nova-Pro-v34-alpha6-Installation-v1.3.exe'
report={'passed':True,'nova_version':'34-alpha6','installer_version':'1.3','files_per_variant':167,
        'installer_sha256':sha(exe.read_bytes()),'installer_bytes':exe.stat().st_size,
        'native_tests':tested,'physical_sd_written':False,'hdmi_hardware_validation_pending':True,
        'system_dll_imports':re.findall(r'DLL Name: (.+)',(d/'build/pe-imports.txt').read_text()),
        'source_sha256':{f.name:sha(f.read_bytes()) for f in d.iterdir() if f.is_file()}}
(o/'INSTALLATEUR-NOVA-v1.3-VERIFICATION.json').write_text(json.dumps(report,indent=2))
note="HEKATE NOVA PRO v34 alpha 6 — INSTALLATEUR WINDOWS 1.3\n\n1. Lancez Hekate-Nova-Pro-v34-alpha6-Installation-v1.3.exe sous Windows 64 bits.\n2. Choisissez la lettre SD et vérifiez le nom et la capacité du lecteur.\n3. Gardez Standard pour une console normale. 8 Go expérimental est réservé à une console physiquement modifiée en RAM.\n4. Installez, attendez la confirmation puis éjectez la carte. Utilisez le payload alpha6 de ce pack.\n\nLes 167 fichiers sont intégrés et vérifiés. Aucun téléchargement nécessaire. L'installateur copie le pack ; il ne formate pas et n'injecte pas le payload. Configurations, appairages et personnalisations sont conservés. Les composants remplacés sont sauvegardés dans nova-backups, avec rapport installation.txt. Le lecteur doit être en FAT32 ou exFAT. Exécutable non signé.\n\nCette alpha ajoute une sortie contrôlée d'un ancien mode DisplayPort avant identification du dock. Si le premier lien perd la synchronisation, le pilote reprend seulement le lien vidéo à 2,7 puis 1,62 Gbit/s, sans couper toute la session PD entre ces replis. La confirmation fraîche porte sur l'ensemble des voies et le flux. Le signal TV-branchée seul ne bloque pas la reprise après une sortie DP confirmée.\n\nLe service TV reste actif pendant les opérations longues ; ses journaux sont différés pendant l'accès au stockage. Les limites de tentatives et les arrêts matériels sont conservés. Aucun reset PD forcé ni modification des tensions.\n\nContrôles PC : compilation ARM, en-têtes, 54 scénarios du service, 429 assertions du pilote, intégration aux opérations longues, copie/partitionnement/FatFs et installation native des deux packs. Aucun test HDMI ni écriture sur une carte physique ici.\n\nTester d'abord le démarrage directement dans le dock officiel, puis retirer/remettre la console. La LED verte seule ne confirme pas l'image. En cas d'échec, transmettre bootloader/tv-test/last-run.txt et les derniers run-*.txt. Le journal doit indiquer 34-alpha6. Attendre la validation de l'image avant de tester les opérations longues ; sauvegarder la SD avant partitionnement.\n"
(o/'INSTALLATEUR-NOVA-v1.3-LIRE-MOI.txt').write_text(note,encoding='utf-8')
zip_path=o/'Hekate-Nova-Pro-v34-alpha6-Installateur-v1.3.zip'
with ZipFile(zip_path,'w',ZIP_DEFLATED) as z:
    for f in (exe,o/'INSTALLATEUR-NOVA-v1.3-LIRE-MOI.txt',o/'INSTALLATEUR-NOVA-v1.3-VERIFICATION.json'):
        z.write(f,f.name)
source_path=o/'Hekate-Nova-Pro-Installateur-v1.3-sources.zip'
with ZipFile(source_path,'w',ZIP_DEFLATED) as z:
    for f in d.iterdir():
        if f.is_file(): z.write(f,'nova-installer-1.3/'+f.name)
    for f in (w/'package_nova34a6.py',w/'finish_nova34a6.py'):
        z.write(f,'build-tools/'+f.name)
(o/'INSTALLATEUR-NOVA-v1.3-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n' for f in (exe,zip_path,source_path)))
state=json.loads((w/'nova34a6-status.json').read_text()); state.update({'packages_verified':True,'installer_native_tests':tested['result'],'installer_version':'1.3'})
(w/'nova34a6-status.json').write_text(json.dumps(state,indent=2))
print(json.dumps(report,indent=2))
