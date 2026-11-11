import sys, struct, zlib, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sgi2png import read_sgi, write_png
tex, outdir = sys.argv[1], sys.argv[2]
q={k:read_sgi(f'{tex}/splash_{k}.rgb') for k in ('tl','tr','bl','br')}
rows=[q['tl'][2][y]+q['tr'][2][y] for y in range(256)]+[q['bl'][2][y]+q['br'][2][y] for y in range(256)]
def px(x,y): r=rows[y]; i=4*x; return r[i:i+4]
# crop logo region
X0,Y0,X1,Y1=8,72,504,336
cw,ch=X1-X0,Y1-Y0
def render(N):
    out=[]
    m=N*0.06; rad=N*0.2
    sw=N-2*m; scale=(N*0.84)/cw; lw=cw*scale; lh=ch*scale
    ox=(N-lw)/2; oy=(N-lh)/2
    for y in range(N):
        r=bytearray()
        for x in range(N):
            # rounded square mask
            cx=min(max(x+0.5,m+rad),N-m-rad); cy=min(max(y+0.5,m+rad),N-m-rad)
            d=((x+0.5-cx)**2+(y+0.5-cy)**2)**0.5
            a=max(0.0,min(1.0,rad-d+0.5))
            t=y/N
            bg=[int(150-50*t),int(185-55*t),int(245-25*t)]
            # logo sample (bilinear)
            lx=(x+0.5-ox)/scale+X0-0.5; ly=(y+0.5-oy)/scale+Y0-0.5
            if X0<=lx<X1-1 and Y0<=ly<Y1-1:
                ix,iy=int(lx),int(ly); fx,fy=lx-ix,ly-iy
                p=[0,0,0,0]
                for (dx,dy,w) in ((0,0,(1-fx)*(1-fy)),(1,0,fx*(1-fy)),(0,1,(1-fx)*fy),(1,1,fx*fy)):
                    s=px(ix+dx,iy+dy)
                    for k in range(4): p[k]+=s[k]*w
                la=p[3]/255
                bg=[int(bg[k]*(1-la)+p[k]*la) for k in range(3)]
            r+=bytes(bg+[int(255*a)])
        out.append(r)
    return out
os.makedirs(f'{outdir}/tuxracer.iconset',exist_ok=True)
pngs={}
for n in (16,32,64,128,256,512,1024):
    p=f'{outdir}/icon_{n}.png'; write_png(p,n,n,render(n)); pngs[n]=p
for n,name in ((16,'16x16'),(32,'16x16@2x'),(32,'32x32'),(64,'32x32@2x'),(128,'128x128'),(256,'128x128@2x'),(256,'256x256'),(512,'256x256@2x'),(512,'512x512'),(1024,'512x512@2x')):
    open(f'{outdir}/tuxracer.iconset/icon_{name}.png','wb').write(open(pngs[n],'rb').read())
# Windows .ico with embedded PNGs
sizes=[16,32,64,128,256]; data=[open(pngs[n],'rb').read() for n in sizes]
hdr=struct.pack('<HHH',0,1,len(sizes)); off=6+16*len(sizes); ent=b''
for n,b in zip(sizes,data):
    ent+=struct.pack('<BBBBHHII',n%256,n%256,0,0,1,32,len(b),off); off+=len(b)
open(f'{outdir}/tuxracer.ico','wb').write(hdr+ent+b''.join(data))
print('ok')
