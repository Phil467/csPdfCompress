/* This program implements an algorithm for compressing large PDFs. The PDF is loaded, its pages are extracted, 
/* and then each page is processed by an algorithm that changes the color of any pixel near a given color to that color. 
/* This algorithm can be applied as many times as there are reference pixels. The pages are then each saved as PNG files, 
/* and afterwards all are recombined into a PDF. The final PDF is smaller and more readable.
/* The program uses the Cairo and Poppler libraries to manipulate PDFs.*/

#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <sstream>
#include <vector>
//#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include "csBITMAPFILE.h"
#include "csPDFMANIP.h"

using namespace std;


typedef struct
{
    HDC dc;
    HBITMAP hbmp;
    BITMAP bm;

} csGRAPHIC_CONTEXT_EXT;

BITMAPINFO csSetBMI(SIZE sz);
csGRAPHIC_CONTEXT_EXT csGetImageGraphicContextExt(char*path, BITMAPINFO*bmi);
void csFreeGraphicContextExt(csGRAPHIC_CONTEXT_EXT bmp);
void bitmapCorrection(unsigned char*&map, int cx, int cy, csRGB color={255,255,255}, int tolerance = 20);
void csCompressPdf();

int main()
{
    csCompressPdf();

    // scompressWithGhostscript("bulletins_hydro.pdf", "bulletins_hydro_compressed.pdf", 150);
    // std::vector<std::string> paths = {"ldr_geoph_Compressed.pdf", "bulletins_geoph_Compressed.pdf"};
    // mergePdfsWithVariablePageSizes(paths, "ldr_bulletins_geoph_Merged.pdf");
    // mergePdfsA4Size({"ldr_hydro_Compressed.pdf", "bulletins_hydro_Compressed.pdf"}, "ldr_bulletins_hydro_Merged.pdf");
    // mergePdfsHighQuality(paths, "ldr_bulletins_geoph_Merged.pdf");
    // cout << "End !" << endl;
    
    return 0;
}


void csCompressPdf()
{
    BITMAPINFO bmi;
    string pdf_path = "ldr_geoph.pdf";

    char* str = (char*)pdf_path.c_str();
    char* prefix = (char*)malloc(pdf_path.size());

    for(int i=0; i<pdf_path.size()-4; i++)
    {
        prefix[i] = str[i];
    }
    prefix[pdf_path.size()-4] = '\0';

    CreateDirectory(prefix,0);

    int N = getPageCount(pdf_path.c_str());

    std::vector<std::string> image_paths;

    for(int i = 0; i<N; i++)
    {
        char bmpFileName[pdf_path.size()*2+20+1];
        sprintf(bmpFileName, "%s/%s_Page%d.bmp\0", prefix, prefix, i);

        char pngFileName[pdf_path.size()+20+1];
        sprintf(pngFileName, "%s/%s_Page%d.png\0", prefix, prefix, i);

        convertPdfPageToBmp(pdf_path, bmpFileName, i, 100);

        csGRAPHIC_CONTEXT_EXT gce = csGetImageGraphicContextExt(bmpFileName, &bmi);
        unsigned char*map = (unsigned char*)gce.bm.bmBits;

        bitmapCorrection(map, gce.bm.bmWidth, gce.bm.bmHeight, {255,255,255}, 60);
        bitmapCorrection(map, gce.bm.bmWidth, gce.bm.bmHeight, {0,0,0}, 100);

        unsigned char* map2 = str32ToStr24(map, gce.bm.bmWidth, gce.bm.bmHeight);
        //createBitmap24("Page_1.bmp", gce.bm.bmWidth, gce.bm.bmHeight, map2);

        createPNGFile(pngFileName, gce.bm.bmWidth, gce.bm.bmHeight, map2);
        //bmpToPdfCentered(pngFileName, "Page_1.pdf", 37.8*21, 37.8*29.7);

        image_paths.push_back(pngFileName);

        csFreeGraphicContextExt(gce);
        free(map2);
        DeleteFile(bmpFileName);

        cout<<"done : "<<i+1<<"/"<<N<<"\n";
    }

    char pdfFileName[pdf_path.size()*2+20+1];
    sprintf(pdfFileName, "%s_Compressed.pdf\0", prefix);
    createPdfFromPng(image_paths, pdfFileName);

    //void mergePdfsA4Size(const std::vector<std::string>& input_pdfs, const std::string& output_pdf) ;

    free(prefix);

}


BITMAPINFO csSetBMI(SIZE sz)
{
    BITMAPINFO bi;
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = sz.cx;
    bi.bmiHeader.biHeight = -sz.cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = sz.cy * 4 * sz.cx;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;
    return bi;
}

csGRAPHIC_CONTEXT_EXT csGetImageGraphicContextExt(char*path, BITMAPINFO*bmi)
{
    csGRAPHIC_CONTEXT_EXT bmp;
    bmp.hbmp=(HBITMAP)LoadImageA(NULL,path,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    GetObjectA((HGDIOBJ)bmp.hbmp,sizeof(bmp.bm),(PVOID)&bmp.bm);
    char*buf0 = (char*)bmp.bm.bmBits;
    HDC dc=GetDC(0);
    bmp.dc=CreateCompatibleDC(dc);
    SelectBitmap(bmp.dc,bmp.hbmp);


    *bmi = csSetBMI({bmp.bm.bmWidth, bmp.bm.bmHeight});
    //cout<<bmp.bm.bmHeight<<" ----- size\n";
    /*DrawStateA(bmp.dc,NULL,NULL,
               (long long)bmp.hbmp,NULL,0,0,0,0,DST_BITMAP);*/
    bmp.bm.bmBits = (LPVOID)malloc(bmi->bmiHeader.biSizeImage*sizeof(void));
    int bres = GetDIBits(bmp.dc,bmp.hbmp,0,bmp.bm.bmHeight,bmp.bm.bmBits,bmi,DIB_RGB_COLORS);
    ReleaseDC(0,dc);
    return bmp;
}


void csFreeGraphicContextExt(csGRAPHIC_CONTEXT_EXT bmp)
{
    DeleteDC(bmp.dc);
    DeleteObject(bmp.hbmp);
    free(bmp.bm.bmBits);
}

void bitmapCorrection(unsigned char*&map, int cx, int cy, csRGB color, int tolerance)
{
    unsigned char r = color.r>tolerance ? color.r-tolerance : color.r;
    unsigned char g = color.g>tolerance ? color.g-tolerance : color.g;
    unsigned char b = color.b>tolerance ? color.b-tolerance : color.b;

    unsigned char r1 = color.r+tolerance<255 ? color.r+tolerance : 255;
    unsigned char g1 = color.g+tolerance<255 ? color.g+tolerance : 255;
    unsigned char b1 = color.b+tolerance<255 ? color.b+tolerance : 255;

    for(int j=0; j<cy; j++)
    for(int i=0; i<cx; i++)
    {
        long n = (j*cx+i)*4;
        if((map[n] >= b && map[n] <= b1) && (map[n+1] >= g && map[n+1] < g1) && (map[n+2] >= r && map[n+2] < r1))
        {
            map[n] = color.b;
            map[n+1] = color.g;
            map[n+2] = color.r;
        }
    }
}


bool isValidBmpFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    // Vérifier l'en-tête BMP
    char header[2];
    file.read(header, 2);

    return (header[0] == 'B' && header[1] == 'M');
}

