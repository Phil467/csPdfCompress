/* This program implements an algorithm for compressing large PDFs. The PDF is loaded, its pages are extracted, 
/* and then each page is processed by an algorithm that changes the color of any pixel near a given color to that color. 
/* This algorithm can be applied as many times as there are reference pixels. The pages are then each saved as PNG files, 
/* and afterwards all are recombined into a PDF. The final PDF is smaller and more readable.
/* The program uses the Cairo and Poppler libraries to manipulate PDFs, and libpng to manipulate PNGs.
/* The file ldr_geoph.pdf is used to show how it works. The results are saved in the created folder "ldr_geoph". */


#include "csBITMAPFILE.h"
#include "csPDFMANIP.h"

using namespace std;


void csCompressPdf();

int main()
{
    csCompressPdf();

    // std::vector<std::string> paths = {"ldr_geoph_Compressed.pdf", "bulletins_geoph_Compressed.pdf"};
    // mergePdfsWithVariablePageSizes(paths, "ldr_bulletins_geoph_Merged.pdf");
    // mergePdfsA4Size({"ldr_hydro_Compressed.pdf", "bulletins_hydro_Compressed.pdf"}, "ldr_bulletins_hydro_Merged.pdf");
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



