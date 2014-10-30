

// Rsa
#include "Definitions.h"
#include "General.h"
#include "FormattedTouchstone.h"
using namespace RsaToolbox;

// Qt
#include <QRegularExpression>
#include <QDebug>

// C++ std lib
#include <iterator>
#include <cmath>


// Actions
uint FormattedTouchstone::ports(QString fileName) {
    QRegularExpression FormattedTouchstone_REGEX(FormattedTouchstone_FILE_REGEX, QRegularExpression::CaseInsensitiveOption);
    if (!FormattedTouchstone_REGEX.match(fileName).hasMatch())
        return 0;

    // Read number of ports
    int dotPosition = fileName.lastIndexOf(".");
    fileName.remove(0, dotPosition + 2);
    fileName.chop(1);
    return(fileName.toUInt());
}
bool FormattedTouchstone::Read(NetworkData &network, QString filename) {
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return false;
    QTextStream snpFile(&file);
    network = NetworkData();
    if (!ReadPorts(network, filename)) {
        return false;
    }
    if (!ReadOptions(network, snpFile)) {
        return false;
    }
    if (!ReadData(network, snpFile)) {
        return false;
    }
    // Else
    return true;
}
bool FormattedTouchstone::Read(NetworkData &network, QTextStream &FormattedTouchstone_in, int ports) {
    if (ports < 1)
        return(false);

    network = NetworkData();
    return(ReadOptions(network, FormattedTouchstone_in)
            && ReadData(network, FormattedTouchstone_in));
}
bool FormattedTouchstone::Write(NetworkData &network, QString filename) {
    QFile file;
    CreateFile(file, filename, network);
    if (file.isWritable() == false) {
        if (file.isOpen()) file.close();
        return(false);
    }
    //else
    QTextStream snpFile(&file);
    WriteComments(network, snpFile);
    WriteOptions(network, snpFile);
    if (network.numberOfPorts() != 2)
        WriteData(network, snpFile);
    else {
        NetworkData copyNetwork = network;
        Flip2Ports(copyNetwork);
        WriteData(copyNetwork, snpFile);

    }
    snpFile.flush();
    file.close();
    return(true);
}
bool FormattedTouchstone::Write(NetworkData &network, QTextStream &FormattedTouchstone_out) {
    WriteComments(network, FormattedTouchstone_out);
    WriteOptions(network, FormattedTouchstone_out);
    if (network.numberOfPorts() != 2)
        WriteData(network, FormattedTouchstone_out);
    else {
        NetworkData copyNetwork = network;
        Flip2Ports(copyNetwork);
        WriteData(copyNetwork, FormattedTouchstone_out);
    }
    FormattedTouchstone_out.flush();
    return(true);
}

// Private

const QString FormattedTouchstone::FormattedTouchstone_FILE_REGEX = "^.*\\.s0*[1-9][0-9]*p$";
const int FormattedTouchstone::COLUMNWIDTH = 18;
const int FormattedTouchstone::PRECISION = 10;

// Fix 2port FormattedTouchstone issue
void FormattedTouchstone::Flip2Ports(NetworkData &network) {
    if (network.numberOfPorts() == 2) {
        const int PORT1 = 0;
        const int PORT2 = 1;
        for (unsigned int i = 0; i < network.points(); i++) {
            ComplexDouble Port1Port2;
            Port1Port2 = network.y()[i][PORT1][PORT2];
            network.y()[i][PORT1][PORT2] = network.y()[i][PORT2][PORT1];
            network.y()[i][PORT2][PORT1] = Port1Port2;
        }
    }
}

/* READ HELPER FUNCTIONS */

// Read/parse lines
bool FormattedTouchstone::ReadLine(QTextStream &snpFile, QStringList &words) {
    if (snpFile.atEnd())
        return false;

    QString line;
    line = snpFile.readLine();
    RemoveComment(line);
    words = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
    if (words.size() > 0)
        return(true);
    else
        return(ReadLine(snpFile, words));
}
void FormattedTouchstone::RemoveComment(QString &line) {
    int commentPosition = line.indexOf("!");
    if (commentPosition != -1)
        line.truncate(commentPosition);
}

// Read ports, options line
bool FormattedTouchstone::ReadPorts(NetworkData &network, QString filename) {
    network.setNumberOfPorts(ports(filename));
    return network.numberOfPorts() != 0;
}
bool FormattedTouchstone::ReadOptions(NetworkData &network, QTextStream &snpFile) {
    QStringList words;
    ReadLine(snpFile, words);
    if (words[0] != "#")
        return false;
    if (!ReadFrequencyPrefix(network, words[1])) {
        return false;
    }
    if (!ReadDataType(network, words[2])) {
        return false;
    }
    if (!ReadFormat(words[3])) {
        return false;
    }
    if (words.size() == 6)
         network.setReferenceImpedance(words.last().toDouble());
    // Else
    return true;
}
bool FormattedTouchstone::ReadFrequencyPrefix(NetworkData &network, QString units) {
    if (!units.contains("Hz", Qt::CaseInsensitive))
        return false;

    units.chop(2);
    network.setXUnits(HERTZ_UNITS, toSiPrefix(units));
    return(true);
}
bool FormattedTouchstone::ReadDataType(NetworkData &network, QString type) {
    QRegularExpression S_REGEX(toString(S_PARAMETER), QRegularExpression::CaseInsensitiveOption);
    QRegularExpression Y_REGEX(toString(Y_PARAMETER), QRegularExpression::CaseInsensitiveOption);
    QRegularExpression Z_REGEX(toString(Z_PARAMETER), QRegularExpression::CaseInsensitiveOption);
    QRegularExpression H_REGEX(toString(H_PARAMETER), QRegularExpression::CaseInsensitiveOption);
    QRegularExpression G_REGEX(toString(G_PARAMETER), QRegularExpression::CaseInsensitiveOption);

    if(type.size() == 1)
    {
        if (S_REGEX.match(type).hasMatch()) {
            network.setParameter(S_PARAMETER);
            return(true);
        }
        if (Y_REGEX.match(type).hasMatch()) {
            network.setParameter(Y_PARAMETER);
            return(true);
        }
        if (Z_REGEX.match(type).hasMatch()) {
            network.setParameter(Z_PARAMETER);
            return(true);
        }
        if (H_REGEX.match(type).hasMatch()) {
            network.setParameter(H_PARAMETER);
            return(true);
        }
        if (G_REGEX.match(type).hasMatch()) {
            network.setParameter(G_PARAMETER);
            return(true);
        }
    }

    // If all else fails
    return(false);
}
bool FormattedTouchstone::ReadFormat(QString format) {
    QRegularExpression RI_REGEX(toString(REAL_IMAGINARY_COMPLEX), QRegularExpression::CaseInsensitiveOption);
    QRegularExpression MA_REGEX(toString(MAGNITUDE_DEGREES_COMPLEX), QRegularExpression::CaseInsensitiveOption);
    QRegularExpression DB_REGEX(toString(DB_DEGREES_COMPLEX), QRegularExpression::CaseInsensitiveOption);

    if(format.length() != 2)
        return false;
    if (RI_REGEX.match(format).hasMatch()) {
        ReadDatum = &ReadRI;
        return(true);
    }
    if (MA_REGEX.match(format).hasMatch()) {
        ReadDatum = &ReadMA;
        return(true);
    }
    if (DB_REGEX.match(format).hasMatch()) {
        ReadDatum = &ReadDB;
        return(true);
    }
    // Else
    return false;
}

// Read data
bool FormattedTouchstone::ReadData(NetworkData &network, QTextStream &snpFile) {
    QRowVector freqs;
    ComplexMatrix3D data;
    while (!snpFile.atEnd()) {
        ComplexMatrix2D row;
        double freq;
        if (ReadRow(network, snpFile, row, freq)) {
            freqs << freq;
            data.push_back(row);
        }
    }

    uint ports = network.numberOfPorts();
    network.setData(freqs, data);
    if (network.numberOfPorts() != ports || network.points() == 0)
    return false;
    if (network.numberOfPorts() == 2)
        Flip2Ports(network);
    return(true);
}
bool FormattedTouchstone::ReadRow(NetworkData &network, QTextStream &snpFile, ComplexMatrix2D &dataRow, double &frequencyPoint) {
    // Begin to read data values
    double wordsToRead = pow(double(network.numberOfPorts()), 2) * 2 + 1;
    QStringList allWords;
    while (wordsToRead > 0 && !snpFile.atEnd()) {
        QStringList words;
        ReadLine(snpFile, words);
        wordsToRead -= words.size();
        allWords.append(words);
    }

    // Check to see if all data was read
    if (wordsToRead != 0) { return(false); }

    // Process data
    frequencyPoint = allWords[0].toDouble();
    QStringList::iterator wordIndex = allWords.begin() + 1;
    dataRow.resize(network.numberOfPorts());
    for (ComplexMatrix2D::iterator rowIndex = dataRow.begin(); rowIndex != dataRow.end(); rowIndex++) {
        (*rowIndex).resize(network.numberOfPorts());
        ComplexRowVector::iterator columnIndex = (*rowIndex).begin();
        for (; columnIndex != (*rowIndex).end(); columnIndex++) {
            *columnIndex = (*ReadDatum)(wordIndex->toDouble(), (wordIndex + 1)->toDouble());
            wordIndex += 2; }
    }
    return(true);
}
ComplexDouble (*FormattedTouchstone::ReadDatum)(double, double);
ComplexDouble FormattedTouchstone::ReadRI(double word1, double word2) {
    return (ComplexDouble(word1, word2));


}
ComplexDouble FormattedTouchstone::ReadMA(double word1, double word2) {
    double real = word1 * cos(word2 * PI/180);
    double imag = word1 * sin(word2 * PI/180);
    return(ComplexDouble(real, imag));
}
ComplexDouble FormattedTouchstone::ReadDB(double word1, double word2) {
    return(ReadMA(toMagnitude(word1), word2));
}


/* WRITE HELPER FUNCTIONS */

void FormattedTouchstone::CreateFile(QFile &file, QString filename, NetworkData &network) {
    QRegularExpression FormattedTouchstone_REGEX(FormattedTouchstone_FILE_REGEX, QRegularExpression::CaseInsensitiveOption);
    if (FormattedTouchstone_REGEX.match(filename).hasMatch()) {
        // Chop off file extension
        int dotPosition = filename.lastIndexOf(".");
        filename.truncate(dotPosition);
    }
    filename = filename + ".s" + QString::number(network.numberOfPorts()) + "p";
    file.setFileName(filename);
    file.open(QFile::WriteOnly);
}
void FormattedTouchstone::WriteComments(NetworkData &network, QTextStream &snpFile) {
    snpFile << "! RsaToolbox (C) 2014 Rohde & Schwarz America" << endl;
    snpFile << "! " << endl;
    snpFile << "! Number of points: " << network.points() << endl;
    snpFile << "! Number of ports: " << network.numberOfPorts() << endl;
    snpFile << "! Balanced ports?: " << "No" << endl;
    snpFile << "! " << endl << "! " << endl;
}

// Write Options + helpers
void FormattedTouchstone::WriteOptions(NetworkData &network, QTextStream &snpFile) {
    // Write options info
    snpFile << "# ";
    snpFile << WriteUnits(network) << " ";
    snpFile << WriteDataType(network) << " ";
    snpFile << WriteFormat(network) << " ";
    // snpFile << "R " << network.impedance << endl;
    snpFile << "R " << 50 << endl;
}
QString FormattedTouchstone::WriteUnits(NetworkData &network) {
    return(toString(network.xPrefix(), network.xUnits()));
}
QString FormattedTouchstone::WriteDataType(NetworkData &network) {
    return(toString(network.parameter()));
}
QString FormattedTouchstone::WriteFormat(NetworkData &network) {
    Q_UNUSED(network);
    return(toString(REAL_IMAGINARY_COMPLEX));
}

// Write data + helpers
void FormattedTouchstone::WriteData(NetworkData &network, QTextStream &snpFile) {
    // Choose data format
    GetWriteFormat(network);

    snpFile.setRealNumberPrecision(PRECISION);
    for (uint currentFreq = 0; currentFreq < network.points(); currentFreq++) {
        snpFile.setFieldAlignment(QTextStream::AlignLeft);
        snpFile.setFieldWidth(COLUMNWIDTH);
        snpFile << network.x()[currentFreq];
        ComplexMatrix2D::iterator row_iter = network.y()[currentFreq].begin();
        WriteRow(network, snpFile, *row_iter);
        row_iter++;
        for (; row_iter != network.y()[currentFreq].end(); row_iter++) {
            snpFile.setFieldAlignment(QTextStream::AlignLeft);
            snpFile.setFieldWidth(COLUMNWIDTH);
            snpFile << " "; // tab past frequency column for subsequent rows
            WriteRow(network, snpFile, *row_iter);
        }
    }
}
void FormattedTouchstone::WriteRow(NetworkData &network, QTextStream &snpFile, ComplexRowVector &row) {
    int columnsWritten = 1;
    int const COLUMNSPERLINE = 4;
    ComplexRowVector::iterator column_iter = row.begin();
    for (; column_iter != row.end(); column_iter++) {
        (*WriteDatum)(snpFile, *column_iter);
        // add delimiter
        if (columnsWritten == network.numberOfPorts()) {
            snpFile.setFieldWidth(0);
            snpFile << endl;
            snpFile.setFieldWidth(COLUMNWIDTH);
        }
        else if (columnsWritten % COLUMNSPERLINE == 0) {
            snpFile.setFieldWidth(0);
            snpFile << endl;
            snpFile.setFieldAlignment(QTextStream::AlignLeft);
            snpFile.setFieldWidth(COLUMNWIDTH);
            snpFile << " ";
        }
        columnsWritten++;
    }
}
void FormattedTouchstone::GetWriteFormat(NetworkData &network) {
    Q_UNUSED(network);
    WriteDatum = &WriteRI;
    //    switch (network.format) {
    //    case REAL_IMAGINARY_COMPLEX:
    //        WriteDatum = &WriteRI;
    //        break;
    //    case MAGNITUDE_DEGREES_COMPLEX:
    //        WriteDatum = &WriteMA;
    //        break;
    //    case DB_DEGREES_COMPLEX:
    //        WriteDatum = &WriteDB;
    //        break;
    //    }
}
void (*FormattedTouchstone::WriteDatum)(QTextStream &, ComplexDouble &);
void FormattedTouchstone::WriteRI(QTextStream &snpFile, ComplexDouble &datum) {
    snpFile.setFieldAlignment(QTextStream::AlignLeft);
    snpFile.setFieldWidth(COLUMNWIDTH);
    snpFile << datum.real();
    snpFile << datum.imag();
}
void FormattedTouchstone::WriteMA(QTextStream &snpFile, ComplexDouble &datum) {
    snpFile.setFieldAlignment(QTextStream::AlignLeft);
    snpFile.setFieldWidth(COLUMNWIDTH);
    snpFile << abs(datum);
    snpFile << arg(datum) * 180 / PI;
}
void FormattedTouchstone::WriteDB(QTextStream &snpFile, ComplexDouble &datum) {
    snpFile.setFieldAlignment(QTextStream::AlignLeft);
    snpFile.setFieldWidth(COLUMNWIDTH);
    snpFile << toDb(datum);
    snpFile << arg(datum) * 180 / PI;
}



