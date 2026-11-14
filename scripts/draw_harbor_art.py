#!/usr/bin/env python3
"""Draw the original Moonlit Harbor art on a 2-world-pixel grid. No external assets."""
from math import sqrt
from pathlib import Path
import random

OUT = Path(__file__).resolve().parents[1] / 'godot-lab/assets'
RNG = random.Random(473)


class Sprite:
    def __init__(self, w, h):
        self.w, self.h, self.shapes = w, h, []

    def rect(self, x, y, w, h, color):
        if w > 0 and h > 0:
            self.shape(color, f'M{x} {y}h{w}v{h}h{-w}z')

    def shape(self, color, d):
        if self.shapes and self.shapes[-1][0] == color:
            self.shapes[-1][1].append(d)
        else:
            self.shapes.append((color, [d]))

    def disk(self, x, y, rad, color, y_scale=1):
        # Scanlines give the moon a clean pixel silhouette when viewed at 2x.
        for row in range(-int(rad*y_scale), int(rad*y_scale)+1):
            w = int(sqrt(max(0, rad**2-(row/y_scale)**2)))
            self.rect(x-w, y+row, 2*w+1, 1, color)

    def write(self, name):
        p = OUT / name
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.w}" height="{self.h}" viewBox="0 0 {self.w} {self.h}" shape-rendering="crispEdges">\n' + '\n'.join(f'<path fill="{c}" d="{" ".join(parts)}"/>' for c, parts in self.shapes) + '\n</svg>\n')


def ridge(s, points, bottom, color):
    d = f'M{points[0][0]} {points[0][1]}'
    for x, y in points[1:]:
        d += f'H{x}V{y}'
    s.shape(color, d+f'V{bottom}H{points[0][0]}z')


def sky():
    s = Sprite(1840, 760)
    bands = [(0, '#0b1328'), (230, '#121a34'), (306, '#1a2240'), (359, '#212747'),
             (398, '#292c4e'), (433, '#313251'), (467, '#3c3959'), (496, '#474060'),
             (524, '#554865'), (552, '#65536e'), (585, '#745c74'), (627, '#816478')]
    for i, (top, c) in enumerate(bands):
        end = bands[i+1][0] if i+1 < len(bands) else s.h
        s.rect(0, top, s.w, end-top, c)
        if i > 0:
            for row, spacing in ((0, 6), (1, 12)):
                start = i % 3
                count = len(range(start, s.w, spacing))
                s.shape(c, f'M{start} {top-1-row}h1v1h-1z' + (f'm{spacing} 0h1v1h-1z' * (count-1)))
    for _ in range(119):
        x, y = RNG.randrange(s.w), RNG.randrange(315, 498)
        if 675 < x < 765 and 390 < y < 475:
            continue
        c = RNG.choice(['#505377', '#596180', '#71708a', '#8791a1'])
        s.rect(x, y, 1+int(RNG.random()<0.12), 1, c)
    for x,y in [(583,367),(673,396),(848,359),(1038,404),(1290,390),(428,365),(1521,419)]:
        s.rect(x,y-1,1,3,'#8d97ac')
        s.rect(x-1,y,3,1,'#8d97ac')
        s.rect(x,y,1,1,'#c4c5c0')
    s.disk(738,431,44,'#4c5069')
    s.disk(738,431,39,'#64657a')
    s.disk(738,431,34,'#a7a2a2')
    s.disk(738,431,32,'#e6d9ba')
    s.disk(735,428,28,'#eedfc1')
    for x,y,r in [(728,423,5),(750,443,6),(726,444,3),(745,418,4),(745,437,3),(731,434,2),(756,428,3)]:
        s.disk(x,y,r,'#cec3af',0.65)
        s.rect(x-r+1,y-1,r,1,'#dfd0b2')
    s.rect(725,410,12,1,'#fff0d0')
    s.write('harbor/sky.svg')


def clouds():
    s = Sprite(1780, 430)
    for x,y,w,c in [(75,323,206,'#55506d'),(324,302,164,'#41445f'),(600,310,139,'#51506d'),
                    (784,339,198,'#6d586f'),(1030,270,179,'#464662'),(1288,322,183,'#63526d'),
                    (1478,362,282,'#6b586d'),(101,389,272,'#746075')]:
        s.shape(c,f'M{x} {y+15}h20v-4h24v-3h19v-5h31v-3h34v3h19v3h15v3h22v4h{max(w-184,8)}v3H{x}z')
        s.rect(x+45,y+17,w-50,2,c)
        s.rect(x+18,y+21,w//2,1,c)
        s.rect(x+w//2,y+25,w//3,1,c)
    s.write('harbor/clouds.svg')


def hills():
    s = Sprite(1600, 330)
    for j,(y,c,hi) in enumerate([(145,'#5d516c','#6b5b74'),(175,'#4b455f','#544c67'),(204,'#3b3d56','#454359')]):
        pts = [(0,y)]
        while pts[-1][0] < s.w:
            x,z = pts[-1]
            step = RNG.randrange(5,15)
            v = max(y-64+j*10,min(y+16,z+RNG.choice([-9,-6,-5,-2,1,4,6,9])))
            pts.append((x+step,v))
        ridge(s,pts,s.h,c)
        for i,(x,z) in enumerate(pts[:-1]):
            if (i+j)%4 == 0:
                s.rect(x,z,pts[i+1][0]-x,1,hi)
    # Low arches and two narrow watchtowers on the far shoreline.
    for x, y, n in [(107,193,5),(530,186,6),(990,194,7),(1305,181,5)]:
        s.rect(x,y,15*n,13,'#35354e')
        s.rect(x,y-4,15*n,3,'#474158')
        for i in range(n):
            s.shape('#49445b',f'M{x+4+i*15} {y+13}v-7h2v-3h5v3h2v7z')
    for x,y in [(152,174),(536,161),(1085,171),(1344,149)]:
        s.rect(x,y,11,46,'#37384f')
        s.shape('#35364e',f'M{x-3} {y}h3v-4h2v-4h2v-4h3v-5h2v5h2v4h2v4h2v4h3v3h-21z')
        s.rect(x+4,y+8,2,4,'#776175')
    s.write('harbor/hills.svg')


def roof(s,x,y,w,h,wall,shaded,roof_color,darker,kind, lit=False):
    s.rect(x,y,w,h,wall)
    s.rect(x+w-6,y,6,h,shaded)
    s.rect(x+1,y+1,2,h-2,'#35384d')
    s.rect(x,y+h-3,w,3,shaded)
    top = y-14 if kind==0 else y-21
    if kind==2:
        s.rect(x-4, y-8, w+8,9,roof_color)
        for j in range(x-3,x+w+3,8):
            s.rect(j,y-11,4,4,roof_color)
    else:
        # The shingles step up towards the ridge, staying on the pixel grid.
        coords = [(x-5,y)]
        for k in range(1,8):
            coords.append((x-5+round((w+10)*k/16),y+round((top-y)*k/8)))
        for k in range(8,17):
            coords.append((x-5+round((w+10)*k/16),top+round((y-top)*(k-8)/8)))
        ridge(s,coords,y+2,roof_color)
        s.rect(x-5,y,w+10,2,darker)
        chimney=x+(w//2 if kind==0 else w//4)
        s.rect(chimney,top-5,5,y-top-3,wall)
        s.rect(chimney-1,top-7,7,2,roof_color)
        s.rect(x+w//4,y-4,w//2,1,darker)
    for wr in range(y+8,y+h-9,13):
        for wx in range(x+7,x+w-7,12):
            c = '#202637'
            if (lit and RNG.random()<0.58) or RNG.random()<0.18:
                c = RNG.choice(['#925f43','#a37450','#bd8a58','#785549'])
            s.rect(wx,wr-1,6,7,'#212639')
            s.rect(wx+1,wr,4,5,c)
            s.rect(wx+1,wr+2,4,1,wall)
            s.rect(wx+2,wr+5,4,1,'#4d4050')
    s.rect(x+w//2-3,y+h-10,7,10,shaded)
    s.rect(x+w//2-2,y+h-12,5,2,shaded)


def town():
    s = Sprite(1600, 430)
    s.rect(0,287,s.w,143,'#293149')
    for y in range(291,428,3):
        d=[]
        for _ in range(26):
            x=RNG.randrange(s.w);w=RNG.randrange(4,29)
            d.append(f'M{x} {y+RNG.randrange(2)}h{w}v1h{-w}z')
        s.shape(RNG.choice(['#394052','#333b50','#3b4258','#35364a']), ' '.join(d))
    for i,(lo,hi,bottom) in enumerate([(12,238,277),(312,581,293),(633,782,279),(867,1150,287),(1240,1452,288),(1480,1595,276)]):
        x=lo
        while x<hi:
            w=RNG.randint(22,46);h=RNG.randint(25,64)
            roof(s,x,bottom-h,w,h,'#35374d','#282d41','#41405a','#292d41',RNG.randrange(3))
            x+=w-2
        s.rect(lo-6,bottom,hi-lo+12,4,'#3d4155')
        s.rect(lo-6,bottom+4,hi-lo+12,5,'#21293d')
        x=lo+11
        while x<hi:
            w=RNG.randint(23,43);h=RNG.randint(25,61)
            y=bottom+9-h
            roof(s,x,y,w,h,'#3e3b50','#2b2c42','#54506a','#30324a',RNG.randrange(3),i in [0,1,3,4])
            x+=w-2
        for x in range(lo+5,hi,26):
            s.rect(x,bottom+8,3,11,'#1e293a')
    # Harbor lighthouse: stone taper, lantern room and iron gallery.
    x=378
    s.shape('#5d5a70',f'M{x-15} 259h15v-13h4v-24h3v-37h3v-39h24v39h3v37h3v24h4v13h15v10h-74z')
    s.shape('#45475e',f'M{x+22} 146h12v39h3v37h3v24h4v13h15v10h-42v-19h2v-45h3z')
    for y,w in [(163,22),(186,25),(209,27),(234,30),(253,36)]:
        s.rect(x+6,y,w,2,'#71657a')
        s.rect(x+20,y-12,4,7,'#272a40')
        s.rect(x+20,y-12,2,5,'#98775b')
    s.rect(x+3,137,35,9,'#3a3b52')
    s.rect(x,132,41,4,'#7c7380')
    s.rect(x+8,115,25,17,'#6f5260')
    s.rect(x+11,117,19,13,'#b88359')
    s.rect(x+14,119,13,9,'#e1ad72')
    s.rect(x+18,120,5,7,'#f2ca8c')
    for k in [9,20,31]:
        s.rect(x+k,115,2,17,'#594c62')
    s.shape('#53536d',f'M{x+5} 115v-3h3v-3h3v-3h3v-3h3v-4h6v4h3v3h3v3h3v3h3v3z')
    s.rect(x+19,92,2,8,'#8f7987')
    s.rect(x+3,133,34,2,'#ad8b7f')
    # Distant quays, mast silhouettes and muted window reflections.
    for x,y,w in [(251,320,49),(798,318,52),(1185,314,43),(1443,314,47),(80,345,60)]:
        s.shape('#222d42',f'M{x} {y}h{w}l-8 6h{-w+17}z')
        s.rect(x+w//2,y-36,1,36,'#414054')
        s.shape('#484459',f'M{x+w//2+2} {y-31}l17 25h-17z')
        s.rect(x+4,y+8,w-8,1,'#414354')
    for x,y in [(124,291),(197,303),(398,283),(428,301),(484,308),(930,294),(1027,310),(1356,312)]:
        for j in range(16):
            dx=RNG.randint(-8,9)
            s.rect(x+dx,y+j*3,RNG.randint(1,9),1,RNG.choice(['#6e5355','#79605c','#56505c','#634d55']))
    s.write('harbor/town.svg')


def tiles():
    s=Sprite(128,16)
    for i in range(8):
        x=i*16
        c=['#42435f','#47455f','#403f59','#4b475f'][i%4]
        s.rect(x,0,16,16,'#262a42')
        s.rect(x+1,1,15,6,c)
        offset=(4+i*3)%11
        s.rect(x+1,1,offset,5,'#4e4a68')
        s.rect(x+offset+1,1,1,5,'#30324a')
        s.rect(x+1,8,15,7,c)
        s.rect(x+2,8,13,1,'#53516f')
        offset=4+(i*5)%8
        s.rect(x+offset,9,1,6,'#30324a')
        s.rect(x+2+i%4,11,3,1,'#5e5775')
        s.rect(x+9-i%3,4,3,1,'#605873')
        s.rect(x+11,12,3,2,'#35344e')
        if i>=4:
            s.rect(x,0,16,3,'#c4b9c6')
            s.rect(x,3,16,2,'#827895')
            s.rect(x+2,0,9,1,'#efe0d2')
            s.rect(x+15,1,1,2,'#9e90aa')
            s.rect(x+1,5,3,1,'#6c6482')
    # The atlas spans 8 original 32px collision tiles; the artwork is 16px per cell.
    s.w,s.h=128,16
    s.write('harbor/stone_16.svg')
    src=(OUT/'harbor/stone_16.svg').read_text()
    (OUT/'tiles.svg').write_text(src.replace('width="128" height="16"','width="256" height="32"'))
    (OUT/'harbor/stone_16.svg').unlink()


def props():
    s=Sprite(80,32)
    # Lamp post, warm glass and iron cap.
    s.rect(7,4,2,26,'#55526d')
    s.rect(8,15,1,15,'#25283d')
    s.rect(4,29,8,2,'#6b6680')
    s.rect(2,7,12,2,'#665467')
    s.rect(4,9,8,9,'#29263a')
    s.rect(5,10,6,6,'#95563d')
    s.rect(6,10,4,5,'#d99464')
    s.rect(7,11,2,3,'#ecc18a')
    s.rect(3,17,10,2,'#5d4b60')
    s.shape('#6d5c70','M3 7v-1h2v-2h2v-2h3v2h2v2h2v1z')
    # Crate and rope coils.
    s.rect(16,17,15,14,'#42374c')
    s.rect(17,18,13,12,'#635061')
    s.rect(17,18,13,2,'#806770')
    s.rect(18,21,11,7,'#543e50')
    s.shape('#866573','M18 27h2v-2h2v-2h2v-2h2v-2h2v2h-2v2h-2v2h-2v2h-2v2h-2z')
    s.rect(17,28,13,2,'#79606d')
    s.shape('#baa183','M32 29v-4h2v-2h10v2h2v4h-2v2H34v-2zM35 26v2h8v-2z')
    s.rect(36,29,6,1,'#8b776d')
    # Painted practice target (foreground), bounded by a 16x24 hit plate.
    s.rect(55,18,2,14,'#53475d')
    s.rect(49,4,14,22,'#443048')
    s.rect(50,4,12,1,'#a8727b')
    s.rect(49,8,1,14,'#8e6879')
    s.rect(52,8,8,14,'#884c60')
    s.shape('#bc7481','M53 10h6v2h2v6h-2v2h-6v-2h-2v-6h2z')
    s.rect(54,13,4,4,'#d9b397')
    s.rect(54,23,4,1,'#935e73')
    # Mooring post.
    s.rect(68,13,7,18,'#403449')
    s.rect(69,14,2,16,'#8b6977')
    s.rect(67,14,9,1,'#ad8389')
    s.rect(66,19,12,3,'#a4877b')
    s.rect(67,20,11,1,'#706179')
    s.write('harbor/props.svg')


if __name__ == '__main__':
    sky()
    clouds()
    hills()
    town()
    tiles()
    props()
