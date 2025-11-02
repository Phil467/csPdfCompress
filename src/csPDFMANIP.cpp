#include "csPDFMANIP.h"

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

        if (!doc) 
	{
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
        if (!doc || doc->is_locked()) 
	{
            std::cerr << "Impossible d'ouvrir le document PDF ou document verrouillé." << std::endl;
            delete doc;
            return false;
        }

        // Vérifier si le numéro de page est valide
        if (pageNum >= doc->pages() || pageNum < 0) 
	{
            std::cerr << "Numéro de page invalide. Le document a " << doc->pages() << " pages." << std::endl;
            delete doc;
            return false;
        }

        // Obtenir la page
        poppler::page* page = doc->create_page(pageNum);
        if (!page) 
	{
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
        if (!file) 
	{
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
        for (int y = height - 1; y >= 0; y--) 
	{
            const char* srcLine = img.const_data() + y * bytesPerLine;

            // Convertir de BGRA (Poppler) à BGR (BMP)
            for (int x = 0; x < width; x++) 
	    {
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
        if (!file) 
	{
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
        for (int y = height - 1; y >= 0; y--) 
	{
            const char* srcLine = img.const_data() + y * bytesPerLine;

            // Convertir de BGRA (Poppler) à BGR (BMP)
            for (int x = 0; x < width; x++) 
	    {
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

    for (const auto& img_path : image_paths) 
    {
        // Charger l'image PNG avec Cairo
        cairo_surface_t* image = cairo_image_surface_create_from_png(img_path.c_str());

        if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) 
	{
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
        if (&img_path != &image_paths.back()) 
	{
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

    for (const auto& pdf_file : input_pdfs) 
    {
        // Charger chaque PDF d'entrée
        std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdf_file));
        if (!doc) 
	{
            std::cerr << "Erreur: Impossible de charger " << pdf_file << std::endl;
            continue;
        }

        // Parcourir toutes les pages
        for (int i = 0; i < doc->pages(); i++)
        {

            std::unique_ptr<poppler::page> p(doc->create_page(i));

            if (!p) continue;

            // Créer une nouvelle page dans le PDF de sortie
            if (i > 0 || &pdf_file != &input_pdfs[0]) 
	    {
                std::cout<<i<<" iuou  ";
                cairo_show_page(cr);
            }

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


        }
    }

    // Finaliser
    cairo_destroy(cr);
    cairo_surface_destroy(output_surface);
}

void mergePdfsWithVariablePageSizes(const std::vector<std::string>& input_pdfs,
                         const std::string& output_pdf) 
{
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
    for (const auto& pdf_path : input_pdfs) 
    {
        try {
            std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdf_path));
            if (!doc) {
                throw std::runtime_error("Failed to load PDF");
            }

            // 3. Render each page
            for (int i = 0; i < doc->pages(); ++i) 
	    {
                std::unique_ptr<poppler::page> page(doc->create_page(i));
                if (!page) continue;

                // Set page size
                poppler::rectf rect = page->page_rect();
                if (!first_page) 
		{
                    cairo_show_page(cr);
                } 
		else 
		{
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
