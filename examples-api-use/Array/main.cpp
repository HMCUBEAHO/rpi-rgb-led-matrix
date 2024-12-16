#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>
#include <led-matrix.h>  // Header der RGB-LED-Matrix-Bibliothek
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // Lade die stb_image.h Bibliothek

// Größe eines einzelnen Bildschirms
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 64;
const int NUM_SCREENS = 6; // Anzahl der Bildschirme in einer Reihe
const int ARRAY_WIDTH = SCREEN_WIDTH * NUM_SCREENS; // Gesamtbreite des Arrays

// Definition eines RGB-Pixels
struct RGB {
    uint8_t r, g, b;
};

// Das Array, das alle Bildschirme enthält
std::vector<RGB> screens(ARRAY_WIDTH * SCREEN_HEIGHT, {0, 0, 0}); // Initialisiert mit schwarz (0, 0, 0)

// Bild auf den Bildschirm einfügen
void insertImage(const std::vector<RGB>& image, int screenNumber) {
    if (screenNumber < 0 || screenNumber >= NUM_SCREENS) {
        std::cerr << "Ungültige Bildschirmnummer!" << std::endl;
        return;
    }

    int xOffset = screenNumber * SCREEN_WIDTH; // Berechnung der horizontalen Verschiebung

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            int indexInArray = (y * ARRAY_WIDTH) + xOffset + x;
            int indexInImage = y * SCREEN_WIDTH + x;
            screens[indexInArray] = image[indexInImage];  // Bildpixel in Array einfügen
        }
    }
}

// RGB in uint32_t umwandeln (für die Matrix-Bibliothek)
uint32_t rgbToUint32(const RGB& pixel) {
    return (static_cast<uint32_t>(pixel.r) << 16) | (static_cast<uint32_t>(pixel.g) << 8) | static_cast<uint32_t>(pixel.b);
}

// Bild einlesen und auf 64x64 verkleinern
std::vector<RGB> loadImage(const std::string& filename) {
    int width, height, channels;
    unsigned char* imgData = stbi_load(filename.c_str(), &width, &height, &channels, 3); // Lädt das Bild mit 3 Kanälen (RGB)

    if (imgData == nullptr) {
        std::cerr << "Fehler beim Laden des Bildes!" << std::endl;
        return {};
    }

    // Erstelle das RGB-Array mit der Größe 64x64
    std::vector<RGB> image(SCREEN_WIDTH * SCREEN_HEIGHT, {0, 0, 0}); // Initialisiert mit schwarz

    // Berechne Skalierung des Bildes
    float scaleX = static_cast<float>(width) / SCREEN_WIDTH;
    float scaleY = static_cast<float>(height) / SCREEN_HEIGHT;

    // Bild auf die Matrixgröße skalieren (64x64)
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            int imgX = static_cast<int>(x * scaleX);
            int imgY = static_cast<int>(y * scaleY);

            int indexInImage = (imgY * width + imgX) * 3;
            int r = imgData[indexInImage];
            int g = imgData[indexInImage + 1];
            int b = imgData[indexInImage + 2];

            image[y * SCREEN_WIDTH + x] = {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)};
        }
    }

    stbi_image_free(imgData);  // Gib den Speicher nach dem Laden des Bildes frei
    return image;
}

int main() {
    std::string imagePath;
    int screenNumber;

    // Benutzer nach Bilddateipfad und Bildschirmnummer fragen
    std::cout << "Gib den Pfad zum Bild ein (z.B. /path/to/your/image.jpg): ";
    std::getline(std::cin, imagePath);

    std::cout << "Gib die Bildschirmnummer ein, auf dem das Bild eingefügt werden soll (0-5): ";
    std::cin >> screenNumber;

    if (screenNumber < 0 || screenNumber >= NUM_SCREENS) {
        std::cerr << "Ungültige Bildschirmnummer! Es müssen Zahlen von 0 bis " << NUM_SCREENS - 1 << " sein." << std::endl;
        return -1;
    }

    // Bild einlesen und auf 64x64 Pixel skalieren
    std::vector<RGB> image = loadImage(imagePath);  // Der Pfad zum Bild

    if (image.empty()) {
        return -1;  // Fehler beim Laden des Bildes
    }

    // Bild in den angegebenen Bildschirm einfügen
    insertImage(image, screenNumber);

    // Konvertiere das RGB-Array in das für die Matrix-Bibliothek erforderliche Format
    std::vector<uint32_t> matrixData(ARRAY_WIDTH * SCREEN_HEIGHT);
    for (int i = 0; i < ARRAY_WIDTH * SCREEN_HEIGHT; ++i) {
        matrixData[i] = rgbToUint32(screens[i]);  // RGB in uint32_t umwandeln
    }

    // Matrix konfigurieren (dies ist abhängig von deiner Setup-Konfiguration)
    int rows = 64;
    int cols = 64 * NUM_SCREENS; // Breite für alle Bildschirme in einer Reihe
    int chainLength = 1; // Anzahl der verbundenen Matrices (hier 1)

    // RGBMatrix-Konfiguration
    RGBMatrix::Options options;
    options.rows = rows;
    options.cols = cols;
    options.chain_length = chainLength;
    options.hardware_mapping = "adafruit-hat";  // Angenommen du verwendest den Adafruit HAT
    options.brightness = 40;
    options.pixel_mapper_config = "U-mapper;Rotate:0";

    RGBMatrix::RGBMatrix* matrix = RGBMatrix::CreateMatrixFromOptions(options);

    if (matrix == nullptr) {
        std::cerr << "Fehler beim Erstellen der Matrix!" << std::endl;
        return -1;
    }

    // Unendliche Schleife, um die Matrix fortlaufend zu aktualisieren, bis mit ^C abgebrochen wird
    while (true) {
        // Die Matrix aktualisieren
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                int pixelIndex = y * cols + x;
                uint32_t pixel = matrixData[pixelIndex];

                // Setze den Pixelwert auf die Matrix
                matrix->SetPixel(x, y, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
            }
        }

        // Warte kurz, um die Matrix anzuzeigen, bevor der nächste Frame gezeichnet wird
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Matrix-Objekt aufräumen
    delete matrix;

    return 0;
}
