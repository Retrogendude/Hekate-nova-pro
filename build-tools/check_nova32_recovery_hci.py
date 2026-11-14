from pathlib import Path
import ast,subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v32/nyx/nyx_gui';d=w/'nova32-tests';lib=p/'libs/btstack';cc=w/'toolchains/w64devkit/bin/gcc.exe'
tree=ast.parse((w/'check_nova32_negotiation.py').read_text())
c=next(ast.literal_eval(n.value)for n in tree.body if isinstance(n,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='c'for t in n.targets))
c=c[:c.index('int main(')]
c=c.replace('static int radio_health;','static int radio_health;\nstatic unsigned watch_disconnect[2],radio_opens;')
c=c.replace('nova_bt_radio_open(void){return 1;}','nova_bt_radio_open(void){radio_opens++;return 1;}')
c=c.replace('mode_count++;streaming[k]=1;input(k);','mode_count++;if(channels[k].side==1&&k<4&&streaming[k])return;streaming[k]=1;input(k);')
c=c.replace('if(streaming[k-1])input(k-1);','if(streaming[k-1]&&(side==0||k-1>=4))input(k-1);')
c=c.replace('case 0x0406:cs(op);','case 0x0406:watch_disconnect[le(a)-0x40]++;cs(op);')
c+=r"""
int main(void){
 nova_test_import(pair);radio_health=1;assert(nova_bt_start(pair));
 for(unsigned step=0;step<18000;step++){
  nova_bt_poll();consume();now++;
  const nova_bt_status *s=nova_bt_get_status();
  if(watch_disconnect[1]&&nc>=6&&s->pad[1].hid_open&&s->pad[1].encrypted&&s->pad[1].mode_ack&&s->pad[1].full_reports>=2)break;
 }
 const nova_bt_status *s=nova_bt_get_status();
 assert(watch_disconnect[1]==1);
#ifndef RECOVERY_BEFORE
 assert(s->pad[1].report_restarts==1&&s->pad[1].recoveries==1);
 assert(!s->pad[0].report_restarts&&!s->pad[0].recoveries);
#endif
 assert(!watch_disconnect[0]);
 assert(s->pad[0].full_reports>50&&s->pad[0].connected&&s->pad[0].buttons==(1u<<23));
 assert(s->pad[1].connected&&s->pad[1].buttons==8&&s->pad[1].hid_open&&s->pad[1].encrypted&&s->pad[1].mode_ack);
 assert(nc==6&&nova_test_l2cap_count()==4&&auth_count==3&&radio_opens==1&&!s->error);
 nova_bt_stop();assert(nova_test_l2cap_count()==0);
 puts("PASS real HCI/HID: a silent right stream receives a report request, reconnects and reauthenticates with retained keys; left reports continue, one radio startup, four active HID channels, Stop frees all channels");
}
"""
(d/'recovery-hci.c').write_text(c)
sources=[str(d/'pairing.o')]+[str(d/'l2cap-probe.c')if f.name=='l2cap.c'else str(f)for f in lib.rglob('*.c')]+[str(p/'bluetooth'/f)for f in ['nova_bt_stack.c','nova_bt_protocol.c','nova_bt_trace.c']]
args=[str(cc),'-std=gnu11','-O1','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(d/'known-config'),'-I'+str(p/'bluetooth'),'-I'+str(lib),str(d/'recovery-hci.c'),*sources,'-o',str(d/'recovery-hci.exe')]
r=subprocess.run(args,capture_output=True,text=True);assert r.returncode==0,r.stderr
r=subprocess.run([str(d/'recovery-hci.exe')],capture_output=True,text=True,timeout=15);print(r.stdout,r.stderr);assert r.returncode==0,r.returncode
old=w/'hekate-nova-pro-v31/nyx/nyx_gui'
old_sources=[str(d/'pairing.o')]+[str(d/'l2cap-probe.c')if f.name=='l2cap.c'else str(f)for f in lib.rglob('*.c')]+[str(old/'bluetooth'/f)for f in ['nova_bt_stack.c','nova_bt_protocol.c','nova_bt_trace.c']]
old_args=[str(cc),'-std=gnu11','-O1','-DRECOVERY_BEFORE','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(d/'known-config'),'-I'+str(old/'bluetooth'),'-I'+str(lib),str(d/'recovery-hci.c'),*old_sources,'-o',str(d/'recovery-hci-before.exe')]
a=subprocess.run(old_args,capture_output=True,text=True);assert a.returncode==0,a.stderr
before=subprocess.run([str(d/'recovery-hci-before.exe')],capture_output=True,text=True,timeout=15);assert before.returncode==1 and 'watch_disconnect[1]==1' in before.stderr,(before.returncode,before.stdout,before.stderr)
print('PASS: actual v31 reproduces a silent right HID link with no automatic reconnect')
(w/'nova32-recovery-hci-check.json').write_text(json.dumps({'passed':True,'physical_radio':False,'output':r.stdout,'v31_regression':before.stderr.strip(),'source_sha256':{n:hashlib.sha256((p/'bluetooth'/n).read_bytes()).hexdigest()for n in ['nova_bt.h','nova_bt_stack.c','nova_bt_protocol.c']}},indent=2))
