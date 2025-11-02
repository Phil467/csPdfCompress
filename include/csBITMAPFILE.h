#ifndef CSBITMAPFILE_H
#define CSBITMAPFILE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <png.h>
#include <iostream>
#include <cstring>

//#include <ghostscript/gdevdsp.h>
//#include <ghostscript/iapi.h>

typedef struct DLL_EXPORT
{
    unsigned char r, g, b;
    double a;

}csRGBA, csRGB;

class csBITMAPFILE
{
    public:
        csBITMAPFILE();
        virtual ~csBITMAPFILE();

    protected:

    private:
};

void createBitmap24(const char* filename, int width, int height, uint8_t* pixelData);
unsigned char* str32ToStr24(unsigned char*str, const int width, const int height);
void createPNGFile(const char* filename, int width, int height, unsigned char* rgb24Data);

#endif // CSBITMAPFILE_H
