#ifndef CSBITMAPFILE_H
#define CSBITMAPFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>

#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <sstream>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

#include <png.h>
#include <iostream>
#include <cstring>


typedef struct
{
    HDC dc;
    HBITMAP hbmp;
    BITMAP bm;

} csGRAPHIC_CONTEXT_EXT;

typedef struct DLL_EXPORT
{
    unsigned char r, g, b;
    double a;

}csRGBA, csRGB;

BITMAPINFO csSetBMI(SIZE sz);
csGRAPHIC_CONTEXT_EXT csGetImageGraphicContextExt(char*path, BITMAPINFO*bmi);
void csFreeGraphicContextExt(csGRAPHIC_CONTEXT_EXT bmp);
void bitmapCorrection(unsigned char*&map, int cx, int cy, csRGB color={255,255,255}, int tolerance = 20);

void createBitmap24(const char* filename, int width, int height, uint8_t* pixelData);
unsigned char* str32ToStr24(unsigned char*str, const int width, const int height);
void createPNGFile(const char* filename, int width, int height, unsigned char* rgb24Data);

#endif // CSBITMAPFILE_H
