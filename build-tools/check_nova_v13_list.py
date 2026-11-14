"""Exhaustively verify the real LVGL list move against an array order model."""
from pathlib import Path
import subprocess,json
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';directory=work/'nova-v13-list';directory.mkdir(exist_ok=True)
program=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libs/lvgl/lv_misc/lv_ll.h>
void *lv_mem_alloc(uint32_t size){return malloc(size);}
void lv_mem_free(const void *ptr){free((void*)ptr);}
static unsigned cases;
static void scenario(int count,int *permutation){
 for(int from=0;from<count;++from)for(int before=0;before<=count;++before){
  lv_ll_t list;lv_ll_init(&list,sizeof(int));int *nodes[7],expected[7],actual[7],length=0;
  for(int i=0;i<count;++i){nodes[i]=lv_ll_ins_tail(&list);*nodes[i]=permutation[i];}
  int moved=permutation[from],next=before==count?-1:permutation[before];
  if(from==before){for(int i=0;i<count;++i)expected[length++]=permutation[i];}
  else{for(int i=0;i<count;++i){if(permutation[i]==next)expected[length++]=moved;if(i!=from)expected[length++]=permutation[i];}if(next==-1)expected[length++]=moved;}
  assert(length==count);lv_ll_move_before(&list,nodes[from],before==count?NULL:nodes[before]);
  void *previous=NULL;length=0;
  for(int *node=lv_ll_get_head(&list);node;node=lv_ll_get_next(&list,node)){assert(length<count);assert(lv_ll_get_prev(&list,node)==previous);assert(*node==expected[length]);actual[length++]=*node;previous=node;}
  assert(length==count&&previous==lv_ll_get_tail(&list));previous=NULL;
  for(int *node=lv_ll_get_tail(&list);node;node=lv_ll_get_prev(&list,node)){assert(length>0);assert(lv_ll_get_next(&list,node)==previous);assert(*node==actual[--length]);previous=node;}
  assert(!length&&previous==lv_ll_get_head(&list));lv_ll_clear(&list);++cases;
 }
}
static void permutations(int count,int *items,int start){if(start==count){scenario(count,items);return;}for(int i=start;i<count;++i){int temp=items[start];items[start]=items[i];items[i]=temp;permutations(count,items,start+1);temp=items[start];items[start]=items[i];items[i]=temp;}}
int main(void){for(int count=1;count<=7;++count){int items[]={0,1,2,3,4,5,6};permutations(count,items,0);}printf("PASS: %u actual-C linked-list permutations/moves; array oracle and both traversal directions, head, tail and reciprocal links verified.\n",cases);return 0;}
'''
p=directory/'check.c';p.write_text(program);exe=directory/'check.exe';cc=work/'toolchains/w64devkit/bin/gcc.exe'
subprocess.run([str(cc),'-std=c11','-O2','-Wall','-Wextra','-Werror','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(work/'lvgl-preview-v13'),'-I'+str(project/'bdk'),str(p),str(project/'bdk/libs/lvgl/lv_misc/lv_ll.c'),'-o',str(exe)],check=True)
summary=subprocess.check_output([str(exe)],text=True).strip();print(summary)
(work/'nova-v13-list.json').write_text(json.dumps({'summary':summary,'max_nodes':7,'host_optimization':'O2','reproduced_v12_failure':'With a wallpaper, the colors window head had a non-NULL previous pointer; the page was missing from forward input traversal.','fix':'lv_ll_move_before updates head when n_before is NULL.'},indent=2))
