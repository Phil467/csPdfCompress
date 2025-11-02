#ifndef CSPDFMANIP_H
#define CSPDFMANIP_H
#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <string>
//#include <podofo/podofo.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>
#include <cairo/cairo-pdf.h>

class csPDFMANIP
{
    public:
        csPDFMANIP();
        virtual ~csPDFMANIP();

    protected:

    private:
};
int getPageCount(const char* pdf_path);
bool convertPdfPageToBmp(const std::string& pdfPath, const std::string& outputPath, int pageNum = 0, int dpi = 300);
void createPdfFromPng(const std::vector<std::string>& image_paths, const std::string& output_pdf);
void mergePdfsA4Size(const std::vector<std::string>& input_pdfs, const std::string& output_pdf);
void mergePdfsWithVariablePageSizes(const std::vector<std::string>& input_pdfs,
                                       const std::string& output_pdf);
void mergePdfsHighQuality(const std::vector<std::string>& input_paths,
                           const std::string& output_path);
void createPdfFromPng_HARU(const char* pngFilePath, const char* pdfFilePath);
void createPdfFromPngs_HARU(const std::vector<const char*>& pngFilePaths, const char* pdfFilePath);

void saveImageAsBMP(cairo_surface_t* surf, const std::string& filename);
void popplerImageToBmp(poppler::image img, std::string output, int dpi);
#endif // CSPDFMANIP_H
