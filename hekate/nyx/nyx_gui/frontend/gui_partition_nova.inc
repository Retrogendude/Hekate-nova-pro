/* Nova partition manager UI. Included after the existing storage actions.
 * Storage/format/flash implementations remain in gui_tools_partition_manager.c. */
static struct {
    lv_obj_t *win,*page,*content,*models,*enabled[2],*presets[2];
    lv_obj_t *sliders[3],*summary,*message,*capacity,*segments[4];
    lv_obj_t *mode_choices,*summary_panel,*summary_title,*overview,*legend;
    lv_style_t segment_style[4],slider_bg[3],slider_ind[3],slider_knob[3];
    u32 mode,model,actual_model,option[2],preset[2];
    bool ready;
    nova_plan_result result;
} partition_nova;
static void nova_partition_layout(void)
{
    bool expert=partition_nova.mode||part_info.emmc;int height=lv_obj_get_height(partition_nova.page);
    lv_obj_set_height(partition_nova.overview,expert?56:84);
    lv_obj_set_hidden(partition_nova.legend,expert);
    lv_obj_set_pos(partition_nova.content,34,expert?140:172);
    lv_obj_set_size(partition_nova.content,expert?1212:744,expert?height-318:height-256);
    lv_obj_set_pos(partition_nova.summary_panel,expert?34:802,expert?height-166:172);
    lv_obj_set_size(partition_nova.summary_panel,expert?1212:444,expert?90:height-256);
    lv_obj_set_pos(partition_nova.summary_title,expert?20:22,expert?6:14);
    lv_obj_set_width(partition_nova.summary_title,expert?1172:400);
    lv_label_set_style(partition_nova.summary_title,(lv_style_t *)nyx_nova_text_style(expert?NOVA_KICKER:NOVA_TITLE));
    nyx_nova_fit_text(partition_nova.summary_title,expert?20:38);
    lv_obj_set_pos(partition_nova.summary,expert?20:22,expert?28:56);lv_obj_set_width(partition_nova.summary,expert?1172:400);
    lv_obj_set_pos(partition_nova.message,expert?20:22,expert?58:172);lv_obj_set_width(partition_nova.message,expert?1172:400);
}
static void nova_partition_emu_text(char *text,const nova_partition_plan *plan)
{
    u32 full=part_info.emmc_is_64gb?EMU_64GB_FULL:EMU_32GB_FULL;
    if(plan->emu_mib&&plan->emu_mib==(plan->emu_double?2*full:full))
        s_printf(text,nyx_tr(plan->emu_double?"2 x %d Go Full":"%d Go Full"),full>>10);
    else if(plan->emu_double)s_printf(text,nyx_tr("2 x %d Go"),plan->emu_mib>>11);
    else s_printf(text,nyx_tr("%d Go"),plan->emu_mib>>10);
}

static const char *nova_partition_error(nova_plan_result result)
{
    switch(result){
    case NOVA_PLAN_OK:return "Disposition valide. Les réserves techniques sont incluses.";
    case NOVA_PLAN_CAPACITY:return "Capacité insuffisante : conserver l'espace minimum pour FAT32 / USER.";
    case NOVA_PLAN_MODEL:return "Le modèle sélectionné ne correspond pas à cette console.";
    case NOVA_PLAN_EMMC_UNAVAILABLE:return "eMMC indisponible : sa taille ne peut pas être vérifiée.";
    case NOVA_PLAN_EMMC_UPGRADED:return "eMMC modifiée : utiliser le mode expert pour cette console.";
    case NOVA_PLAN_LINUX_SIZE:return "La partition Linux est trop petite.";
    case NOVA_PLAN_ANDROID_SIZE:return "Android doit inclure ses partitions système et ses données.";
    default:return "Disposition incompatible. Vérifier les tailles avant de continuer.";
    }
}

static nova_plan_result nova_partition_current(nova_partition_plan *plan)
{
    memset(plan,0,sizeof(*plan));
    plan->total_sectors=part_info.total_sct;
    plan->fat_min_mib=part_info.hos_min_size;
    if(!part_info.emmc&&!partition_nova.mode)
        return nova_partition_simplified(plan,partition_nova.actual_model,partition_nova.model,
            part_info.emmc_size_mb,partition_nova.option[0],partition_nova.option[1]);
    plan->emu_mib=part_info.emu_size;plan->emu_double=part_info.emu_double;
    plan->linux_mib=part_info.l4t_size;plan->android_mib=part_info.and_size;
    if(plan->emu_mib&&!part_info.emmc_size_mb)return NOVA_PLAN_EMMC_UNAVAILABLE;
    return nova_partition_validate(plan);
}

static bool nova_partition_preflight(void)
{
    nova_partition_plan plan;
    if(!partition_nova.ready||nova_partition_current(&plan)!=NOVA_PLAN_OK){
        nyx_show_help("Disposition incompatible. Vérifier les tailles avant de continuer.");return false;
    }
    u32 sectors=0;
    if(part_info.emmc){
        if(!emmc_initialize(false)){
            emmc_set_partition(EMMC_GPP);
            if(emmc_storage.sec_cnt>HOS_USER_SECTOR)sectors=emmc_storage.sec_cnt-HOS_USER_SECTOR;
            emmc_end();
        }
    }else if(!sd_mount()){sectors=sd_storage.sec_cnt;sd_unmount();}
    if(!sectors||ALIGN_DOWN(sectors,AU_ALIGN_SECTORS)!=part_info.total_sct){
        nyx_show_help("Support indisponible ou capacité modifiée. Rouvrir le gestionnaire de partitions.");return false;
    }
    part_info.hos_size=plan.fat_mib;
    return true;
}

static void nova_partition_refresh(void)
{
    if(!partition_nova.ready)return;
    nova_partition_plan plan;partition_nova.result=nova_partition_current(&plan);
    bool valid=partition_nova.result==NOVA_PLAN_OK;
    part_info.emu_size=plan.emu_mib;part_info.l4t_size=plan.linux_mib;
    part_info.and_size=plan.android_mib;part_info.emu_double=plan.emu_double;
    if(valid)part_info.hos_size=plan.fat_mib;
    char text[320];
    bool expert=partition_nova.mode||part_info.emmc;
    if(expert){char emu[64];nova_partition_emu_text(emu,&plan);
        s_printf(text,nyx_tr("%s : %d Go  |  emuMMC : %s  |  Linux : %d Go  |  Android : %d Go"),part_info.emmc?"USER":"FAT32",plan.fat_mib>>10,emu,plan.linux_mib>>10,plan.android_mib>>10);
    }else s_printf(text,"%s: %d MiB\nemuMMC%s: %d MiB\nLinux: %d MiB\nAndroid: %d MiB",part_info.emmc?"USER":"FAT32",plan.fat_mib,plan.emu_double?" (2x)":"",plan.emu_mib,plan.linux_mib,plan.android_mib);
    lv_label_set_text(partition_nova.summary,text);
    nyx_nova_fit_text(partition_nova.summary,expert?28:110);
    lv_label_set_text(partition_nova.message,nova_partition_error(partition_nova.result));
    nyx_nova_fit_text(partition_nova.message,expert?24:70);
    lv_obj_set_click(part_info.partition_button,valid);
    lv_btn_set_state(part_info.partition_button,valid?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    if(expert)s_printf(text,nyx_tr("Capacité : %d Go"),part_info.total_sct>>21);
    else s_printf(text,nyx_tr("Capacité : %d MiB (%d,%02d Go) · réserves : %d MiB"),(part_info.total_sct>>11),(part_info.total_sct>>11)/1024,((part_info.total_sct>>11)%1024)*100/1024,16);
    lv_label_set_text(partition_nova.capacity,text);
    nyx_nova_fit_text(partition_nova.capacity,expert?22:28);
    u32 widths[4];nova_partition_widths(&plan,1164,widths);int x=24;
    for(u32 i=0;i<4;++i){
        lv_obj_set_hidden(partition_nova.segments[i],!plan.fat_mib||!widths[i]);
        lv_obj_set_pos(partition_nova.segments[i],x,expert?36:42);
        lv_obj_set_height(partition_nova.segments[i],expert?10:12);
        lv_obj_set_width(partition_nova.segments[i],widths[i]?widths[i]:1);x+=widths[i];
    }
    lv_obj_t *labels[]={part_info.lbl_hos,part_info.lbl_emu,part_info.lbl_l4t,part_info.lbl_and};
    const u32 values[]={plan.fat_mib,plan.emu_mib,plan.linux_mib,plan.android_mib};
    for(u32 i=0;i<4;++i)if(labels[i]){
        if(expert&&i==1)nova_partition_emu_text(text,&plan);
        else if(expert)s_printf(text,nyx_tr("%d Go"),values[i]>>10);
        else s_printf(text,nyx_tr("%d,%02d Go"),values[i]/1024,(values[i]%1024)*100/1024);
        lv_label_set_text(labels[i],text);nyx_nova_fit_text(labels[i],expert?26:32);
    }
    if(!partition_nova.mode&&!part_info.emmc){
        for(u32 i=0;i<2;++i)if(partition_nova.presets[i])
            for(lv_obj_t *button=lv_obj_get_child(partition_nova.presets[i],NULL);button;button=lv_obj_get_child(partition_nova.presets[i],button)){
                bool on=partition_nova.option[i]!=0;lv_obj_set_click(button,on);
                lv_btn_set_state(button,on?(lv_obj_get_free_num(button)==partition_nova.preset[i]?LV_BTN_STATE_TGL_REL:LV_BTN_STATE_REL):LV_BTN_STATE_INA);
            }
    }
}

static lv_res_t nova_partition_simple_choice(lv_obj_t *button)
{
    u32 group=lv_obj_get_free_num(lv_obj_get_parent(button)),value=lv_obj_get_free_num(button);
    if(group==70)partition_nova.model=value==0?(partition_nova.actual_model<2?partition_nova.actual_model:0):value+1;
    else if(group==71||group==72){u32 index=group-71;partition_nova.option[index]=value?partition_nova.preset[index]+1:0;}
    else if(group==73||group==74){u32 index=group-73;partition_nova.preset[index]=value;partition_nova.option[index]=value+1;}
    nova_partition_refresh();return LV_RES_OK;
}

static lv_res_t nova_partition_expert_slider(lv_obj_t *slider)
{
    u32 index=lv_obj_get_free_num(slider);
    if(index==0)return _action_slider_emu(slider);
    if(index==1)return _action_slider_l4t(slider);
    /* The original callback takes Android USER size; this UI shows the
     * total allocation, including the existing 6-GiB system reservation. */
    int total=lv_slider_get_value(slider);
    lv_slider_set_value(slider,total>=10?total-6:0);
    _action_slider_and(slider);
    lv_slider_set_value(slider,part_info.and_size>>10);
    return LV_RES_OK;
}

static u32 nova_partition_emu_position(void)
{
    u32 full=part_info.emmc_is_64gb?EMU_64GB_FULL:EMU_32GB_FULL;
    if(!part_info.emu_size)return 0;
    if(part_info.emu_size==full)return EMU_SLIDER_1X_FULL;
    if(part_info.emu_size==full*2)return EMU_SLIDER_2X_FULL;
    u32 size=part_info.emu_double?part_info.emu_size/2:part_info.emu_size;
    return ((size-EMU_RSVD_MB)>>10)-EMU_SLIDER_OFFSET+(part_info.emu_double?EMU_SLIDER_1X_MAX:0);
}
static lv_res_t nova_partition_step(lv_obj_t *button)
{
    u32 code=lv_obj_get_free_num(button),index=code>>1;if(index>=3)return LV_RES_OK;
    lv_obj_t *slider=partition_nova.sliders[index];if(!slider)return LV_RES_OK;
    int old=lv_slider_get_value(slider),value=old+(code&1?1:-1);
    if(index){int minimum=index==1?8:10;
        if((code&1)&&old==0)value=minimum;
        else if(!(code&1)&&old==minimum)value=0;
    }
    if(value<lv_slider_get_min_value(slider))value=lv_slider_get_min_value(slider);
    if(value>lv_slider_get_max_value(slider))value=lv_slider_get_max_value(slider);
    lv_slider_set_value(slider,value);return nova_partition_expert_slider(slider);
}

static void nova_partition_build_controls(void)
{
    lv_obj_clean(partition_nova.content);
    nova_partition_layout();
    part_info.lbl_hos=part_info.lbl_emu=part_info.lbl_l4t=part_info.lbl_and=NULL;
    memset(partition_nova.sliders,0,sizeof(partition_nova.sliders));
    memset(partition_nova.presets,0,sizeof(partition_nova.presets));
    lv_obj_t *panel=partition_nova.content;
    if(!partition_nova.mode&&!part_info.emmc){
        nyx_nova_text(panel,"Modèle de console",24,12,696,NOVA_SMALL);
        partition_nova.models=nyx_choices_create(panel,"Switch V1 V2\nSwitch Lite\nSwitch OLED",partition_nova.model<2?0:partition_nova.model-1,3,696,38,nova_partition_simple_choice,70,true);
        lv_obj_set_pos(partition_nova.models,24,40);
        const char *titles[]={"Installer Linux ?","Installer Android ?"};
        for(u32 i=0;i<2;++i){
            int y=100+i*72;
            lv_obj_t *title=nyx_nova_text(panel,titles[i],24,y+7,205,NOVA_BODY);nyx_nova_fit_text(title,38);
            partition_nova.enabled[i]=nyx_choices_create(panel,"Non\nOui",partition_nova.option[i]?1:0,2,180,40,nova_partition_simple_choice,71+i,false);
            lv_obj_set_pos(partition_nova.enabled[i],240,y);
            partition_nova.presets[i]=nyx_choices_create(panel,"16 Go\n32 Go",partition_nova.preset[i],2,282,58,nova_partition_simple_choice,73+i,false);
            lv_obj_set_pos(partition_nova.presets[i],438,y);
            for(lv_obj_t *choice=lv_obj_get_child(partition_nova.presets[i],NULL);choice;choice=lv_obj_get_child(partition_nova.presets[i],choice)){
                lv_obj_t *size=lv_obj_get_child(choice,NULL);lv_obj_set_y(size,4);nyx_nova_fit_text(size,24);
                lv_obj_t *hint=nyx_nova_text(choice,lv_obj_get_free_num(choice)?"Recommandé":"Minimum",4,32,lv_obj_get_width(choice)-8,NOVA_KICKER);
                static lv_style_t caption;lv_style_copy(&caption,nyx_nova_text_style(NOVA_KICKER));caption.text.letter_space=0;lv_label_set_style(hint,&caption);
                lv_label_set_align(hint,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(hint,20);
            }
        }
    }else{
        const char *titles[]={"emuMMC","Linux","Android (total)"};
        const u32 values[]={nova_partition_emu_position(),part_info.l4t_size>>10,part_info.and_size>>10};
        const u32 maximum[]={EMU_SLIDER_MAX,(part_info.total_sct>>21)>2?(part_info.total_sct>>21)-2:0,(part_info.total_sct>>21)>2?(part_info.total_sct>>21)-2:0};
        for(u32 i=0;i<3;++i){
            int y=8+i*60;
            if(i==0&&part_info.emmc){nyx_nova_text(panel,"eMMC USER",24,y,500,NOVA_BODY);part_info.lbl_hos=nyx_nova_text(panel,"",820,y,350,NOVA_SMALL);continue;}
            lv_obj_t *title=nyx_nova_text(panel,titles[i],24,y,500,NOVA_BODY);nyx_nova_fit_text(title,26);
            lv_obj_t *slider=lv_slider_create(panel,NULL);partition_nova.sliders[i]=slider;
            lv_obj_set_pos(slider,76,y+34);lv_obj_set_size(slider,1060,24);
            lv_slider_set_range(slider,0,maximum[i]>32760?32760:maximum[i]);lv_slider_set_value(slider,values[i]);
            lv_obj_set_free_num(slider,i);lv_slider_set_action(slider,nova_partition_expert_slider);
            lv_slider_set_style(slider,LV_SLIDER_STYLE_BG,&partition_nova.slider_bg[i]);
            lv_slider_set_style(slider,LV_SLIDER_STYLE_INDIC,&partition_nova.slider_ind[i]);
            lv_slider_set_style(slider,LV_SLIDER_STYLE_KNOB,&partition_nova.slider_knob[i]);
            for(unsigned direction=0;direction<2;direction++){
                lv_obj_t *step=lv_btn_create(panel,NULL);lv_btn_set_fit(step,false,false);lv_obj_set_size(step,40,34);lv_obj_set_pos(step,direction?1148:24,y+26);
                lv_obj_set_free_num(step,i*2+direction);lv_btn_set_action(step,LV_BTN_ACTION_CLICK,nova_partition_step);
                lv_btn_set_action(step,LV_BTN_ACTION_LONG_PR_REPEAT,nova_partition_step);
                lv_obj_t *sign=lv_label_create(step,NULL);lv_label_set_text(sign,direction?"+":"-");
            }
            lv_obj_t *value=nyx_nova_text(panel,"",820,y,350,NOVA_SMALL);nyx_nova_fit_text(value,26);
            if(i==0)part_info.lbl_emu=value;else if(i==1)part_info.lbl_l4t=value;else part_info.lbl_and=value;
        }
    }
    if(!part_info.lbl_hos){part_info.lbl_hos=lv_label_create(panel,NULL);lv_obj_set_hidden(part_info.lbl_hos,true);}
    nova_partition_refresh();
}

static lv_res_t nova_partition_mode(lv_obj_t *button)
{
    partition_nova.mode=lv_obj_get_free_num(button);
    if(partition_nova.mode&&partition_nova.result!=NOVA_PLAN_OK){
        part_info.emu_size=part_info.l4t_size=part_info.and_size=0;part_info.emu_double=false;
    }
    nova_partition_build_controls();return LV_RES_OK;
}

static lv_res_t nova_partition_notes(lv_obj_t *button)
{
    (void)button;
    nyx_show_help(part_info.emmc?
        "Le mode expert conserve le partitionnement manuel. La partition USER sera formatée après confirmation.\nLes fonctions d'installation Linux et Android restent disponibles.":
        "Le mode simplifié utilise une emuMMC complète : 29 Go pour V1/V2/Lite, 58 Go pour OLED (valeurs arrondies).\nLinux et Android sont indépendants : Minimal = 16 Go, Recommandé = 32 Go par système. Android inclut ses partitions système.\nAu moins 2 Go restent pour FAT32. Les alignements et réserves techniques sont comptés.\nL'emuMMC ne sera créée qu'avec l'outil dédié après le partitionnement. Réserver Linux/Android n'installe pas leurs fichiers.\nLa sauvegarde automatique existante est limitée à 1,2 Go ; au-delà, une copie manuelle sera demandée. Une emuMMC redimensionnée reformate USER.");
    return LV_RES_OK;
}

static lv_res_t nova_partition_review_action(lv_obj_t *buttons,const char *text)
{
    u32 selected=lv_btnm_get_pressed(buttons);nyx_mbox_action(buttons,text);
    if(!selected)_create_mbox_partitioning_next_legacy(NULL);
    return LV_RES_INV;
}

static lv_res_t nova_partition_review(lv_obj_t *button)
{
    (void)button;nova_partition_plan plan;nova_plan_result result=nova_partition_current(&plan);
    if(result!=NOVA_PLAN_OK){nyx_show_help(nova_partition_error(result));return LV_RES_OK;}
    part_info.hos_size=plan.fat_mib;
    char message[768];
    if(partition_nova.mode||part_info.emmc){char emu[64];nova_partition_emu_text(emu,&plan);
        s_printf(message,nyx_tr("Vérifier la disposition\n\nFAT32 / USER : %d Go\nemuMMC : %s\nLinux : %d Go\nAndroid : %d Go\nRéserves et alignements inclus.\n\nLa confirmation d'effacement existante sera affichée à l'étape suivante."),plan.fat_mib>>10,emu,plan.linux_mib>>10,plan.android_mib>>10);
    }else s_printf(message,nyx_tr("Vérifier la disposition\n\nFAT32 / USER : %d MiB\nemuMMC : %d MiB\nLinux : %d MiB\nAndroid : %d MiB\nRéserves : 16 MiB + alignement\n\nLa confirmation d'effacement existante sera affichée à l'étape suivante."),plan.fat_mib,plan.emu_mib,plan.linux_mib,plan.android_mib);
    lv_obj_t *background=lv_obj_create(lv_scr_act(),NULL);lv_obj_set_style(background,&mbox_darken);lv_obj_set_size(background,LV_HOR_RES,LV_VER_RES);
    lv_obj_t *box=lv_mbox_create(background,NULL);lv_obj_set_width(box,1000);lv_mbox_set_text(box,message);
    static const char *buttons[]={"\222Continue","\222Cancel",""};lv_mbox_add_btns(box,buttons,nova_partition_review_action);
    lv_obj_align(box,NULL,LV_ALIGN_CENTER,0,0);return LV_RES_OK;
}

static void nova_partition_create_ui(lv_obj_t *win)
{
    memset(&partition_nova,0,sizeof(partition_nova));partition_nova.win=win;
    partition_nova.page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(partition_nova.page);
    switch(fuse_read_hw_type()){
    case FUSE_NX_HW_TYPE_IOWA:partition_nova.actual_model=1;break;
    case FUSE_NX_HW_TYPE_HOAG:partition_nova.actual_model=2;break;
    case FUSE_NX_HW_TYPE_AULA:partition_nova.actual_model=3;break;
    default:partition_nova.actual_model=0;break;
    }
    partition_nova.model=partition_nova.actual_model;partition_nova.mode=part_info.emmc?1:0;
    int height=lv_obj_get_height(partition_nova.page);
    if(!part_info.emmc){
        partition_nova.mode_choices=nyx_choices_create(partition_nova.page,"Mode simplifié\nMode expert",0,2,1212,42,nova_partition_mode,75,false);
        lv_obj_set_pos(partition_nova.mode_choices,34,14);
    }else nyx_nova_text(partition_nova.page,"Mode expert — eMMC interne",34,22,1212,NOVA_TITLE);
    lv_obj_t *overview=nyx_nova_panel(partition_nova.page,34,72,1212,84);partition_nova.overview=overview;
    partition_nova.capacity=nyx_nova_text(overview,"",24,12,1164,NOVA_SMALL);
    const u32 colors[]={0x55C9F1,0xA98BFA,0x8AE6B6,0xF4B85F};
    for(u32 i=0;i<4;++i){
        lv_style_copy(&partition_nova.segment_style[i],&lv_style_plain);
        partition_nova.segment_style[i].body.main_color=partition_nova.segment_style[i].body.grad_color=LV_COLOR_HEX(colors[i]);
        partition_nova.segment_style[i].body.radius=0;partition_nova.segment_style[i].body.border.width=0;
        lv_obj_t *segment=lv_cont_create(overview,NULL);lv_cont_set_fit(segment,false,false);lv_cont_set_style(segment,&partition_nova.segment_style[i]);
        lv_obj_set_size(segment,1,12);lv_obj_set_click(segment,false);partition_nova.segments[i]=segment;
    }
    partition_nova.legend=nyx_nova_text(overview,"FAT32  ·  emuMMC  ·  Linux  ·  Android",24,61,1164,NOVA_KICKER);
    for(u32 i=0;i<3;++i){
        lv_style_copy(&partition_nova.slider_bg[i],lv_theme_get_current()->slider.bg);
        lv_style_copy(&partition_nova.slider_ind[i],lv_theme_get_current()->slider.indic);
        lv_style_copy(&partition_nova.slider_knob[i],lv_theme_get_current()->slider.knob);
        partition_nova.slider_bg[i].body.radius=2;partition_nova.slider_bg[i].body.border.width=0;
        partition_nova.slider_ind[i].body.radius=2;partition_nova.slider_ind[i].body.main_color=partition_nova.slider_ind[i].body.grad_color=LV_COLOR_HEX(colors[i+1]);
        partition_nova.slider_knob[i].body.radius=3;partition_nova.slider_knob[i].body.border.width=1;
    }
    partition_nova.content=nyx_nova_panel(partition_nova.page,34,172,744,height-256);
    lv_obj_t *summary_panel=nyx_nova_panel(partition_nova.page,802,172,444,height-256);
    partition_nova.summary_panel=summary_panel;
    partition_nova.summary_title=nyx_nova_text(summary_panel,"Récapitulatif",22,14,400,NOVA_TITLE);
    partition_nova.summary=nyx_nova_text(summary_panel,"",22,56,400,NOVA_SMALL);
    partition_nova.message=nyx_nova_text(summary_panel,"",22,172,400,NOVA_KICKER);
    part_info.slider_bar_hos=lv_bar_create(partition_nova.page,NULL);lv_obj_set_hidden(part_info.slider_bar_hos,true);lv_bar_set_range(part_info.slider_bar_hos,0,32760);
    lv_obj_t *tools=lv_btn_create(partition_nova.page,NULL);lv_btn_set_fit(tools,false,false);lv_obj_set_size(tools,370,46);lv_obj_set_pos(tools,34,height-64);
    lv_btn_set_action(tools,LV_BTN_ACTION_CLICK,part_info.emmc?_action_check_flash_linux:_action_part_manager_ums_sd);
    lv_obj_t *label=lv_label_create(tools,NULL);lv_label_set_text(label,part_info.emmc?"Flash Linux":"SD UMS");
    btn_flash_l4t=lv_btn_create(partition_nova.page,tools);lv_obj_set_pos(btn_flash_l4t,418,height-64);lv_btn_set_action(btn_flash_l4t,LV_BTN_ACTION_CLICK,_action_check_flash_linux);
    label=lv_label_create(btn_flash_l4t,NULL);lv_label_set_text(label,"Flash Linux");
    btn_flash_android=lv_btn_create(partition_nova.page,tools);lv_obj_set_pos(btn_flash_android,802,height-64);lv_btn_set_action(btn_flash_android,LV_BTN_ACTION_CLICK,_action_flash_android);
    label=lv_label_create(btn_flash_android,NULL);lv_label_set_text(label,"Flash Android");
    if(part_info.emmc)lv_obj_set_hidden(tools,true);
    /* Existing install actions stay visible below the allocation controls. */
    lv_win_add_btn(win,NULL,"Notes",nova_partition_notes);
    part_info.partition_button=lv_win_add_btn(win,NULL,"Next Step",nova_partition_review);
    partition_nova.ready=true;nova_partition_build_controls();
}
