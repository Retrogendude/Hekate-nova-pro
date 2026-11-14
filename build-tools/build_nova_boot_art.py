"""Create the original static boot wordmark; runtime animates only its short track."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json
work=Path(__file__).resolve().parent
root=work/'hekate-6.5.4'
bg=(16,21,29);cyan=(85,201,241);white=(242,245,250)
image=Image.new('RGB',(400,190),bg)
logo=Image.open(work/'nova-brand.png').convert('RGBA').resize((94,94),Image.Resampling.LANCZOS)
colored=Image.new('RGB',logo.size,cyan);image.paste(colored,(153,6),logo.getchannel('A'))
font=ImageFont.truetype(str(root/'nyx/nyx_gui/fonts/source/NotoSans-Regular.ttf'),32)
draw=ImageDraw.Draw(image);title='Hekate Nova Pro'
width=draw.textlength(title,font=font);draw.text(((400-width)/2,116),title,fill=white,font=font)
colors=[]
for color in (cyan,white):
    for a in (0,32,64,96,128,160,208,255):colors.append(tuple((bg[i]*(255-a)+color[i]*a)//255 for i in range(3)))
palette=Image.new('P',(1,1));palette.putpalette([c for rgb in colors for c in rgb]+[0]*(768-48))
indexed=image.quantize(palette=palette,dither=Image.Dither.NONE)
raw=bytes(indexed.getdata());runs=[];at=0
while at<len(raw):
    end=at+1
    while end<len(raw) and raw[end]==raw[at] and end-at<255:end+=1
    runs.extend((end-at,raw[at]));at=end
assert all(runs[i]>0 for i in range(0,len(runs),2))
assert len(runs)<6500,len(runs)
inc='/* Original Nova Pro mark + Noto Sans wordmark, GPL/OFL. Generated. */\n'
inc+='static const u32 nova_boot_palette[] = {'+','.join('0x%02X%02X%02X'%c for c in colors)+'};\n'
inc+='static const u8 nova_boot_rle[] = {'+','.join(map(str,runs))+'};\n'
out=root/'bdk/display/nova_boot_art.inc';out.write_text(inc)
image.save(work/'nova-boot-wordmark.png')
(work/'nova-boot-art.json').write_text(json.dumps({'width':400,'height':190,'rle_bytes':len(runs),'name':title,'font':'Noto Sans, OFL','original_brand':'Nova N monogram'},indent=2))
print(f'Boot art: {len(runs)} bytes; original logo and exact Hekate Nova Pro title.')
