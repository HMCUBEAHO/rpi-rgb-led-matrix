#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <QApplication>
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QColor>
#include <QFileDialog>
#include <QImage>
#include <QPushButton>
#include <QVector>
#include <QHeaderView>
#include <QDebug>
#include <QFile>
#include <QTextStream>

class PixelGridWidget : public QWidget {
public:
    PixelGridWidget(QWidget* parent = nullptr) : QWidget(parent) {
        // Layout für das gesamte Pixel-Grid
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Erstelle ein TableWidget mit 64 Zeilen und 384 Spalten
        tableWidget = new QTableWidget(64, 384, this);

        // Setze die Zellgröße (kleinste darstellbare Größe)
        int cellSize = 1;  // Die Pixel können auch etwas größer dargestellt werden
        for (int row = 0; row < 64; ++row) {
            tableWidget->setRowHeight(row, cellSize);
        }
        for (int col = 0; col < 384; ++col) {
            tableWidget->setColumnWidth(col, cellSize);
        }

        // Entferne Header
        tableWidget->horizontalHeader()->setVisible(false);
        tableWidget->verticalHeader()->setVisible(false);

        // Füge einen Button hinzu, um ein Bild zu laden
        QPushButton* loadImageButton = new QPushButton("Bild laden", this);
        connect(loadImageButton, &QPushButton::clicked, this, &PixelGridWidget::loadImage);

        // Füge einen Button hinzu, um die RGB-Werte als Array in eine Datei zu speichern
        QPushButton* saveArrayButton = new QPushButton("RGB-Werte als Array speichern", this);
        connect(saveArrayButton, &QPushButton::clicked, this, &PixelGridWidget::saveRGBValuesAsArray);

        // Füge das TableWidget und die Buttons dem Layout hinzu
        mainLayout->addWidget(tableWidget);
        mainLayout->addWidget(loadImageButton);
        mainLayout->addWidget(saveArrayButton);

        setLayout(mainLayout);
    }

private:
    QTableWidget* tableWidget;

    void loadImage() {
        // Öffne den Datei-Dialog, um ein Bild auszuwählen
        QString fileName = QFileDialog::getOpenFileName(this, "Bild laden", "", "Bilder (*.png *.jpg *.bmp *.jpeg)");
        if (!fileName.isEmpty()) {
            // Lade und verarbeite das Bild
            QImage image = loadAndScaleImage(fileName.toStdString().c_str(), 384, 64);  // Skaliere auf 384x64
            if (!image.isNull()) {
                // Zeige das Bild im QTableWidget an
                displayImageInTable(image);
            }
        }
    }

    QImage loadAndScaleImage(const char* filename, int width, int height) {
        int originalWidth, originalHeight, channels;

        // Lade das Bild mit stb_image
        unsigned char* imageData = stbi_load(filename, &originalWidth, &originalHeight, &channels, 3);
        if (!imageData) {
            qDebug() << "Fehler beim Laden des Bildes!";
            return QImage();
        }

        // Speicher für das skalierte Bild
        QImage scaledImage(width, height, QImage::Format_RGB888);

        // Skalierung des Bildes auf 384x64 (Naive Methode - lineare Interpolation)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Berechne den Original-Pixel-Index
                int originalX = (x * originalWidth) / width;
                int originalY = (y * originalHeight) / height;

                // Hole den RGB-Wert des Originalbildes
                unsigned char r = imageData[(originalY * originalWidth + originalX) * 3 + 0];
                unsigned char g = imageData[(originalY * originalWidth + originalX) * 3 + 1];
                unsigned char b = imageData[(originalY * originalWidth + originalX) * 3 + 2];

                // Setze den RGB-Wert im skalieren Bild
                scaledImage.setPixel(x, y, qRgb(r, g, b));
            }
        }

        // Bilddaten freigeben
        stbi_image_free(imageData);

        return scaledImage;
    }

    void displayImageInTable(const QImage& image) {
        // Skalierte Bilddaten in das TableWidget übertragen
        for (int row = 0; row < 64; ++row) {
            for (int col = 0; col < 384; ++col) {
                QColor color = QColor(image.pixel(col, row));
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setBackgroundColor(color);
                tableWidget->setItem(row, col, item);
            }
        }
    }

    void saveRGBValuesAsArray() {
        // Öffne den Dialog, um den Speicherort der Textdatei auszuwählen
        QString fileName = QFileDialog::getSaveFileName(this, "RGB-Werte als Array speichern", "", "Textdateien (*.txt)");
        if (fileName.isEmpty()) {
            return;
        }

        // Öffne die Datei zum Schreiben
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Fehler beim Öffnen der Datei!";
            return;
        }

        QTextStream out(&file);

        // Beginne mit der Array-Deklaration
        out << "uint8_t matrix[64][384][3] = {\n";

        // Schreibe jede Zeile des Arrays
        for (int row = 0; row < 64; ++row) {
            out << "    {";
            for (int col = 0; col < 384; ++col) {
                QColor color = tableWidget->item(row, col)->backgroundColor();
                int r = color.red();
                int g = color.green();
                int b = color.blue();

                // Schreibe den RGB-Wert im Format {r, g, b}
                out << "{" << r << ", " << g << ", " << b << "}";

                // Füge ein Komma nach jedem Pixel hinzu, außer nach dem letzten
                if (col < 383) {
                    out << ", ";
                }
            }
            out << "},\n";
        }

        // Schließe das Array ab
        out << "};\n";

        // Datei schließen
        file.close();
        qDebug() << "RGB-Werte als Array in Datei gespeichert.";
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    PixelGridWidget widget;
    widget.resize(800, 200);  // Fenstergröße für das 64x384 Pixel-Array
    widget.show();

    return app.exec();
}
