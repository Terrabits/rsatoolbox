

// RsaToolbox
#include "Vna.h"
using namespace RsaToolbox;

// Qt
#include <QCoreApplication>
#include <QDir>


void main()
{
    Vna vna(ConnectionType::VisaTcpConnection, "127.0.0.1");

    // Start from instrument preset
    vna.preset();
    vna.pause();

    // Get diagram 1 interface
    VnaDiagram d1 = vna.diagram(1);

    // Set diagram title
    d1.setTitle("My Measurement");

    // Set min, max
    d1.setYAxis(-40., 40.0);

    // Save screenshot on Vna
    // Default location:
    //   C:\Users\Public\Documents\Rohde-Schwarz\Vna
    // Formats:
    //   Bmp
    //   Jpg
    //   Png
    //   Pdf
    //   Svg
//    d1.saveScreenshot("screenshot_d1.png", ImageFormat::Png);

    // Save screenshot locally
    // (save, transfer to PC)
    QDir src(SOURCE_DIR);
    d1.saveScreenshotLocally(src.filePath("local_screenshot_d1.jpg"), ImageFormat::Jpg);

    // Create new diagram
    // with next index
    uint nextIndex = vna.createDiagram();

    // Create diagram
    // with specific index
    vna.createDiagram(3);

    // Get channels, traces
    // displayed in diagram 1
    d1.channels(); // => QVector<uint>
    d1.traces();   // => QStringList
}
