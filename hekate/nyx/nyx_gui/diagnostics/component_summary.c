#include "component_summary.h"
#include "diag_format.h"
#include "../localization.h"
#include <stdlib.h>
#include <string.h>

static char manual_observations[NYX_MANUAL_COUNT][1024];
static bool manual_warnings[NYX_MANUAL_COUNT];
void nyx_component_observation(u32 kind,const char *text,bool warning)
{
    if(kind>=NYX_MANUAL_COUNT)return;
    nyx_diag_snprintf(manual_observations[kind],sizeof(manual_observations[kind]),"%s",text?text:"");
    manual_warnings[kind]=warning&&manual_observations[kind][0];
}
static int manual_kind(u32 component)
{return component==13?NYX_MANUAL_COMMANDS:component==12?NYX_MANUAL_TOUCH:component==11?NYX_MANUAL_DISPLAY:component==1?NYX_MANUAL_RAM:-1;}
static const nyx_component_definition components[]={
 {"Processeur / SoC","Identité SoC et démarrage du service CPU0. Aucun stress CPU.","Contrôler les alimentations et horloges CPU, puis les liaisons et les erreurs du service.",{"cpu_complex"},{1,27,28},1,1,3,0},
 {"Mémoire / horloges","Symétrie DRAM et verrouillage PLL. Le test RAM isolé est distinct.","Comparer les rails mémoire, l'identité DRAM et les horloges. Compléter par le service RAM isolé.",{"dram_sym","plls"},{14,15},2,2,2,0},
 {"eMMC","Santé déclarée, GPT, lectures et données logiques lorsqu'elles sont accessibles.","Vérifier alimentation, bus eMMC et contenu logique. Une GPT invalide ne prouve pas une panne de puce.",{"emmc_health","gpt","emmc_errors","cal0_hash","cal0_model","cal0_panel","fuses_pkg1"},{18,19,20,21,24},7,3,5,1},
 {"microSD","Initialisation et compteurs SD. Les reprises de lecture incluent SD et eMMC.","Comparer avec une carte connue fonctionnelle et examiner le lecteur. Les reprises communes ne désignent pas seules la microSD.",{"sd_errors"},{23,24},1,1,2,2},
 {"Alimentation / régulateurs","PMIC et rails mesurés, selon les puces présentes sur le modèle.","Contrôler tensions et liaisons I2C avant de conclure à une puce défectueuse.",{"soc_pmic_otp","pmic_rails","max77812","max77621_cpu","max77621_gpu"},{4,5,7},5,2,3,0},
 {"Contrôleur de charge","Identification, alimentation et défauts déclarés du chargeur.","Recouper chargeur, batterie et USB-C. Contrôler alimentation et bus commun avant remplacement.",{"charger_id","charger_pg","charger_fault"},{10},3,3,1,0},
 {"USB-C / Power Delivery","Réponse du contrôleur USB-PD. Le transfert USB complet n'est pas certifié.","Examiner connecteur, alimentation et liaison de contrôle USB-PD, puis recouper avec le chargeur.",{"usb_pd"},{11},1,1,1,0},
 {"Batterie / jauge","Santé déclarée, température et identification de la jauge.","Vérifier connecteur, sonde de température et mesures de la jauge avec la chaîne de charge.",{"batt_health","batt_ntc","fuel_devname"},{9},3,3,1,0},
 {"Wi-Fi","Énumération PCIe et lien avec le module. Le réseau complet n'est pas certifié.","Recouper PCIe, alimentation radio, horloges et service CPU0. Vérifier les prérequis avant de suspecter le module.",{"wifi_pcie"},{27,28},1,1,2,0},
 {"Bluetooth","Échange HCI avec la partie Bluetooth du module radio.","Contrôler alimentation radio, UART et liaisons communes Wi-Fi/Bluetooth.",{"bt_hci"},{26},1,1,1,0},
 {"Audio","Codec, horloges et essai sonore. L'écoute reste une observation manuelle.","Vérifier les rails et horloges du codec, puis les sorties et les connexions audio.",{"audio_codec","audio_clocks","audio_beep"},{29,30,31},3,3,3,0},
 {"Écran / rétroéclairage","Identification de dalle et contrôle du rétroéclairage. Les pixels restent à observer.","Comparer dalle, nappe et alimentation du rétroéclairage. Utiliser les mires pour l'observation des pixels.",{"dsi_id","backlight"},{32,33},2,2,2,0},
 {"Tactile","Identification et test électrique ITO. La grille manuelle est séparée.","Examiner nappe, alimentation et résultat ITO, puis la couverture manuelle de la grille.",{"touch_id","touch_ito"},{34},2,2,1,0},
 {"Commandes / Joy-Con","Inventaire matériel. Boutons, sticks et rails demandent le test interactif.","Compléter le test interactif : boutons, repos, amplitude et reconnexions. Vérifier les rails si des coupures sont observées.",{0},{36,37},0,0,2,0},
 {"Thermique / ventilation","Capteur thermique, températures et réponse du ventilateur.","Recouper températures, commande PWM, alimentation et réponse du ventilateur.",{"tmp451_id","soc_die_temp","pcb_temp","fan_stalled"},{12,13},4,4,2,0}
};

u32 nyx_component_count(void){return ARRAY_SIZE(components);}
const char *nyx_component_state(u32 state)
{
    static const char *names[]={"NON TESTÉ","PARTIEL","PASS — contrôles réussis","AVERTISSEMENT","ANOMALIE","ERREUR DE TEST"};
    return nyx_tr(names[state<ARRAY_SIZE(names)?state:0]);
}
static bool find(const char *key,nyx_diag_finding_t *out)
{
    for(u32 i=0;i<nyx_diag_finding_count();++i)if(nyx_diag_finding(i,out)&&!strcmp(out->key,key))return true;
    return false;
}
bool nyx_component_get(u32 index,nyx_component_result *out)
{
    if(!out||index>=nyx_component_count())return false;
    memset(out,0,sizeof(*out));const nyx_component_definition *d=&components[index];out->definition=d;
    bool error=false,probe_warning=false;u32 worst=0;bool measured=false;
    for(u32 i=0;i<d->probe_count;++i){
        const nyx_diag_probe_t *probe=nyx_diag_probe(d->probes[i]);
        if(probe&&probe->state!=NYX_DIAG_NOT_RUN)++out->executed;
        if(probe&&probe->state>=NYX_DIAG_ERROR)error=true;
        if(probe&&(probe->state==NYX_DIAG_WARN||probe->state==NYX_DIAG_FAIL)&&!(d->storage&&d->probes[i]==24))probe_warning=true;
    }
    for(u32 i=0;i<d->key_count;++i){
        nyx_diag_finding_t f;if(!find(d->keys[i],&f))continue;
        if(f.severity>2){error=true;continue;}
        ++out->found;if(i<d->required_count)++out->required_found;
        if(f.severity>worst||(f.severity==worst&&(!measured||f.detail[0]))){
            nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%s : %s",f.key,f.detail[0]?f.detail:nyx_component_state(f.severity==0?NYX_COMPONENT_PASS:f.severity==1?NYX_COMPONENT_WARN:NYX_COMPONENT_ANOMALY));measured=true;
        }
        if(f.severity>worst)worst=f.severity;
    }
    bool ready=!d->storage||nyx_diag_storage_ready(d->storage==1);
    out->partial=!d->required_count||out->required_found<d->required_count||out->executed<d->probe_count||!ready;
    /* WARN may also encode an unavailable prerequisite. Keep coverage
     * explicitly partial instead of inferring English from translated text. */
    if(worst==1||probe_warning)out->partial=true;
    out->state=error?NYX_COMPONENT_TEST_ERROR:worst>=2?NYX_COMPONENT_ANOMALY:worst==1||probe_warning?NYX_COMPONENT_WARN:
        out->partial?(out->executed||out->found?NYX_COMPONENT_PARTIAL:NYX_COMPONENT_NOT_TESTED):NYX_COMPONENT_PASS;
    if(!measured){
        if(!d->required_count)nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%s",nyx_tr("Contrôle interactif nécessaire."));
        else nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%d / %d résultats disponibles",out->required_found,d->required_count);
    }
    int manual=manual_kind(index);
    if(manual>=0&&manual_observations[manual][0]){
        if(out->state==NYX_COMPONENT_NOT_TESTED){out->state=NYX_COMPONENT_PARTIAL;out->partial=true;}
        if(manual_warnings[manual]&&out->state<NYX_COMPONENT_WARN){out->state=NYX_COMPONENT_WARN;out->partial=true;}
        if(!d->required_count)nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%s",nyx_tr("Mesures interactives enregistrées. Vérifier leur couverture dans le détail."));
    }
    return true;
}

#define SUMMARY_CAPACITY 32768u
static void add(char *buffer,const char *text)
{
    u32 used=strlen(buffer);if(used<SUMMARY_CAPACITY-1)nyx_diag_snprintf(buffer+used,SUMMARY_CAPACITY-used,"%s",text);
}
static void line(char *buffer,const char *text){add(buffer,nyx_tr(text));add(buffer,"\n");}
static void evidence(char *buffer,const nyx_component_result *result)
{
    const nyx_component_definition *d=result->definition;char row[512];
    nyx_diag_snprintf(row,sizeof(row),"%s — %s\n",nyx_tr(d->name),nyx_component_state(result->state));add(buffer,row);
    line(buffer,d->scope);line(buffer,result->measurement);
    if(result->partial)line(buffer,"Couverture partielle ou prérequis non vérifiés.");
    int kind=manual_kind(d-components);
    if(kind>=0&&manual_observations[kind][0]){
        line(buffer,"Dernier contrôle complémentaire de cette session, distinct des mesures automatiques :");
        add(buffer,manual_observations[kind]);add(buffer,"\n");
    }
}
char *nyx_component_summary_text(void)
{
    char *buffer=calloc(1,SUMMARY_CAPACITY);if(!buffer)return NULL;
    add(buffer,"\n\n");
    line(buffer,"Bilan par composant");
    line(buffer,"PASS concerne uniquement les contrôles réalisés. Il ne certifie pas toutes les fonctions d'une puce.");
    line(buffer,"Le bilan normal n'exécute pas les motifs du test RAM isolé.");
    if(nyx_diag_truncated())line(buffer,"Le rapport détaillé est tronqué. Les preuves disponibles et les contrôles non testés restent identifiés.");
    for(int state=NYX_COMPONENT_TEST_ERROR;state>=0;--state)for(u32 i=0;i<nyx_component_count();++i){
        nyx_component_result r;nyx_component_get(i,&r);if(r.state!=(u32)state)continue;evidence(buffer,&r);add(buffer,"\n");
    }
    line(buffer,"Pistes de panne");bool anomaly=false;
    nyx_component_result charge,pd,cpu,wifi;
    nyx_component_get(5,&charge);nyx_component_get(6,&pd);nyx_component_get(0,&cpu);nyx_component_get(8,&wifi);
    if(charge.state>=NYX_COMPONENT_ANOMALY&&pd.state>=NYX_COMPONENT_ANOMALY)
        line(buffer,"USB-PD et chargeur présentent des anomalies : vérifier leurs alimentations et liaisons de contrôle communes avant de conclure à deux puces défectueuses.");
    if(cpu.state>=NYX_COMPONENT_ANOMALY&&wifi.state>=NYX_COMPONENT_ANOMALY)
        line(buffer,"Le diagnostic Wi-Fi dépend du service CPU0. Son échec peut empêcher le contrôle radio ; le résultat Wi-Fi ne localise donc pas seul la panne.");
    for(u32 i=0;i<nyx_component_count();++i){
        nyx_component_result r;nyx_component_get(i,&r);if(r.state<NYX_COMPONENT_WARN)continue;
        anomaly=true;line(buffer,r.definition->name);line(buffer,r.measurement);line(buffer,r.definition->hint);add(buffer,"\n");
    }
    if(!anomaly)line(buffer,"Aucune anomalie structurée relevée. Compléter les contrôles partiels et les observations manuelles.");
    line(buffer,"Une absence de réponse peut venir de l'alimentation ou d'une liaison. Ces pistes ne prescrivent pas le remplacement d'une puce.");
    return buffer;
}
char *nyx_component_detail_text(u32 index)
{
    nyx_component_result r;if(!nyx_component_get(index,&r))return NULL;
    char *buffer=calloc(1,SUMMARY_CAPACITY);if(!buffer)return NULL;evidence(buffer,&r);
    line(buffer,"Preuves enregistrées");
    for(u32 i=0;i<r.definition->key_count;++i){
        char row[512];nyx_diag_finding_t f;
        if(find(r.definition->keys[i],&f))nyx_diag_snprintf(row,sizeof(row),"%s : %s — %s\n",f.key,nyx_component_state(f.severity==0?NYX_COMPONENT_PASS:f.severity==1?NYX_COMPONENT_WARN:NYX_COMPONENT_ANOMALY),f.detail);
        else nyx_diag_snprintf(row,sizeof(row),"%s : %s\n",r.definition->keys[i],nyx_component_state(NYX_COMPONENT_NOT_TESTED));
        add(buffer,row);
    }
    if(r.state>=NYX_COMPONENT_WARN){line(buffer,"Pistes de panne");line(buffer,r.definition->hint);}
    line(buffer,"Les rapports détaillés d'origine restent disponibles sans relancer les tests.");
    return buffer;
}
