from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import json, hashlib, re
w=Path(__file__).resolve().parent; o=w.parent/'outputs'; d=w/'nova-installer-1.4'
sha=lambda b:hashlib.sha256(b).hexdigest()
tested=json.loads((d/'build/test-result.json').read_text()); assert tested['passed']
package=json.loads((w/'nova34a7-package-check.json').read_text()); assert package['passed']
nyx=(w/'hekate-nova-pro-v34-alpha7/output/nyx.bin').read_bytes()
assert b'Hekate Nova Pro 34-alpha7 - automatic dock output' in nyx
assert b'video stream synchronization confirmed' in nyx
assert b'video stream confirmed by fresh sink CRC frames and stable lanes' in nyx
for suffix in ('','-8GB'):
    name='hekate-nova-pro-v34-alpha7'+suffix+'.zip'; p=o/name
    assert sha(p.read_bytes())==package['archives'][name]['sha256']
    with ZipFile(p) as z:
        manifest=json.loads(z.read('BUNDLE-MANIFEST.json'))
        assert len(z.namelist())==167
        for n,h in manifest.items(): assert sha(z.read(n))==h,n
        assert z.read('bootloader/sys/nyx.bin')==nyx
        payload=z.read('bootloader/update.bin')
        alias=next(n for n in z.namelist() if n.startswith('bootloader/payloads/hekate_nova_pro_6.5.5_v34_alpha7'))
        assert z.read(alias)==payload and bool(payload[0x122]&1)==bool(suffix)
exe=o/'Hekate-Nova-Pro-v34-alpha7-Installation-v1.4.exe'
report={'passed':True,'nova_version':'34-alpha7','installer_version':'1.4','files_per_variant':167,
        'installer_sha256':sha(exe.read_bytes()),'installer_bytes':exe.stat().st_size,
        'native_tests':tested,'physical_sd_written':False,'hdmi_hardware_validation_pending':True,
        'system_dll_imports':re.findall(r'DLL Name: (.+)',(d/'build/pe-imports.txt').read_text()),
        'source_sha256':{f.name:sha(f.read_bytes()) for f in d.iterdir() if f.is_file()}}
(o/'INSTALLATEUR-NOVA-v1.4-VERIFICATION.json').write_text(json.dumps(report,indent=2))
note="HEKATE NOVA PRO v34 alpha 7 — INSTALLATEUR WINDOWS 1.4\n\n1. Lancez Hekate-Nova-Pro-v34-alpha7-Installation-v1.4.exe sur Windows 64 bits.\n2. Choisissez la lettre SD, vérifiez son nom et sa capacité puis gardez Standard pour une console normale. 8 Go expérimental est réservé à une console physiquement modifiée en RAM.\n3. Installez, attendez la confirmation puis éjectez la carte. Démarrez avec le payload alpha7 de ce pack.\n\nL'installateur copie et vérifie les 167 fichiers intégrés. Il ne formate pas et n'injecte pas le payload. Configurations, appairages et personnalisations existants sont conservés. Les composants remplacés sont sauvegardés dans nova-backups avec rapport installation.txt. Carte FAT32/exFAT requise ; aucun téléchargement supplémentaire. Exécutable non signé.\n\nCette alpha corrige le contrôle qui coupait une image reçue avec certains docks malgré SINK_STATUS nul. Elle utilise alors une preuve supplémentaire : progression fraîche du compteur CRC du récepteur, lectures/écritures AUX réussies, source active, erreurs valides à zéro et cinq lectures consécutives de voies alignées. Les simples valeurs CRC anciennes ne suffisent pas. Exit DP n'est plus envoyé pour SIGNAL_ON seul quand le mode DP est déjà arrêté.\n\nDébits de repli, service TV dans les opérations longues, journaux différés et protections restent présents. Aucun reset PD forcé ni modification des tensions.\n\nCompilation ARM, en-têtes, 54 scénarios de service, 957 assertions du pilote et intégration aux boucles longues réussis sur PC. Contrôles de stockage repris de l'alpha6 sur sources identiques vérifiées. L'installateur est testé sur dossiers Windows avec les deux vrais packs, sans écriture sur SD physique. Aucun essai HDMI de cette alpha ici.\n\nTester d'abord une image permanente au démarrage sur dock officiel puis après retrait/remise. En cas de coupure, transmettre bootloader/tv-test/last-run.txt et les derniers run-*.txt. Les rapports doivent indiquer 34-alpha7 ; la preuve alternative est signalée par 'video stream confirmed by fresh sink CRC frames and stable lanes'. La stabilité de cette version reste à confirmer sur console avant les essais de partitionnement/copie ; sauvegarder la SD avant partitionnement.\n"
(o/'INSTALLATEUR-NOVA-v1.4-LIRE-MOI.txt').write_text(note,encoding='utf-8')
zip_path=o/'Hekate-Nova-Pro-v34-alpha7-Installateur-v1.4.zip'
with ZipFile(zip_path,'w',ZIP_DEFLATED) as z:
    for f in (exe,o/'INSTALLATEUR-NOVA-v1.4-LIRE-MOI.txt',o/'INSTALLATEUR-NOVA-v1.4-VERIFICATION.json'):
        z.write(f,f.name)
source_path=o/'Hekate-Nova-Pro-Installateur-v1.4-sources.zip'
with ZipFile(source_path,'w',ZIP_DEFLATED) as z:
    for f in d.iterdir():
        if f.is_file(): z.write(f,'nova-installer-1.4/'+f.name)
    for f in (w/'package_nova34a7.py',w/'finish_nova34a7.py'):
        z.write(f,'build-tools/'+f.name)
(o/'INSTALLATEUR-NOVA-v1.4-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n' for f in (exe,zip_path,source_path)))
state=json.loads((w/'nova34a7-status.json').read_text()); state.update({'packages_verified':True,'installer_native_tests':tested['result'],'installer_version':'1.4'})
(w/'nova34a7-status.json').write_text(json.dumps(state,indent=2))
print(json.dumps(report,indent=2))
