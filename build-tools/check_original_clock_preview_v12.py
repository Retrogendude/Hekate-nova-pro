from pathlib import Path
p=Path(__file__).with_name('render_lvgl_preview_v12.py');s=p.read_text()
s=s.replace('ACTION(_joycon_info_dump_action) ACTION(_create_mbox_clock_edit)','ACTION(_joycon_info_dump_action)')
s=s.replace('static lv_style_t hint_small_style, hint_small_style_white;','static lv_style_t hint_small_style, hint_small_style_white,mbox_darken;\nstatic lv_res_t _create_mbox_clock_edit(lv_obj_t *button);')
s=s.replace("body+=rtc[at:end]+'\\n'+stripped(nyx/'frontend/gui_nova_setup.c')",'''body+=rtc[at:end]+'\\n'
body+='\\n'.join(re.findall(r'^#define CLOCK_[^\\n]*',opts,re.M))+'\\n'
body+=function(gui,'nyx_create_onoff_button')+function(gui,'nyx_mbox_action')
body+=opts[opts.index('typedef struct _time_edit_obj_t'):opts.index('static lv_res_t _joycon_info_dump_action(')]
''')
s=s.replace('else if(!strcmp(argv[3],"setup"))nyx_nova_first_setup(NULL);','else if(!strcmp(argv[3],"setup")){first_time_clock_edit(NULL);root=lv_obj_get_child(lv_obj_get_child(lv_scr_act(),NULL),NULL);}')
s=s.replace('assert(nova_setup.open&&nova_setup.language==n_cfg.language&&!n_cfg.setup_done);for(u32 i=0;i<5;++i)assert(nova_setup.values[i]);','assert(clock_language==n_cfg.language&&clock_ctxt.year&&clock_ctxt.month&&clock_ctxt.day&&clock_ctxt.hour&&clock_ctxt.min);assert(type_is(root,"lv_mbox"));')
s=s.replace('PASS: bounded border reflection','PASS: white diagonal glyph sheen')
p.write_text(s,encoding='utf-8')
print('Native preview now exercises the original clock dialog and its added languages')
