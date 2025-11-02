#include "csPDFMANIP.h"

csPDFMANIP::csPDFMANIP()
{
    //ctor
}

csPDFMANIP::~csPDFMANIP()
{
    //dtor
}

#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>
#include <iostream>
//#include "stb_image_write.h" 

int getPageCount(const char* pdf_path)
{
    int page_count;
    try {
        // Charger le document PDF
        std::unique_ptr<poppler::document> doc(
            poppler::document::load_from_file(pdf_path)
        );

        if (!doc) {
            std::cerr << "Erreur: Impossible de charger le PDF" << std::endl;
            return 1;
        }

        // Récupérer le nombre de pages
        page_count = doc->pages();

        std::cout << "Nombre de pages : " << page_count << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 0;
    }

    return page_count;
}

bool convertPdfPageToBmp(const std::string& pdfPath, const std::string& outputPath, int pageNum, int dpi)
{
    try {
        // Ouvrir le document PDF avec Poppler
        poppler::document* doc = poppler::document::load_from_file(pdfPath);
        if (!doc || doc->is_locked()) {
            std::cerr << "Impossible d'ouvrir le document PDF ou document verrouillé." << std::endl;
            delete doc;
            return false;
        }

        // Vérifier si le numéro de page est valide
        if (pageNum >= doc->pages() || pageNum < 0) {
            std::cerr << "Numéro de page invalide. Le document a " << doc->pages() << " pages." << std::endl;
            delete doc;
            return false;
        }

        // Obtenir la page
        poppler::page* page = doc->create_page(pageNum);
        if (!page) {
            std::cerr << "Impossible de créer la page." << std::endl;
            delete doc;
            return false;
        }

        // Configurer le rendu
        poppler::page_renderer renderer;
        renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
        renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);

        // Rendre la page en image
        poppler::image img = renderer.render_page(page, dpi, dpi);
        if (!img.is_valid()) {
            std::cerr << "Échec du rendu de la page." << std::endl;
            delete page;
            delete doc;
            return false;
        }

        // Convertir l'image en format BMP
        // Poppler ne supporte pas directement l'enregistrement en BMP, nous allons donc
        // utiliser les données brutes pour créer un fichier BMP

        int width = img.width();
        int height = img.height();
        int bytesPerLine = img.bytes_per_row();

        // Calculer le padding BMP (chaque ligne doit être alignée sur 4 octets)
        int paddingSize = (4 - ((width * 3) % 4)) % 4;
        int stride = width * 3 + paddingSize;

        // Créer l'en-tête du fichier BMP
        BITMAPFILEHEADER fileHeader;
        BITMAPINFOHEADER infoHeader;

        fileHeader.bfType = 0x4D42; // "BM"
        fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + stride * height;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;
        fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        infoHeader.biSize = sizeof(BITMAPINFOHEADER);
        infoHeader.biWidth = width;
        infoHeader.biHeight = height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 24; // 24 bits par pixel (RGB)
        infoHeader.biCompression = BI_RGB;
        infoHeader.biSizeImage = stride * height;
        infoHeader.biXPelsPerMeter = dpi * 39.37; // Conversion DPI en pixels par mètre
        infoHeader.biYPelsPerMeter = dpi * 39.37;
        infoHeader.biClrUsed = 0;
        infoHeader.biClrImportant = 0;

        // Ouvrir le fichier de sortie
        FILE* file = fopen(outputPath.c_str(), "wb");
        if (!file) {
            std::cerr << "Impossible d'ouvrir le fichier de sortie." << std::endl;
            delete page;
            delete doc;
            return false;
        }

        // Écrire les en-têtes
        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
        fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

        // Allouer un buffer pour une ligne BMP
        unsigned char* lineBuffer = new unsigned char[stride];
        memset(lineBuffer, 0, stride); // Initialiser à zéro pour le padding

        // Convertir les données d'image et les écrire ligne par ligne
        // Note: BMP stocke les lignes de bas en haut
        for (int y = height - 1; y >= 0; y--) {
            const char* srcLine = img.const_data() + y * bytesPerLine;

            // Convertir de BGRA (Poppler) à BGR (BMP)
            for (int x = 0; x < width; x++) {
                lineBuffer[x * 3]     = srcLine[x * 4];     // B
                lineBuffer[x * 3 + 1] = srcLine[x * 4 + 1]; // G
                lineBuffer[x * 3 + 2] = srcLine[x * 4 + 2]; // R
                // Ignorer le canal alpha (srcLine[x * 4 + 3])
            }

            // Écrire la ligne
            fwrite(lineBuffer, stride, 1, file);
        }

        // Nettoyer
        delete[] lineBuffer;
        fclose(file);
        delete page;
        delete doc;

        std::cout << "Conversion réussie: " << pdfPath << " (page " << pageNum + 1
                  << ") -> " << outputPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}
/*
void extractBmpFromPdf(const std::string& pdfFilePath, int dpi) 
{
    // Charger le document PDF
    poppler::document* doc = poppler::document::load_from_file(pdfFilePath);
    if (!doc) {
        std::cerr << "Erreur lors du chargement du fichier PDF." << std::endl;
        return;
    }

    // Parcourir les pages du document
    for (int i = 0; i < doc->pages(); ++i) {
        poppler::page* p = doc->create_page(i);
        if (!p) {
            std::cerr << "Erreur lors de la création de la page." << std::endl;
            continue;
        }
int j = 0;
        // Obtenir la liste des images sur la page
        GList* imageMapping = poppler_page_get_image_mapping(POPPLER_PAGE(p));
        for (GList* iter = imageMapping; iter != nullptr; iter = iter->next) {
            
            PopplerImageMapping* mapping = static_cast<PopplerImageMapping*>(iter->data);
            cairo_surface_t *surf = poppler_page_get_image(POPPLER_PAGE(p), mapping->image_id);
            int width = cairo_image_surface_get_width(surf);
            int height = cairo_image_surface_get_height(surf);
            
            // Obtenir les données de l'image
            unsigned char* data = cairo_image_surface_get_data(surf);
            
            // Enregistrer l'image au format BMP
            std::string output = "image_" + std::to_string(i) + "_" + std::to_string(mapping->image_id) + ".bmp";
            saveImageAsBMP(surf, output);
            std::cout << "Image enregistrée : " << output << std::endl;
            j++;
        }

        g_list_free(imageMapping); // Libérer la liste des images
        delete p; // Libérer la mémoire de la page
    }

    delete doc; // Libérer la mémoire du document
}
*/
/*void saveImageAsBMP(cairo_surface_t* surf, const std::string& filename) {
    // Obtenir les dimensions de la surface
    int width = cairo_image_surface_get_width(surf);
    int height = cairo_image_surface_get_height(surf);
    
    // Obtenir les données de l'image
    unsigned char* data = cairo_image_surface_get_data(surf);
    
    // Enregistrer l'image au format BMP
    stbi_write_bmp(filename.c_str(), width, height, 4, data); // 4 pour RGBA
}*/

void popplerImageToBmp(poppler::image img, std::string output, int dpi)
{
        int width = img.width();
        int height = img.height();
        int bytesPerLine = img.bytes_per_row();

        // Calculer le padding BMP (chaque ligne doit être alignée sur 4 octets)
        int paddingSize = (4 - ((width * 3) % 4)) % 4;
        int stride = width * 3 + paddingSize;

        // Créer l'en-tête du fichier BMP
        BITMAPFILEHEADER fileHeader;
        BITMAPINFOHEADER infoHeader;

        fileHeader.bfType = 0x4D42; // "BM"
        fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + stride * height;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;
        fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        infoHeader.biSize = sizeof(BITMAPINFOHEADER);
        infoHeader.biWidth = width;
        infoHeader.biHeight = height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 24; // 24 bits par pixel (RGB)
        infoHeader.biCompression = BI_RGB;
        infoHeader.biSizeImage = stride * height;
        infoHeader.biXPelsPerMeter = dpi * 39.37; // Conversion DPI en pixels par mètre
        infoHeader.biYPelsPerMeter = dpi * 39.37;
        infoHeader.biClrUsed = 0;
        infoHeader.biClrImportant = 0;

        // Ouvrir le fichier de sortie
        FILE* file = fopen(output.c_str(), "wb");
        if (!file) {
            std::cerr << "Impossible d'ouvrir le fichier de sortie." << std::endl;
            return;
        }

        // Écrire les en-têtes
        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
        fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

        // Allouer un buffer pour une ligne BMP
        unsigned char* lineBuffer = new unsigned char[stride];
        memset(lineBuffer, 0, stride); // Initialiser à zéro pour le padding

        // Convertir les données d'image et les écrire ligne par ligne
        // Note: BMP stocke les lignes de bas en haut
        for (int y = height - 1; y >= 0; y--) {
            const char* srcLine = img.const_data() + y * bytesPerLine;

            // Convertir de BGRA (Poppler) à BGR (BMP)
            for (int x = 0; x < width; x++) {
                int x3 = x * 3;
                int x4 = x * 4;
                lineBuffer[x3]     = srcLine[x4];     // B
                lineBuffer[x3 + 1] = srcLine[x4 + 1]; // G
                lineBuffer[x3 + 2] = srcLine[x4 + 2]; // R
                // Ignorer le canal alpha (srcLine[x * 4 + 3])
            }

            // Écrire la ligne
            fwrite(lineBuffer, stride, 1, file);
        }

        // Nettoyer
        delete[] lineBuffer;
        fclose(file);

        std::cout << "Conversion réussie: " << output << std::endl;
        
}


void createPdfFromPng(const std::vector<std::string>& image_paths, const std::string& output_pdf)
{
    // Créer un surface PDF avec Cairo
    cairo_surface_t* surface = cairo_pdf_surface_create(output_pdf.c_str(), 595.0, 842.0); // A4 (72 DPI)
    cairo_t* cr = cairo_create(surface);

    for (const auto& img_path : image_paths) {
        // Charger l'image PNG avec Cairo
        cairo_surface_t* image = cairo_image_surface_create_from_png(img_path.c_str());

        if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
            std::cerr << "Erreur: Impossible de charger " << img_path << std::endl;
            continue;
        }

        // Dimensions de l'image et de la page
        double img_width = cairo_image_surface_get_width(image);
        double img_height = cairo_image_surface_get_height(image);
        double scale = std::min(595.0/img_width, 842.0/img_height); // Ajustement à la page A4

        // Dessiner l'image centrée
        cairo_save(cr);
        cairo_scale(cr, scale, scale);
        cairo_set_source_surface(cr, image,
                               (595.0/scale - img_width)/2,
                               (842.0/scale - img_height)/2);
        cairo_paint(cr);
        cairo_restore(cr);

        // Nouvelle page pour l'image suivante
        if (&img_path != &image_paths.back()) {
            cairo_show_page(cr);
        }

        cairo_surface_destroy(image);
    }

    // Finaliser
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void mergePdfsA4Size(const std::vector<std::string>& input_pdfs, const std::string& output_pdf)
{
    // Créer le PDF de sortie
    cairo_surface_t* output_surface = cairo_pdf_surface_create(output_pdf.c_str(), 595.0, 842.0); // A4
    cairo_t* cr = cairo_create(output_surface);

    for (const auto& pdf_file : input_pdfs) {
        // Charger chaque PDF d'entrée
        std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdf_file));
        if (!doc) {
            std::cerr << "Erreur: Impossible de charger " << pdf_file << std::endl;
            continue;
        }

        // Parcourir toutes les pages
        for (int i = 0; i < doc->pages(); i++)
        {

            std::unique_ptr<poppler::page> p(doc->create_page(i));

            if (!p) continue;

            // Créer une nouvelle page dans le PDF de sortie
            if (i > 0 || &pdf_file != &input_pdfs[0]) {
                std::cout<<i<<" iuou  ";
                cairo_show_page(cr);
            }
std::cout<<"iuou ";
            // Dessiner la page
            poppler::page_renderer renderer;
            poppler::image img = renderer.render_page(p.get());
            cairo_surface_t* img_surface = cairo_image_surface_create_for_data(
                (unsigned char*)img.data(),
                CAIRO_FORMAT_RGB24,
                img.width(),
                img.height(),
                img.bytes_per_row()
            );

            cairo_set_source_surface(cr, img_surface, 0, 0);
            cairo_paint(cr);
            cairo_surface_destroy(img_surface);

        std::cout<<"iuou ";
        }
    }

    // Finaliser
    cairo_destroy(cr);
    cairo_surface_destroy(output_surface);
}

void mergePdfsWithVariablePageSizes(const std::vector<std::string>& input_pdfs,
                         const std::string& output_pdf) {
   // 1. Create output PDF
    cairo_surface_t* surface = cairo_pdf_surface_create(output_pdf.c_str(), 595.0, 842.0);
    cairo_t* cr = cairo_create(surface);
    //cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_COMPRESSED, "false");
    cairo_pdf_surface_restrict_to_version(surface, CAIRO_PDF_VERSION_1_5);
    bool first_page = true;
    poppler::page_renderer renderer;

    // Configure for best quality
    renderer.set_render_hint(poppler::page_renderer::antialiasing, false);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, false);

    // 2. Process each PDF
    for (const auto& pdf_path : input_pdfs) {
        try {
            std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdf_path));
            if (!doc) {
                throw std::runtime_error("Failed to load PDF");
            }

            // 3. Render each page
            for (int i = 0; i < doc->pages(); ++i) {
                std::unique_ptr<poppler::page> page(doc->create_page(i));
                if (!page) continue;

                // Set page size
                poppler::rectf rect = page->page_rect();
                if (!first_page) {
                    cairo_show_page(cr);
                } else {
                    first_page = false;
                }
                cairo_pdf_surface_set_size(surface, rect.width(), rect.height());

                // Render at high DPI (300)
                poppler::image img = renderer.render_page(page.get());

                // Convert to Cairo surface
                cairo_surface_t* img_surface = cairo_image_surface_create_for_data(
                    (unsigned char*)img.data(),
                    CAIRO_FORMAT_RGB24,
                    img.width(),
                    img.height(),
                    img.bytes_per_row()
                );

                // Draw to PDF
                cairo_save(cr);
                cairo_set_source_surface(cr, img_surface, 0, 0);
                cairo_paint(cr);
                cairo_restore(cr);

                cairo_surface_destroy(img_surface);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing " << pdf_path << ": " << e.what() << std::endl;
        }
    }

    // Cleanup
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
/*
void mergePdfsHighQuality(const std::vector<std::string>& input_paths,
                           const std::string& output_path) {
    try {
        // 1. Initialisation
        PoDoFo::PdfMemDocument output_stream;
//output_stream.Load(output_path.c_str());
int index = 0;
        // 2. Fusion des PDFs
        for (const auto& input_path : input_paths) {
            PoDoFo::PdfMemDocument input_doc;
            input_doc.Load(input_path.c_str());

            // 3. Copie des pages sans ré-encodage
            for (int i = 0; i < PoDoFo::PdfPageCollection(input_doc).GetCount(); ++i) {
                PoDoFo::PdfPage& page = PoDoFo::PdfPageCollection(input_doc).GetPageAt(i);
                PoDoFo::PdfPage& new_page = PoDoFo::PdfPageCollection(output_stream).CreatePage(
                    page.GetMediaBox()
                );

                // Copie directe du contenu vectoriel
                //new_page.GetContents()->Add(page->GetContents()->GetObject()->Copy());
                PoDoFo::PdfPageCollection(output_stream).InsertDocumentPageAt(index, input_doc, i);
                //index++;
            }
        }

        // 4. Désactivation de la compression
       // output_stream.SetCompress(false);
        //output_stream.Close();

    } catch (PoDoFo::PdfError& e) {
        std::cerr << "Erreur PoDoFo: " << e.what() << std::endl;
    }
}*/
/*
std::string getMetadataString(const PoDoFo::nullable<const PoDoFo::PdfString&>& metadata, const std::string& defaultValue = "Non défini") {
    if (metadata.has_value()) {
        return metadata.value().GetString();
    }
    return defaultValue;
}

// Fonction pour obtenir des informations sur le PDF avec PoDoFo
void getPdfInfo(const std::string& pdfPath) {
    try {
        // Initialiser PoDoFo
        PoDoFo::PdfMemDocument document;
        document.Load(pdfPath.c_str());

        // Obtenir le nombre de pages
        int pageCount = document.GetPages().GetCount();
        std::cout << "Nombre de pages: " << pageCount << std::endl;

        // Obtenir les dimensions de la première page
        PoDoFo::PdfPage& page = document.GetPages().GetPageAt(0);
        PoDoFo::Rect rect = page.GetMediaBox();

        std::cout << "Dimensions de la première page: "
                  << rect.Width << " x " << rect.Height
                  << " points" << std::endl;

        // Autres informations
        const PoDoFo::PdfInfo* info = document.GetInfo();
        if (info) {
            std::cout << "Titre: " << getMetadataString(info->GetTitle()) << std::endl;
            std::cout << "Auteur: " << getMetadataString(info->GetAuthor()) << std::endl;
            std::cout << "Sujet: " << getMetadataString(info->GetSubject())<< std::endl;
            std::cout << "Mots-clés: " << getMetadataString(info->GetKeywords())<< std::endl;
            std::cout << "Créateur: " << getMetadataString(info->GetCreator()) << std::endl;
            std::cout << "Producteur: " << getMetadataString(info->GetProducer())<< std::endl;
        }
    }
    catch (const PoDoFo::PdfError& e) {
        std::cerr << "Erreur PoDoFo: " << e.what() << std::endl;
    }
}

int test(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << (" <fichier_pdf> <fichier_bmp_sortie> [numéro_page] [dpi]") << std::endl;
        return 1;
    }

    //std::string pdfPath = argv[1];
    //std::string bmpPath = argv[2];
    std::string pdfPath = "bulletins_hydro.pdf";
    std::string bmpPath = "bulletins_hydro.bmp";
    int pageNum = (argc > 3) ? std::atoi(argv[3]) - 1 : 0; // Conversion en base 0
    int dpi = (argc > 4) ? std::atoi(argv[4]) : 300;

    // Afficher les informations sur le PDF
    getPdfInfo(pdfPath);

    // Convertir la page en BMP
    if (convertPdfPageToBmp(pdfPath, bmpPath, pageNum, dpi)) {
        return 0;
    } else {
        return 1;
    }
}
*/

/*
#include <hpdf.h>

void createPdfFromPng_HARU(const char* pngFilePath, const char* pdfFilePath) 
{
    // Créer un document PDF
    HPDF_Doc pdf = HPDF_New(nullptr, nullptr);
    if (!pdf) {
        std::cerr << "Erreur lors de la création du document PDF." << std::endl;
        return;
    }

    // Ajouter une page au document
    HPDF_Page page = HPDF_AddPage(pdf);
    if (!page) {
        std::cerr << "Erreur lors de l'ajout de la page." << std::endl;
        HPDF_Free(pdf);
        return;
    }

    // Charger l'image PNG
    HPDF_Image image = HPDF_LoadPngImageFromFile(pdf, pngFilePath);
    if (!image) {
        std::cerr << "Erreur lors du chargement de l'image PNG." << std::endl;
        HPDF_Free(pdf);
        return;
    }

    // Obtenir les dimensions de l'image
    float width = HPDF_Image_GetWidth(image);
    float height = HPDF_Image_GetHeight(image);

    // Définir la taille de la page en fonction de l'image
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    HPDF_Page_SetWidth(page, width);
    HPDF_Page_SetHeight(page, height);

    // Dessiner l'image sur la page
    HPDF_Page_DrawImage(page, image, 0, 0, width, height);

    // Enregistrer le document PDF
    if (HPDF_SaveToFile(pdf, pdfFilePath) != HPDF_OK) {
        std::cerr << "Erreur lors de l'enregistrement du fichier PDF." << std::endl;
    }

    // Libérer les ressources
    HPDF_Free(pdf);
}
/*

void createPdfFromPngs_HARU(const std::vector<const char*>& pngFilePaths, const char* pdfFilePath) 
{
    // Créer un document PDF
    HPDF_Doc pdf = HPDF_New(nullptr, nullptr);
    if (!pdf) {
        std::cerr << "Erreur lors de la création du document PDF." << std::endl;
        return;
    }

    for (const char* pngFilePath : pngFilePaths) {
        // Ajouter une page au document
        HPDF_Page page = HPDF_AddPage(pdf);
        if (!page) {
            std::cerr << "Erreur lors de l'ajout de la page." << std::endl;
            HPDF_Free(pdf);
            return;
        }

        // Charger l'image PNG
        HPDF_Image image = HPDF_LoadPngImageFromFile(pdf, pngFilePath);
        if (!image) {
            std::cerr << "Erreur lors du chargement de l'image PNG: " << pngFilePath << std::endl;
            HPDF_Free(pdf);
            return;
        }

        // Obtenir les dimensions de l'image
        float width = HPDF_Image_GetWidth(image);
        float height = HPDF_Image_GetHeight(image);

        // Définir la taille de la page en fonction de l'image
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
        HPDF_Page_SetWidth(page, width);
        HPDF_Page_SetHeight(page, height);

        // Dessiner l'image sur la page
        HPDF_Page_DrawImage(page, image, 0, 0, width, height);
    }

    // Enregistrer le document PDF
    if (HPDF_SaveToFile(pdf, pdfFilePath) != HPDF_OK) {
        std::cerr << "Erreur lors de l'enregistrement du fichier PDF." << std::endl;
    }

    // Libérer les ressources
    HPDF_Free(pdf);
}
*/
/*int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <output.pdf> <input1.png> <input2.png> ..." << std::endl;
        return 1;
    }

    const char* pdfFilePath = argv[1];
    std::vector<const char*> pngFilePaths;

    // Récupérer les chemins des fichiers PNG
    for (int i = 2; i < argc; ++i) {
        pngFilePaths.push_back(argv[i]);
    }

    convertPngsToPdf(pngFilePaths, pdfFilePath);
    return 0;
}*/