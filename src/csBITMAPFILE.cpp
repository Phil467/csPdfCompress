#include "csBITMAPFILE.h"

csBITMAPFILE::csBITMAPFILE()
{
    //ctor
}

csBITMAPFILE::~csBITMAPFILE()
{
    //dtor
}


#pragma pack(push, 1) // Désactive l'alignement pour la structure
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

void createBitmap24(const char* filename, int width, int height, uint8_t* pixelData) 
{
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Calcul du padding pour que chaque ligne ait une taille multiple de 4
    int padding = (4 - (width * 3) % 4) % 4;
    int imageSize = (width * 3 + padding) * height;

    // Initialisation des en-têtes
    BITMAPFILEHEADER fileHeader = {0};
    BITMAPINFOHEADER infoHeader = {0};

    // Remplissage du BITMAPFILEHEADER
    fileHeader.bfType = 0x4D42; // 'BM'
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Remplissage du BITMAPINFOHEADER
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24; // 24 bits par pixel
    infoHeader.biSizeImage = imageSize;

    // Écriture des en-têtes
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

    // Écriture des données pixels
    for (int y = height - 1; y >= 0; y--) { // Les lignes sont écrites de bas en haut
        fwrite(pixelData + y * width * 3, width * 3, 1, file);
        
        // Ajout du padding si nécessaire
        uint8_t pad = 0;
        for (int p = 0; p < padding; p++) {
            fwrite(&pad, 1, 1, file);
        }
    }

    fclose(file);
}


unsigned char* str32ToStr24Bmp(unsigned char*str, const int width, const int height) 
{
    // Allouer un nouveau buffer pour l'image 24 bits (RGB)
    int paddingSize = (4 - ((width * 3) % 4)) % 4;
    int stride = width * 3 + paddingSize;  // Taille d'une ligne avec padding
    int size = width * height * 3 + 1;
    unsigned char* rgb24Data = (uint8_t*)malloc(size);
    
    // Convertir de 32 bits (RGBA) à 24 bits (RGB)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIndex = (y * width + x) * 4;  // Index dans le buffer source (RGBA)
            int dstIndex = (y * width + x) * 3;  // Index dans le buffer destination (RGB)
            
            // Copier seulement les composantes R, G et B
            rgb24Data[dstIndex]     = str[srcIndex + 0];     // R
            rgb24Data[dstIndex + 1] = str[srcIndex + 1]; // G
            rgb24Data[dstIndex + 2] = str[srcIndex + 2]; // B
            // Ignorer str[srcIndex + 3] qui est le canal alpha (A)
        }
    }
    rgb24Data[size] = '\0';
    return rgb24Data;
}

unsigned char* str32ToStr24(unsigned char*str, const int width, const int height) 
{
    // Allouer un nouveau buffer pour l'image 24 bits (RGB)
    int paddingSize = (4 - ((width * 3) % 4)) % 4;
    int stride = width * 3 + paddingSize;  // Taille d'une ligne avec padding
    int size = width * height * 3 + 1;
    unsigned char* rgb24Data = (uint8_t*)malloc(size);
    
    // Convertir de 32 bits (RGBA) à 24 bits (RGB)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIndex = (y * width + x) * 4;  // Index dans le buffer source (RGBA)
            int dstIndex = (y * width + x) * 3;  // Index dans le buffer destination (RGB)
            
            // Copier seulement les composantes R, G et B
            rgb24Data[dstIndex]     = str[srcIndex + 2];     // R
            rgb24Data[dstIndex + 1] = str[srcIndex + 1]; // G
            rgb24Data[dstIndex + 2] = str[srcIndex + 0]; // B
            // Ignorer str[srcIndex + 3] qui est le canal alpha (A)
        }
    }
    rgb24Data[size] = '\0';
    return rgb24Data;
}


void createPNGFile(const char* filename, int width, int height, unsigned char* rgb24Data) {
    
    
    // Créer des pointeurs de ligne pour libpng
    png_bytep* row_pointers = new png_bytep[height];
    for (int y = 0; y < height; y++) {
        row_pointers[y] = rgb24Data + y * width * 3;
    }
    
    // Ouvrir le fichier
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        std::cerr << "Impossible d'ouvrir le fichier " << filename << std::endl;
        delete[] row_pointers;
        return;
    }
    
    // Initialiser les structures libpng
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cerr << "Erreur lors de la création de la structure png_write" << std::endl;
        fclose(fp);
        delete[] rgb24Data;
        delete[] row_pointers;
        return;
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr << "Erreur lors de la création de la structure png_info" << std::endl;
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        delete[] rgb24Data;
        delete[] row_pointers;
        return;
    }
    
    // Configuration de la gestion d'erreur
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "Erreur lors de l'écriture du PNG" << std::endl;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        delete[] rgb24Data;
        delete[] row_pointers;
        return;
    }
    
    // Initialiser l'E/S
    png_init_io(png_ptr, fp);
    
    // Configurer les en-têtes PNG
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, 
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    // Écrire les en-têtes
    png_write_info(png_ptr, info_ptr);
    
    // Écrire l'image
    png_write_image(png_ptr, row_pointers);
    
    // Finaliser l'écriture
    png_write_end(png_ptr, NULL);
    
    // Nettoyage
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    delete[] row_pointers;
    
    std::cout << "Image PNG enregistrée avec succès!" << std::endl;

   // GhostscriptProcessor gs;
}

