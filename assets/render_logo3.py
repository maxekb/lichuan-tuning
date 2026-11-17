#!/usr/bin/env python3
"""Ten Servo_Tuning logo variants (no brand tie-ins). Usage: python3 -I render_logo3.py [outdir]"""
import math
import sys
from PIL import Image, ImageDraw, ImageFont

S = 4
SIZE = 512 * S
BLUE = (0x3D, 0xA8, 0xFF)
TEAL = (0x34, 0xD3, 0x99)
YEL = (0xE8, 0xB9, 0x3C)
GRAY = (0x7C, 0x8A, 0xA0)
GRID = (0x33, 0x44, 0x5A)
DARK = (30, 42, 58, 255)
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

def px(v): return int(v * S)

def base():
    img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    return img, ImageDraw.Draw(img)

def font(sz): return ImageFont.truetype(FONT, px(sz))

def lerp(a, b, t): return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))

def cubic(p0, p1, p2, p3, n=60):
    out = []
    for i in range(n + 1):
        t = i / n; u = 1 - t
        out.append((u**3*p0[0] + 3*u*u*t*p1[0] + 3*u*t*t*p2[0] + t**3*p3[0],
                    u**3*p0[1] + 3*u*u*t*p1[1] + 3*u*t*t*p2[1] + t**3*p3[1]))
    return out

def stroke_pts(d, pts, w, col_fn=None):
    """Draw a polyline; col_fn(t)->color for gradient, else constant tuple."""
    n = max(len(pts) - 1, 1)
    for i in range(len(pts) - 1):
        col = col_fn(i / n) if col_fn else col_fn_const[0]
        a, b = pts[i], pts[i + 1]
        d.line([px(a[0]), px(a[1]), px(b[0]), px(b[1])], fill=col, width=px(w))
        d.ellipse([px(b[0]) - px(w)//2, px(b[1]) - px(w)//2,
                   px(b[0]) + px(w)//2, px(b[1]) + px(w)//2], fill=col)

col_fn_const = [BLUE]

def grad(t): return lerp(BLUE, TEAL, t)

# 1. "ST" monogram in encoder ring
def v01():
    img, d = base()
    d.ellipse([px(46), px(46), px(466), px(466)], outline=BLUE, width=px(16))
    for k in range(24):
        a = math.radians(k * 15)
        d.line([px(256 + 186*math.cos(a)), px(256 + 186*math.sin(a)),
                px(256 + 198*math.cos(a)), px(256 + 198*math.sin(a))], fill=GRAY, width=px(4))
    ia = math.radians(-45)
    ix, iy = 256 + 212*math.cos(ia), 256 + 212*math.sin(ia)
    d.ellipse([px(ix-12), px(iy-12), px(ix+12), px(iy+12)], fill=YEL)
    f = font(170)
    d.text((px(194), px(256)), "S", font=f, fill=BLUE, anchor="mm")
    d.text((px(318), px(256)), "T", font=f, fill=TEAL, anchor="mm")
    return img

# 2. Gear with shaft key
def v02():
    img, d = base()
    pts = []
    for k in range(12):
        a0 = k * 30.0
        for a, r in ((a0-11, 196), (a0+11, 196), (a0+19-11, 150), (a0+19+11, 150)):
            ar = math.radians(a)
            pts.append((px(256 + r*math.cos(ar)), px(256 + r*math.sin(ar))))
    d.polygon(pts, fill=TEAL)
    d.ellipse([px(152), px(152), px(360), px(360)], fill=DARK)
    d.ellipse([px(206), px(206), px(306), px(306)], outline=BLUE, width=px(14))
    d.rectangle([px(244), px(176), px(268), px(216)], fill=BLUE)
    for k in range(6):
        a = math.radians(k * 60 + 30)
        bx, by = 256 + 122*math.cos(a), 256 + 122*math.sin(a)
        d.ellipse([px(bx-9), px(by-9), px(bx+9), px(by+9)], fill=GRAY)
    return img

# 3. Tachometer gauge
def v03():
    img, d = base()
    box = [px(76), px(76), px(436), px(436)]
    d.arc(box, 150, 420, fill=GRID, width=px(14))
    for k in range(11):
        a = math.radians(150 + k * 27)
        x0, y0 = 256 + 162*math.cos(a), 256 + 162*math.sin(a)
        x1, y1 = 256 + 180*math.cos(a), 256 + 180*math.sin(a)
        d.line([px(x0), px(y0), px(x1), px(y1)], fill=GRAY, width=px(6))
    d.arc([px(96), px(96), px(416), px(416)], 150, 312, fill=TEAL, width=px(12))
    na = math.radians(312)
    d.line([px(256), px(256), px(256 + 130*math.cos(na)), px(256 + 130*math.sin(na))],
           fill=BLUE, width=px(14))
    d.ellipse([px(236), px(236), px(276), px(276)], fill=BLUE)
    f = font(46)
    d.text((px(256), px(362)), "RPM", font=f, fill=GRAY, anchor="mm")
    return img

# 4. Fieldbus star around motor hub
def v04():
    img, d = base()
    nodes = [(256 + 170*math.cos(math.radians(a)), 256 + 170*math.sin(math.radians(a)))
             for a in (-90, 30, 150)]
    for nx, ny in nodes:
        d.line([px(256), px(256), px(nx), px(ny)], fill=GRAY, width=px(8))
    d.ellipse([px(196), px(196), px(316), px(316)], fill=DARK, outline=TEAL, width=px(12))
    for k in range(12):
        a = math.radians(k * 30)
        dx, dy = 256 + 46*math.cos(a), 256 + 46*math.sin(a)
        d.ellipse([px(dx-4), px(dy-4), px(dx+4), px(dy+4)], fill=TEAL)
    for i, (nx, ny) in enumerate(nodes):
        c = BLUE if i else YEL
        d.ellipse([px(nx-30), px(ny-30), px(nx+30), px(ny+30)], fill=c)
    return img

# 5. App-icon: smooth S-curve on rounded square
def v05():
    img, d = base()
    d.rounded_rectangle([px(28), px(28), px(484), px(484)], radius=px(96), fill=DARK)
    pts = cubic((112, 384), (216, 384), (196, 128), (300, 128)) + \
          cubic((300, 128), (352, 108), (392, 122), (404, 150), 30)
    col_fn_const[0] = BLUE
    stroke_pts(d, pts, 30, grad)
    d.ellipse([px(392), px(138), px(420), px(166)], fill=TEAL)
    return img

# 6. Encoder ring + tuned step response (the original, refined)
def v06():
    img, d = base()
    for k in range(24):
        a = math.radians(k * 15 - 7.5)
        d.line([px(256 + 204*math.cos(a)), px(256 + 204*math.sin(a)),
                px(256 + 220*math.cos(a)), px(256 + 220*math.sin(a))], fill=GRID, width=px(7))
    ia = math.radians(-45)
    d.ellipse([px(256 + 212*math.cos(ia) - 11), px(256 + 212*math.sin(ia) - 11),
               px(256 + 212*math.cos(ia) + 11), px(256 + 212*math.sin(ia) + 11)], fill=YEL)
    x = 86
    while x < 420:
        d.line([px(x), px(152), px(min(x+20, 428)), px(152)], fill=GRAY, width=px(5))
        x += 36
    pts = cubic((92, 380), (158, 380), (168, 330), (208, 240)) + \
          cubic((208, 240), (238, 172), (262, 132), (296, 139)) + \
          cubic((296, 139), (322, 145), (332, 152), (420, 152), 40)
    stroke_pts(d, pts, 24, grad)
    d.ellipse([px(406), px(138), px(434), px(166)], fill=TEAL)
    return img

# 7. Servo motor silhouette
def v07():
    img, d = base()
    d.rounded_rectangle([px(128), px(196), px(330), px(316)], radius=px(24),
                        outline=BLUE, width=px(12))
    for k in range(1, 4):
        x = 128 + k * 50
        d.line([px(x), px(216), px(x), px(296)], fill=BLUE, width=px(6))
    d.rounded_rectangle([px(330), px(172), px(362), px(340)], radius=px(8),
                        outline=BLUE, width=px(10))
    d.line([px(362), px(256), px(452), px(256)], fill=TEAL, width=px(16))
    d.ellipse([px(444), px(248), px(460), px(264)], fill=TEAL)
    d.rounded_rectangle([px(176), px(140), px(244), px(196)], radius=px(8),
                        outline=TEAL, width=px(8))
    d.line([px(160), px(316), px(160), px(360)], fill=GRAY, width=px(8))
    d.line([px(298), px(316), px(298), px(360)], fill=GRAY, width=px(8))
    d.line([px(140), px(360), px(318), px(360)], fill=GRAY, width=px(8))
    return img

# 8. Step response with overshoot and arrows (axes)
def v08():
    img, d = base()
    d.line([px(96), px(96), px(96), px(416)], fill=GRAY, width=px(6))
    d.line([px(96), px(416), px(440), px(416)], fill=GRAY, width=px(6))
    x = 96
    while x < 424:
        d.line([px(x), px(170), px(min(x+16, 424)), px(170)], fill=(0x55, 0x60, 0x70), width=px(5))
        x += 32
    pts = cubic((96, 400), (150, 400), (172, 120), (214, 116)) + \
          cubic((214, 116), (244, 112), (252, 210), (284, 206)) + \
          cubic((284, 206), (308, 202), (316, 168), (340, 170)) + \
          cubic((340, 170), (368, 172), (376, 170), (424, 170), 30)
    stroke_pts(d, pts, 18, grad)
    d.polygon([px(424), px(158), px(452), px(170), px(424), px(182)], fill=TEAL)
    return img

# 9. Oscilloscope screen
def v09():
    img, d = base()
    d.rounded_rectangle([px(56), px(86), px(456), px(426)], radius=px(32),
                        outline=BLUE, width=px(10))
    for k in range(1, 3):
        d.line([px(56 + k*100), px(96), px(56 + k*100), px(416)], fill=GRID, width=px(4))
    for k in range(1, 3):
        d.line([px(66), px(86 + k*113), px(446), px(86 + k*113)], fill=GRID, width=px(4))
    sine = [(92 + t*330, 236 - 62*math.sin(t*math.pi*4)) for t in [i/120 for i in range(121)]]
    col_fn_const[0] = BLUE
    stroke_pts(d, sine, 12, lambda t: BLUE)
    sq = []
    for t in [i/160 for i in range(161)]:
        v = 330 if (math.sin(t*math.pi*4 + 0.6) >= 0) else 270
        sq.append((92 + t*330, v))
    stroke_pts(d, sq, 9, lambda t: TEAL)
    d.ellipse([px(414), px(168), px(434), px(188)], fill=BLUE)
    return img

# 10. Ring spanner over a sine wave
def v10():
    img, d = base()
    wave = [(72 + t*368, 268 - 84*math.sin(t*math.pi*2)) for t in [i/120 for i in range(121)]]
    col_fn_const[0] = TEAL
    stroke_pts(d, wave, 22, lambda t: TEAL)
    cx, cy, r = 176, 344, 56
    d.ellipse([px(cx-r), px(cy-r), px(cx+r), px(cy+r)], outline=BLUE, width=px(22))
    d.line([px(cx+38), px(cy+38), px(cx+150), px(cy+150)], fill=BLUE, width=px(30))
    ia = math.radians(-140)
    d.ellipse([px(cx + r*math.cos(ia) - 8), px(cy + r*math.sin(ia) - 8),
               px(cx + r*math.cos(ia) + 8), px(cy + r*math.sin(ia) + 8)], fill=YEL)
    return img

def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "."
    fns = [v01, v02, v03, v04, v05, v06, v07, v08, v09, v10]
    cols, cw, ch = 5, 312, 336
    sheet = Image.new("RGB", (cols * cw + 20, 2 * ch + 20), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    fr = ImageFont.truetype(FONT, 30)
    for i, fn in enumerate(fns):
        img = fn().resize((512, 512), Image.LANCZOS)
        img.save(f"{out}/logo_{i+1:02d}.png")
        cell = Image.new("RGBA", (512, 512), (40, 40, 40, 255))
        cell.alpha_composite(img)
        x, y = 20 + (i % cols) * cw, 20 + (i // cols) * ch
        sheet.paste(cell.resize((cw - 12, cw - 12)), (x + 6, y))
        d.text((x + 8, y + cw - 4), str(i + 1), font=fr, fill=(255, 255, 255))
    sheet.save(f"{out}/logo_variants.png")

if __name__ == "__main__":
    main()
