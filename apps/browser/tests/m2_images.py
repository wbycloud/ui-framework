"""Generate deterministic WIC orientation/format samples, never network inputs."""
from pathlib import Path
from PIL import Image,ImageOps
import io,struct,sys

root=Path(sys.argv[1]);root.mkdir(parents=True,exist_ok=False)
image=Image.new('RGB',(12,8))
for y in range(8):
    for x in range(12):image.putpixel((x,y),((220 if x<6 else 30),(180 if y<4 else 40),(60 if x<6 else 200)))
for orientation in range(1,9):
    path=root/f'orientation-{orientation}.jpg';exif=Image.Exif();exif[274]=orientation
    image.save(path,quality=100,subsampling=0,exif=exif)
    decoded=ImageOps.exif_transpose(Image.open(path)).convert('RGBA')
    (root/f'orientation-{orientation}.rgba').write_bytes(struct.pack('<II',*decoded.size)+decoded.tobytes())
for kind in ('PNG','BMP','GIF'):
    out=io.BytesIO()
    if kind=='GIF':image.save(out,format=kind,save_all=True,append_images=[Image.new('RGB',image.size,(0,0,0))],duration=100,loop=0)
    else:image.save(out,format=kind)
    (root/f'format.{kind.lower()}').write_bytes(out.getvalue())
(root/'broken.png').write_bytes(b'broken-image')
print('M2 image fixtures ready: eight orientations, PNG/BMP/animated GIF and broken input')
