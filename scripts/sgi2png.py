import struct, zlib, sys
def read_sgi(path):
    d=open(path,'rb').read()
    magic,storage,bpc,dim,xs,ys,zs=struct.unpack('>hbbHHHH',d[:12])
    assert magic==474 and bpc==1
    ch=[[None]*ys for _ in range(zs)]
    if storage==0:
        off=512
        for z in range(zs):
            for y in range(ys):
                ch[z][y]=d[off:off+xs]; off+=xs
    else:
        n=ys*zs
        starts=struct.unpack('>%dI'%n,d[512:512+4*n]); 
        for z in range(zs):
            for y in range(ys):
                p=starts[y+z*ys]; out=bytearray()
                while True:
                    c=d[p]; p+=1; cnt=c&0x7f
                    if not cnt: break
                    if c&0x80: out+=d[p:p+cnt]; p+=cnt
                    else: out+=bytes([d[p]])*cnt; p+=1
                ch[z][y]=bytes(out)
    rows=[]
    for y in range(ys-1,-1,-1):  # SGI is bottom-up
        r=bytearray()
        for x in range(xs):
            px=[ch[z][y][x] for z in range(zs)]
            if zs==1: px=px*3+[255]
            elif zs==3: px=px+[255]
            r+=bytes(px)
        rows.append(r)
    return xs,ys,rows
def write_png(path,w,h,rows):
    raw=b''.join(b'\0'+bytes(r) for r in rows)
    def chunk(t,b): return struct.pack('>I',len(b))+t+b+struct.pack('>I',zlib.crc32(t+b)&0xffffffff)
    open(path,'wb').write(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(raw,9))+chunk(b'IEND',b''))
if __name__=='__main__':
    tex=sys.argv[1]; out=sys.argv[2]
    q={k:read_sgi(f'{tex}/splash_{k}.rgb') for k in ('tl','tr','bl','br')}
    w=q['tl'][0]; h=q['tl'][1]
    print('quadrant',w,h,[ (k,v[0],v[1]) for k,v in q.items()])
    rows=[q['tl'][2][y]+q['tr'][2][y] for y in range(h)]+[q['bl'][2][y]+q['br'][2][y] for y in range(q['bl'][1])]
    write_png(out,w+q['tr'][0],len(rows),rows)
