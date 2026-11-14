"""Exercise the actual C planner without linking any storage I/O."""
from pathlib import Path
from ctypes import Structure, c_uint32, c_int, POINTER, CDLL
from zipfile import ZipFile
import hashlib,json,random,re,subprocess

work=Path(__file__).resolve().parent
project=work/'hekate-6.5.4';front=project/'nyx/nyx_gui/frontend'
directory=work/'partition-tests-v16';directory.mkdir(exist_ok=True)
library=directory/'plan.dll'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11','-shared','-Wall','-Wextra','-Werror','-O2','-I'+str(project/'bdk'),str(front/'gui_partition_plan.c'),'-o',str(library)],check=True)
class Plan(Structure):
    _fields_=[(key,c_uint32)for key in ['total_sectors','fat_min_mib','emu_mib','linux_mib','android_mib','fat_mib','alignment_sectors']]+[('emu_double',c_int)]
native=CDLL(str(library));native.nova_partition_validate.argtypes=[POINTER(Plan)]
native.nova_partition_simplified.argtypes=[POINTER(Plan),*[c_uint32]*5]
native.nova_partition_widths.argtypes=[POINTER(Plan),c_uint32,POINTER(c_uint32)]
count=0
def check(condition):
    global count
    count+=1;assert condition,count
def valid(plan):
    check(native.nova_partition_validate(plan)==0)
    total=(plan.total_sectors//32768*32768)>>11
    check(plan.fat_mib+plan.emu_mib+plan.linux_mib+plan.android_mib+16==total)
    check(plan.fat_mib>=plan.fat_min_mib)
    for width in (1,17,1164,1212,65535):
        widths=(c_uint32*4)();native.nova_partition_widths(plan,width,widths)
        check(sum(widths)==width)
        for size,actual in zip((plan.fat_mib,plan.emu_mib,plan.linux_mib,plan.android_mib),widths):
            check(actual<=width and (size!=0 or actual==0))
for model in range(4):
    full=59680 if model==3 else 29856;data=59640 if model==3 else 29820
    for linux in range(3):
        for android in range(3):
            p=Plan(total_sectors=256*2097152+123,fat_min_mib=2048)
            check(native.nova_partition_simplified(p,model,model,data,linux,android)==0)
            check(p.emu_mib==full)
            check(p.linux_mib==(0,16384,32768)[linux])
            check(p.android_mib==(0,16384,32768)[android])
            if android:check(p.android_mib-6144==(10240,26624)[android-1])
            valid(p)
            p.total_sectors=(full+(0,16384,32768)[linux]+(0,16384,32768)[android]+2048+16)*2048
            # Exact minimum FAT32 after technical alignment is accepted.
            check(native.nova_partition_validate(p)==0)
            p.total_sectors-=32768;check(native.nova_partition_validate(p)==1 and p.fat_mib==0)
    for chosen in range(4):
        p=Plan(total_sectors=256*2097152,fat_min_mib=2048)
        check(native.nova_partition_simplified(p,model,chosen,data,0,0)==(0 if model==chosen else 2))
    check(native.nova_partition_simplified(p,model,model,0,0,0)==3)
    check(native.nova_partition_simplified(p,model,model,full,0,0)==4)
for total in (0,1,32767,32768,16*2048,2048*2048):
    p=Plan(total_sectors=total,fat_min_mib=2048);check(native.nova_partition_validate(p)==1)
for key,value,error in [('linux_mib',4096,5),('android_mib',8192,6),('emu_mib',4096,7),('emu_mib',0xFFFFFFFF,7),('linux_mib',0xFFFFFC00,1),('linux_mib',8193,8)]:
    p=Plan(total_sectors=0xFFFFFFFF,fat_min_mib=2048);setattr(p,key,value);check(native.nova_partition_validate(p)==error)
p=Plan(total_sectors=0xFFFFFFFF,fat_min_mib=2048,emu_double=True);check(native.nova_partition_validate(p)==7)
rng=random.Random(16)
for _ in range(5000):
    copies=rng.choice((1,2));full=rng.choice((0,29856,59680,32+rng.randrange(4,25)*1024))
    p=Plan(total_sectors=rng.randrange(0,0x100000000),fat_min_mib=rng.choice((1024,2048)),emu_mib=full*copies,emu_double=bool(full and copies==2),linux_mib=rng.choice((0,8192,16384,32768)),android_mib=rng.choice((0,10240,16384,32768)))
    total=(p.total_sectors//32768*32768)>>11
    expected=p.emu_mib+p.linux_mib+p.android_mib+16+p.fat_min_mib<=total
    check((native.nova_partition_validate(p)==0)==expected)
    if expected:valid(p)

# The operations that write partitions, copy data, flash installers and fix
# attributes must be byte-for-byte identical to the previously delivered source.
def function(text,name):
    start=re.search(r'^(?:static )?[^\n]+\b'+name+r'\([^;]*?\)\s*\n\{',text,re.M);assert start,name
    end=text.index('\n}',start.end())+2
    return text[start.start():end]
preserved={}
with ZipFile(work/'baseline-v15-sources.zip')as archive:
    for filename,names in {
        'gui_tools_partition_manager.c':['_create_gpt_partition','_sd_prepare_and_flash_mbr_gpt','_emmc_prepare_and_flash_mbr_gpt','_stat_and_copy_files','_copy_file','_backup_and_restore_files','_action_flash_linux_data','_action_flash_android_data','_action_fix_mbr_gpt'],
        'gui_tools.c':['_fix_attributes'],
        'gui_emmc_tools.c':['_emmc_backup_buttons_decider','_emmc_backup_buttons_raw_toggle']
    }.items():
        before=archive.read('hekate-6.5.4/nyx/nyx_gui/frontend/'+filename).decode('utf-8').replace('\r\n','\n')
        after=(front/filename).read_text(encoding='utf-8')
        for name in names:
            a,b=function(before,name),function(after,name);check(a==b)
            preserved[name]=hashlib.sha256(b.encode()).hexdigest()
    before=archive.read('hekate-6.5.4/nyx/nyx_gui/frontend/gui_tools.c').decode('utf-8').replace('\r\n','\n')
    after=(front/'gui_tools.c').read_text(encoding='utf-8')
    for text in (before,after):assert '\n\tif (sd_mount())' in function(text,'_create_window_dump_pk12_tool')
    a=function(before,'_create_window_dump_pk12_tool');b=function(after,'_create_window_dump_pk12_tool')
    check(a[a.index('\n\tif (sd_mount())'):]==b[b.index('\n\tif (sd_mount())'):])
(work/'partition-check-v16.json').write_text(json.dumps({'assertions':count,'preserved_storage_functions':preserved,'random_plans':5000,'hardware_io':'not linked or executed'},indent=2))
print(f'PASS: {count} assertions on actual C partition planner; original storage/extraction mechanisms preserved.')
