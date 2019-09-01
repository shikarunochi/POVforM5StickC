from PIL import Image
import math
import sys

if len(sys.argv) > 1:
    filename = sys.argv[1]
else:
    fileName = 'test.png'

image = Image.open(filename)

w,h = image.size

divCount = w #分割数
numPixels = 10 #ピクセル数

print("#define NUMPIXELS " + str(numPixels))
print("#define DIVCOUNT " + str(divCount))

print("const uint32_t picDate[DIVCOUNT][NUMPIXELS] = {")
for lineCount in range(0,divCount):
    print("\t{", end = "")
    for pixelCount in range(0, numPixels):
        r, g, b = image.getpixel((lineCount,numPixels - pixelCount - 1))
        print("0x%02X%02X%02X" % (r,g,b),end="")
        if pixelCount != numPixels - 1:
            print(",", end = "")
    print("},")
print("};")