from PIL import Image
import math
import sys

#読み込みファイル
if len(sys.argv) > 1:
    filename = sys.argv[1]
else:
    fileName = 'test.png'

#分割数
if len(sys.argv) > 2:
    divCount = sys.argv[2]
else:
    divCount = 120

sourceImage = Image.open(filename)

numPixels = 10 #ピクセル数
angleStep = (int)(360 / divCount)

image = sourceImage.resize((numPixels * 2 , numPixels * 2)) #ピクセル数 x 2 にリサイズ

h = image.height
w = image.width
hCenter = (int)(h / 2)
wCenter = (int)(w / 2)

print("#define NUMPIXELS " + str(numPixels))
print("#define DIVCOUNT " + str(divCount))

print("const uint32_t picDate[DIVCOUNT][NUMPIXELS] = {")
for angleCount in range(0,divCount):
    #print("angle=" + str(angleCount * angleStep), end="")
    print("\t{", end = "")
    for pixelCount in range(0, numPixels):
        x = hCenter + (int)(pixelCount * math.cos(angleCount * angleStep/(180/math.pi))) 
        y = wCenter + (int)(pixelCount * math.sin(angleCount * angleStep/(180/math.pi))) 
        if x < w and y < h:
            r, g, b = image.getpixel((x,y))
        else:
            r = 0
            g = 0
            b = 0
        print("0x%02X%02X%02X" % (r,g,b),end="")
        if pixelCount != numPixels - 1:
            print(",", end = "")
        #print(":x=" + str(x), end="")
        #print(":y=" + str(y))
    print("},")
print("};")