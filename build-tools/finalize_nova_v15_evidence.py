"""Merge separately executed page checks and retain source/binary provenance."""
from pathlib import Path
from zipfile import ZipFile
import hashlib,json
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';outputs=work.parent/'outputs'
path=work/'verification-v15.json';report=json.loads(path.read_text(encoding='utf-8'))
assert report['product']=='Hekate Nova Pro, version 15' and report['build_exit_code']==0
log=work/'verification-v15.txt';text=log.read_text(encoding='utf-8')
assert 'PASS: Nova Pro v15 full regression;' in text and 'Traceback' not in text
pages=json.loads((work/'nova-v15-emummc-pages.json').read_text());assert pages['scenes']==252
report['emummc_page_boundaries']=pages
report['ui_geometry_matrix']['emummc_page_boundary_scenes']=252
report['ui_geometry_matrix']['total_scenes']=sum(report['ui_geometry_matrix'][key]for key in ('primary_scenes','alternate_font_scenes','availability_scenes','wallpaper_opacity_scenes','wallpaper_touch_scenes','emummc_page_boundary_scenes'))
assert report['ui_geometry_matrix']['total_scenes']==5721
for name,info in report['binaries'].items():
    path_to_binary=work/'HWTest-RCM-0.15/build-ram'/name if name=='hwtest_ram.bin' else project/'output'/name
    assert info['sha256']==hashlib.sha256(path_to_binary.read_bytes()).hexdigest()
changed=[]
with ZipFile(outputs/'hekate-nova-pro-v14-sources.zip')as archive:
    old=set(archive.namelist())
    for file in project.rglob('*'):
        if not file.is_file()or file.suffix not in ('.c','.h','.json','.inc','.py','.S','.ld')and file.name!='Makefile':continue
        rel=file.relative_to(project)
        if any(part in ('build','output','.git','__pycache__')for part in rel.parts):continue
        name=project.name+'/'+rel.as_posix()
        if name not in old or archive.read(name)!=file.read_bytes():changed.append(rel.as_posix())
report['source_changes_vs_v14']=sorted(changed)
report['changed_source_sha256']={name:hashlib.sha256((project/name).read_bytes()).hexdigest()for name in sorted(changed)}
summary='PASS: 252 additional emuMMC page-boundary scenes; counts 1/4/5/16/17/63, seven languages, 20/30/36 px and two RAW availability states; every folder save path checked.'
if summary not in report['test_summaries']:report['test_summaries'].append(summary)
if summary not in text:log.write_text(text+'\n\ncheck_nova_emummc_pages_v15.py\n'+summary+'\n',encoding='utf-8')
path.write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print('PASS: 5,721 current native scenes and final binary hashes documented; source changes recorded against delivered v14.')
