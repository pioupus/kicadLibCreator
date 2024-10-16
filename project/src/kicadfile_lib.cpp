#include "kicadfile_lib.h"
#include "sexpresso/sexpresso/sexpresso.hpp"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QLineF>
#include <QPainterPath>
#include <QTextStream>
#include <QtMath>
#include <vector>

const static auto COLOR_FOREGROUND = QColor(153, 0, 0);
const static auto COLOR_BACKGROUND = QColor(255, 255, 204);

QStringList splitParams(const QString &params) {
    QStringList list;
    QString arg;
    enum { Idle, Arg, QuotedArg } state = Idle;
    foreach (QChar const c, params) {
        switch (state) {
            case Idle:
                if (c == '"') {
                    state = QuotedArg;
                } else if (!c.isSpace()) {
                    arg += c;
                    state = Arg;
                }
                break;
            case Arg:
                if (c == '"') {
                    state = QuotedArg;
                } else if (!c.isSpace()) {
                    arg += c;
                } else {
                    list << arg;
                    arg.clear();
                    state = Idle;
                }
                break;
            case QuotedArg:
                if (c == '"') {
                    // state = arg.isEmpty() ? Idle : Arg;
                    state = Arg;
                } else {
                    arg += c;
                }
                break;
        }
    }
    list << arg;
    // qDebug() << list;
    return list;
}

KicadSymbolFieldJustify_t decodeJustify(QChar ch) {
    KicadSymbolFieldJustify_t justify = ksfj_left;
    if (ch == 'L') {
        justify = ksfj_left;
    } else if (ch == 'R') {
        justify = ksfj_right;
    } else if (ch == 'C') {
        justify = ksfj_center;
    } else if (ch == 'B') {
        justify = ksfj_bottom;
    } else if (ch == 'T') {
        justify = ksfj_top;
    }
    return justify;
};
#if 0

KICADLibFieldIndex::KICADLibFieldIndex() {
    rawIndex = 0;
}

void KICADLibFieldIndex::setRawIndex(int rawindex) {
    this->rawIndex = rawindex;
}

int KICADLibFieldIndex::getRawIndex() {
    return rawIndex;
}

QString KICADLibFieldIndex::getFieldIndexDescription() {
    switch (rawIndex) {
        case 1:
            return "value";
            break;
        case 0:
            return "reference";
            break;
        case 2:
            return "footprint";
            break;
        case 3:
            return "datasheet";
            break;
    }
    return "";
}

KICADLibSchematicDeviceDefinition::KICADLibSchematicDeviceDefinition() {}

void KICADLibSchematicDeviceDefinition::decode(QString str) {
    QStringList params = splitParams(str);
    name = params[1];
    reference = params[2];
    text_offset = params[4].toInt();
    drawPinNumber = params[5] == "Y";
    drawPinName = params[6] == "Y";
    unitCount = params[7].toInt();
    unitLocked = params[8] == "L";
    optionFlag_IsPowersymbol = params[9] == "P";
}

QString KICADLibSchematicDeviceDefinition::encode() {
    QString result;
    result = "DEF ";
    result += name + " ";
    result += reference + " 0 "; // the zero is field "unused"
    result += QString::number(text_offset) + " ";
    if (drawPinNumber) {
        result += "Y ";
    } else {
        result += "N ";
    }

    if (drawPinName) {
        result += "Y ";
    } else {
        result += "N ";
    }

    result += QString::number(unitCount) + " ";

    if (unitLocked) {
        result += "L ";
    } else {
        result += "F ";
    }

    if (optionFlag_IsPowersymbol) {
        result += "P";
    } else {
        result += "N";
    }
    return result;
}

void KICADLibSchematicDeviceDefinition::clear() {
    name = "";
    reference = "";
    text_offset = 0;
    drawPinNumber = false;
    drawPinName = false;
    unitCount = 0;
    unitLocked = false;
    optionFlag_IsPowersymbol = false;
}

bool KICADLibSchematicDeviceDefinition::isValid() {
    bool result = true;
    if (name == "") {
        result = false;
    }

    if (reference == "") {
        result = false;
    }
    return result;
}

KICADLibSchematicDeviceField::KICADLibSchematicDeviceField() {
    clear();
}

void KICADLibSchematicDeviceField::decode(QString str) {
    QStringList params = splitParams(str);
    QString fi = params[0];
    fieldIndex.setRawIndex(fi.right(1).toInt());
    text = params[1];
    position.setX(params[2].toInt());
    position.setY(params[3].toInt());
    dimension = params[4].toInt();
    if (params[5].toUpper() == "H") {
        orientation = ksfo_horizontal;
    } else {
        orientation = ksfo_vertical;
    }

    visible = params[6] == "V";
    if (7 < params.count()) {
        QString hjus = params[7].toUpper();
        hjustify = decodeJustify(hjus[0]);
    } else {
        qDebug() << "field not correct. defaulted.";
        hjustify = ksfj_left;
    }
    if (8 < params.count()) {
        QString para = params[8].toUpper();
        vjustify = decodeJustify(para[0]);

        FontstyleItalic = false;
        FontstyleBold = false;
        if (para[1] == 'I') {
            FontstyleItalic = true;
        }
        if (para[2] == 'B') {
            FontstyleBold = true;
        }
    } else {
        qDebug() << "field not correct. defaulted.";
        vjustify = ksfj_top;
        FontstyleItalic = false;
        FontstyleBold = false;
    }

    if (9 < params.count()) {
        name = params[9];
    } else {
        name = "";
    }
}

QString KICADLibSchematicDeviceField::encode() {
    auto encodeJustify = [](KicadSymbolFieldJustify_t justify) {
        QChar ch = '\0';
        if (justify == ksfj_left) {
            ch = 'L';
        } else if (justify == ksfj_right) {
            ch = 'R';
        } else if (justify == ksfj_center) {
            ch = 'C';
        } else if (justify == ksfj_bottom) {
            ch = 'B';
        } else if (justify == ksfj_top) {
            ch = 'T';
        }
        return ch;
    };

    QString result = "F";
    result += QString::number(fieldIndex.getRawIndex()) + " ";

    result += "\"" + text + "\" ";
    result += QString::number(position.x()) + " ";
    result += QString::number(position.y()) + " ";
    result += QString::number(dimension) + " ";
    if (orientation == ksfo_horizontal) {
        result += "H ";
    } else {
        result += "V ";
    }

    if (visible) {
        result += "V ";
    } else {
        result += "I ";
    }
    result += QString(encodeJustify(hjustify)) + " ";
    result += encodeJustify(vjustify);
    if (FontstyleItalic) {
        result += "I";
    } else {
        result += "N";
    }

    if (FontstyleBold) {
        result += "B ";
    } else {
        result += "N ";
    }

    result += "\"" + name + "\"";
    return result;
}

void KICADLibSchematicDeviceField::clear() {
    fieldIndex.setRawIndex(0);
    text = "";
    position = QPoint(0, 0);
    dimension = 1;
    orientation = ksfo_horizontal;
    visible = false;
    hjustify = ksfj_left;
    vjustify = ksfj_top;
    FontstyleItalic = false;
    FontstyleBold = false;
    name = "";
}

void KICADLibSchematicDeviceField::setDesign(FieldDesignSettingsItem designItem, bool overwriteTextPosition) {
    if (overwriteTextPosition) {
        position = designItem.position;
        hjustify = designItem.hjustify;
        vjustify = designItem.vjustify;
        orientation = designItem.orientation;
    }
    dimension = designItem.dimension;
    visible = designItem.visible;

    FontstyleItalic = designItem.FontstyleItalic;
    FontstyleBold = designItem.FontstyleBold;
}

bool KICADLibSchematicDeviceField::operator<(const KICADLibSchematicDeviceField &R) const {
    return fieldIndex.rawIndex < R.fieldIndex.rawIndex;
}

bool KICADLibSchematicDeviceField::operator>(const KICADLibSchematicDeviceField &R) const {
    return fieldIndex.rawIndex > R.fieldIndex.rawIndex;
}
#endif
sKICADLibSchematicDeviceChildUnit::sKICADLibSchematicDeviceChildUnit(const sexpresso::Sexp &from) {
    /*
    (symbol
     "UNIT_ID"
     SYMBOL_PROPERTIES...
     GRAPHIC_ITEMS...
     PINS...
     UNITS...
         [(unit_name "UNIT_NAME")]
     )
     */

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "symbol");
    unit_id = QString::fromStdString(from.value.sexp[1].getString());
    for (unsigned int i = 2; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "unit_name") {
            unit_name = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
        } else if (node_type == "arc") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolArc>(sexp_lib_node));
        } else if (node_type == "circle") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCircle>(sexp_lib_node));
        } else if (node_type == "bezier") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCurve>(sexp_lib_node));
        } else if (node_type == "polyline") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolLine>(sexp_lib_node));
        } else if (node_type == "rectangle") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolRectangle>(sexp_lib_node));
        } else if (node_type == "text") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolText>(sexp_lib_node));
        } else if (node_type == "pin") {
            auto sym = (sexp_lib_node);
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolPin>(sexp_lib_node));
        } else {
            assert(0);
        }
    }
}

sKICADLibSchematicDeviceChildUnit sKICADLibSchematicDeviceChildUnit::operator=(const sKICADLibSchematicDeviceChildUnit &from) {
    return sKICADLibSchematicDeviceChildUnit(from);
}

sKICADLibSchematicDeviceChildUnit::sKICADLibSchematicDeviceChildUnit(const sKICADLibSchematicDeviceChildUnit &from) {
    for (const auto &v : from.sexp_graphical_symbols) {
        const auto *arc = dynamic_cast<sKicadGraphSymbolArc *>(v.get());
        const auto *circle = dynamic_cast<sKicadGraphSymbolCircle *>(v.get());
        const auto *curve = dynamic_cast<sKicadGraphSymbolCurve *>(v.get());
        const auto *line = dynamic_cast<sKicadGraphSymbolLine *>(v.get());
        const auto *rect = dynamic_cast<sKicadGraphSymbolRectangle *>(v.get());
        const auto *text = dynamic_cast<sKicadGraphSymbolText *>(v.get());
        const auto *pin = dynamic_cast<sKicadGraphSymbolPin *>(v.get());

        if (arc) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolArc>(*arc));
        } else if (circle) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCircle>(*circle));
        } else if (curve) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCurve>(*curve));
        } else if (line) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolLine>(*line));
        } else if (rect) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolRectangle>(*rect));
        } else if (text) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolText>(*text));
        } else if (pin) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolPin>(*pin));
        } else {
            assert(0);
        }

        (void)v;
    }
    unit_id = from.unit_id;
    unit_name = from.unit_name;
}

sexpresso::Sexp sKICADLibSchematicDeviceChildUnit::to_sexp() const {
    /*
    (symbol
     "UNIT_ID"

     GRAPHIC_ITEMS...
     PINS...
     UNITS...
         [(unit_name "UNIT_NAME")]
     )
     */
    sexpresso::Sexp sexp{};
    sexp.addChild("symbol \"" + unit_id.toStdString() + "\"");

    // sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("unit_name"), "\"" + unit_id.toStdString() + "\""}));

    for (const auto &da : sexp_graphical_symbols) {
        sexp.addChild(da->to_sexp());
    }

    return sexp;
}

KICADLibSchematicDevice::KICADLibSchematicDevice() {}

KICADLibSchematicDevice::KICADLibSchematicDevice(const sexpresso::Sexp &from) {
    /*
    (symbol
     "LIBRARY_ID" | "UNIT_ID"
     [(extends "LIBRARY_ID")]
     [(pin_numbers hide)]
     [(pin_names [(offset OFFSET)] hide)]
     (in_bom yes | no)
     (on_board yes | no)
     SYMBOL_PROPERTIES...
     GRAPHIC_ITEMS...
     PINS...
     UNITS...
         [(unit_name "UNIT_NAME")]
     )
     */
    sexp_on_board = true;
    sexp_in_bom = true;
    sexp_pin_numbers_hide = false;
    sexp_exclude_from_sim = false;
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "symbol");
    symbol_name = QString::fromStdString(from.value.sexp[1].getString());
    for (unsigned int i = 2; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "extends") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            sexp_extend = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
        } else if (node_type == "pin_numbers") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            if (sexp_lib_node.value.sexp[1].getString() == "hide") {
                sexp_pin_numbers_hide = true;
            }
        } else if (node_type == "exclude_from_sim") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            if (sexp_lib_node.value.sexp[1].getString() == "yes") {
                sexp_exclude_from_sim = true;
            } else if (sexp_lib_node.value.sexp[1].getString() == "no") {
                sexp_exclude_from_sim = false;
            } else {
                assert(0);
            }
        } else if (node_type == "pin_names") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            sexp_pin_names = sKicadPinName(sexp_lib_node);

        } else if (node_type == "in_bom") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            if (sexp_lib_node.value.sexp[1].getString() == "yes") {
                sexp_in_bom = true;
            } else if (sexp_lib_node.value.sexp[1].getString() == "no") {
                sexp_in_bom = false;
            } else {
                assert(0);
            }
        } else if (node_type == "on_board") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            if (sexp_lib_node.value.sexp[1].getString() == "yes") {
                sexp_on_board = true;
            } else if (sexp_lib_node.value.sexp[1].getString() == "no") {
                sexp_on_board = false;
            } else {
                assert(0);
            }

        } else if (node_type == "property") {
            auto property_name = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());

            property_name = property_name.toLower();
            property_name.front() = property_name.front().toUpper(); //lower case except first chararcter
            sexp_properties.insert(property_name, sSymbolProperty(sexp_lib_node));
        } else if (node_type == "arc") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolArc>(sexp_lib_node));
        } else if (node_type == "circle") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCircle>(sexp_lib_node));
        } else if (node_type == "bezier") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCurve>(sexp_lib_node));
        } else if (node_type == "polyline") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolLine>(sexp_lib_node));
        } else if (node_type == "rectangle") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolRectangle>(sexp_lib_node));
        } else if (node_type == "text") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolText>(sexp_lib_node));
        } else if (node_type == "pin") {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolPin>(sexp_lib_node));
        } else if (node_type == "symbol") { //defines unit
            auto unit = sKICADLibSchematicDeviceChildUnit(sexp_lib_node);
            sexp_units.append(unit);
        } else {
            assert(0);
        }
    }
}

KICADLibSchematicDevice::KICADLibSchematicDevice(const KICADLibSchematicDevice &from) {
    sexp_in_bom = from.sexp_in_bom;
    sexp_on_board = from.sexp_on_board;
    sexp_extend = from.sexp_extend;
    sexp_pin_numbers_hide = from.sexp_pin_numbers_hide;
    sexp_exclude_from_sim = from.sexp_exclude_from_sim;
    sexp_pin_names = from.sexp_pin_names;
    sexp_properties = from.sexp_properties;
    sexp_units = from.sexp_units;
    symbol_name = from.symbol_name;

    for (const auto &v : from.sexp_graphical_symbols) {
        const auto *arc = dynamic_cast<sKicadGraphSymbolArc *>(v.get());
        const auto *circle = dynamic_cast<sKicadGraphSymbolCircle *>(v.get());
        const auto *curve = dynamic_cast<sKicadGraphSymbolCurve *>(v.get());
        const auto *line = dynamic_cast<sKicadGraphSymbolLine *>(v.get());
        const auto *rect = dynamic_cast<sKicadGraphSymbolRectangle *>(v.get());
        const auto *text = dynamic_cast<sKicadGraphSymbolText *>(v.get());
        const auto *pin = dynamic_cast<sKicadGraphSymbolPin *>(v.get());

        if (arc) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolArc>(*arc));
        } else if (circle) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCircle>(*circle));
        } else if (curve) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCurve>(*curve));
        } else if (line) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolLine>(*line));
        } else if (rect) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolRectangle>(*rect));
        } else if (text) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolText>(*text));
        } else if (pin) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolPin>(*pin));
        } else {
            assert(0);
        }
    }
}

KICADLibSchematicDevice &KICADLibSchematicDevice::operator=(const KICADLibSchematicDevice &from) {
#if 0
    T& operator=(const T& other)
    {
        // Guard self assignment
        if (this == &other)
            return *this;
        
               // assume *this manages a reusable resource, such as a heap-allocated buffer mArray
        if (size != other.size)           // resource in *this cannot be reused
        {
            temp = new int[other.size];   // allocate resource, if throws, do nothing
            delete[] mArray;              // release resource in *this
            mArray = temp;
            size = other.size;
        } 
        
        std::copy(other.mArray, other.mArray + other.size, mArray);
        return *this;
    }
#endif
    if (this == &from)
        return *this;

    sexp_in_bom = from.sexp_in_bom;
    sexp_on_board = from.sexp_on_board;
    sexp_extend = from.sexp_extend;
    sexp_pin_numbers_hide = from.sexp_pin_numbers_hide;
    sexp_exclude_from_sim = from.sexp_exclude_from_sim;
    sexp_pin_names = from.sexp_pin_names;
    sexp_properties = from.sexp_properties;
    sexp_units = from.sexp_units;
    symbol_name = from.symbol_name;

    for (const auto &v : from.sexp_graphical_symbols) {
        const auto *arc = dynamic_cast<sKicadGraphSymbolArc *>(v.get());
        const auto *circle = dynamic_cast<sKicadGraphSymbolCircle *>(v.get());
        const auto *curve = dynamic_cast<sKicadGraphSymbolCurve *>(v.get());
        const auto *line = dynamic_cast<sKicadGraphSymbolLine *>(v.get());
        const auto *rect = dynamic_cast<sKicadGraphSymbolRectangle *>(v.get());
        const auto *text = dynamic_cast<sKicadGraphSymbolText *>(v.get());
        const auto *pin = dynamic_cast<sKicadGraphSymbolPin *>(v.get());

        if (arc) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolArc>(*arc));
        } else if (circle) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCircle>(*circle));
        } else if (curve) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolCurve>(*curve));
        } else if (line) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolLine>(*line));
        } else if (rect) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolRectangle>(*rect));
        } else if (text) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolText>(*text));
        } else if (pin) {
            sexp_graphical_symbols.push_back(std::make_unique<sKicadGraphSymbolPin>(*pin));
        } else {
            assert(0);
        }
    }

    return *this;
}

KICADLibSchematicDeviceLibrary::KICADLibSchematicDeviceLibrary() {}

void KICADLibSchematicDeviceLibrary::clear() {
    fileName = "";
    symbolList.clear();
}

#if 0
QString KICADLibDCMFile::getdcmFileNameFromLibFileName(QString libfileName) {
    QFileInfo fi(libfileName);
#if 0
    qDebug() << "orig:"<<libfileName;
    qDebug() << "baseName:"<<fi.baseName();
    qDebug() << "absolutePath:"<<fi.absolutePath();
    qDebug() << "bundleName:"<<fi.bundleName();
#endif
    return QString(fi.absolutePath()) + QDir().separator() + fi.baseName() + ".dcm";
}

void KICADLibDCMFile::loadFile(QString libfileName) {
    clear();
    QString dcmFileName = getdcmFileNameFromLibFileName(libfileName);

    if (QFileInfo::exists(dcmFileName)) {
        QFile dcmFile(dcmFileName);
        dcmFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream dcmStream(&dcmFile);
        KICADLibDCMEntry dcmEntry;
        while (!dcmStream.atEnd()) {
            QString line = dcmStream.readLine();

            // qDebug() << line;
            line = line.trimmed();
            QChar line_c = line[0];
            QString data = line.mid(1).trimmed();

            if (line.startsWith("$CMP")) {
                dcmEntry.name = line.split(" ")[1].trimmed();
                // qDebug() << "dcmEntry.name" << dcmEntry.name;
                dcmEntry.description = "";
                dcmEntry.keywords = "";
                dcmEntry.datasheetlink = "";
            } else if (line_c == 'D') {
                dcmEntry.description = data;
            } else if (line_c == 'K') {
                dcmEntry.keywords = data;
            } else if (line_c == 'F') {
                dcmEntry.datasheetlink = data;
            } else if (line.startsWith("$ENDCMP")) {
                dcmEntries.append(dcmEntry);
                componentNames.append(dcmEntry.name);
            }
        }
    }
}

KICADLibDCMEntry KICADLibDCMFile::getEntryByName(QString name, bool &ok) {
    KICADLibDCMEntry result;
    int index = componentNames.indexOf(name);
    if (index != -1) {
        result = dcmEntries[index];
        ok = true;
        // qDebug() << "found dcm";
    } else {
        ok = false;
        // qDebug() << "not found dcm " + name << componentNames.count();
    }
    return result;
}

void KICADLibDCMFile::clear() {
    componentNames.clear();
    dcmEntries.clear();
}
#endif

void KICADLibSchematicDeviceLibrary::loadFile(QString fileName) {
    enum class SECTION { none, def, field, fplist, draw, alias };
    clear();
    this->fileName = fileName;
    QFile libFile(fileName);
    libFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&libFile);
    symbolList.clear();

    if (fileName.endsWith("lib")) {
        assert(0);
    } else if (fileName.endsWith("kicad_sym")) {
        auto file_text = textStream.readAll();
        if (file_text.size() == 0) {
            file_text = R"foo((kicad_symbol_lib
    (version 20231120)
    (generator "kicad_libcreator")
    (generator_version "1.0")
)
")foo";
            file_text = "(kicad_symbol_lib\n\t(version 20231120)\n\t(generator \"kicad_libcreator\")\n\t(generator_version \"1.0\")\n)\n";
        }
        auto sexp_file = sexpresso::parse(file_text.toStdString());

        auto sexp_lib = sexp_file.getChildByPath("kicad_symbol_lib");
        assert(sexp_lib);
        if (sexp_lib->isSexp()) {
            //   qDebug() << sexp_lib->value.sexp.size();
            for (auto sexp_lib_node : sexp_lib->arguments()) {
                auto node_name = sexp_lib_node.value.sexp[0].getString();

                if (node_name == "version") {
                    lib_version = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
                }

                else if (node_name == "generator") {
                    generator = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
                }

                else if (node_name == "generator_version") {
                    generator_version = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
                }

                else if (node_name == "symbol") {
                    //   qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
                    symbolList.append(KICADLibSchematicDevice(sexp_lib_node));
                }
            }

        } else {
            qDebug() << "somesthing went wrong when loading s-expression:" << sexp_lib->value.str;
        }
    } else {
        assert(0);
    }
}

bool KICADLibSchematicDeviceLibrary::saveFile(QString fileName) {
#if 0
    QFile file(fileName);
    QFile fileDCM(KICADLibDCMFile::getdcmFileNameFromLibFileName(fileName));save

    bool filedcmok = fileDCM.open(QIODevice::WriteOnly | QIODevice::Truncate);

    bool result;
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate) && filedcmok) {
        result = true;
        QTextStream ts(&file);
        QTextStream dcms(&fileDCM);
        ts << "EESchema-LIBRARY Version 2.3" << Qt::endl;save
        dcms << "EESchema-DOCLIB  Version 2.0" << Qt::endl;
        for (int i = 0; i < symbolList.count(); i++) {
            ts << symbolList[i].def.encode() << Qt::endl;
            for (int n = 0; n < symbolList[i].fields.count(); n++) {
                ts << symbolList[i].fields.encode(n) << Qt::endl;
            }
            if (symbolList[i].fpList.count()) {
                ts << "$FPLIST" << Qt::endl;
                for (int n = 0; n < symbolList[i].fpList.count(); n++) {
                    ts << " " << symbolList[i].fpList[n] << Qt::endl;
                }
                ts << "$ENDFPLIST" << Qt::endl;
            }
            if (symbolList[i].alias.count()) {
                QString alias_str = "ALIAS";
                for (int n = 0; n < symbolList[i].alias.count(); n++) {
                    alias_str += " " + symbolList[i].alias[n];
                }
                ts << alias_str << Qt::endl;
            }
            ts << "DRAW" << Qt::endl;
            for (int n = 0; n < symbolList[i].drawSymbols.count(); n++) {
                ts << symbolList[i].drawSymbols[n].encode() << Qt::endl;
            }
            ts << "ENDDRAW" << Qt::endl;

            ts << "ENDDEF" << Qt::endl;

            if (symbolList[i].dcmEntry.hasFields()) {
                QString dcmName = symbolList[i].def.name;
                if (dcmName[0] == '~') {
                    dcmName = dcmName.mid(1);
                }
                dcms << "$CMP " + dcmName << Qt::endl;
                if (symbolList[i].dcmEntry.description.size()) {
                    dcms << "D " << symbolList[i].dcmEntry.description << Qt::endl;
                }
                if (symbolList[i].dcmEntry.keywords.size()) {
                    dcms << "K " << symbolList[i].dcmEntry.keywords << Qt::endl;
                }
                if (symbolList[i].dcmEntry.datasheetlink.size()) {
                    dcms << "F " << symbolList[i].dcmEntry.datasheetlink << Qt::endl;
                }
                dcms << "$ENDCMP" << Qt::endl;
            }
        }
    } else {
        result = false;
    }
    file.close();
    fileDCM.close();
    return result;
#else
    const char *locale = "C";
    std::locale::global(std::locale(locale));

    auto se = sexpresso::Sexp("kicad_symbol_lib");

    se.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("version"), lib_version.toStdString()}));

    se.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("generator"), "\"" + generator.toStdString() + "\""}));

    se.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("generator_version"), "\"" + generator_version.toStdString() + "\""}));

    for (const auto &v : symbolList) {
        se.addChild(v.to_sexp());
    }
    QFile file(fileName);

    auto sexp = sexpresso::Sexp{};
    sexp.addChild(se);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream ts(&file);
        ts << QString::fromStdString(sexp.toString());
    }
    file.close();
    return true;
#endif
}

QString KICADLibSchematicDeviceLibrary::getName() {
    QFileInfo fi(fileName);
    return fi.fileName();
}

const QList<KICADLibSchematicDevice> &KICADLibSchematicDeviceLibrary::getSymbolList() const {
    return symbolList;
}

int KICADLibSchematicDeviceLibrary::indexOf(QString deviceName) {
    int result = -1;
    for (int i = 0; i < symbolList.size(); i++) {
        if (symbolList[i].get_symbol_name() == deviceName) {
            result = i;
        }
    }
    return result;
}

void KICADLibSchematicDeviceLibrary::insertDevice(const KICADLibSchematicDevice &device) {
    int index = indexOf(device.get_symbol_name());
    if (index != -1) {
        symbolList.removeAt(index);
    }
    symbolList.append(device);
}

void KICADLibSchematicDevice::clear() {
    // fpList.clear();
    sexp_graphical_symbols.clear();
    //sexp_pins.clear();
}

bool KICADLibSchematicDevice::isValid() {
    bool result = true;

    //  if (def.isValid() == false) {
    //      result = false;
    //  }
    return result;
}
#if 0
KICADLibSchematicDeviceField KicadFieldList::getFieldbyIndex(int index) {
    KICADLibSchematicDeviceField result;

    for (int i = 0; i < fields.count(); i++) {
        if (fields[i].fieldIndex.getRawIndex() == index) {
            result = fields[i];
            break;
        }
    }
    return result;
}

int KicadFieldList::count() {
    return fields.count();
}FreeCAD.saveParameter()

void KicadFieldList::append(KICADLibSchematicDeviceField field) {
    fields.append(field);
}

QString KicadFieldList::encode(int index) {
    return fields[index].encode();
}

void KicadFieldList::clear() {
    fields.clear();
}

void KicadFieldList::removeAllAboveIndex(int index) {
    int i = 0;
    while (i < fields.count()) {
        if (fields[i].fieldIndex.getRawIndex() >= index) {
            fields.removeAt(i);
        } else {
            i++;
        }
    }
}

void KicadFieldList::setField(KICADLibSchematicDeviceField field) {
    bool found = false;
    int foundatIndex = 0;
    for (int i = 0; i < fields.count(); i++) {
        if (field.fieldIndex.getRawIndex() == fields[i].fieldIndex.getRawIndex()) {
            found = true;
            foundatIndex = i;
            break;
        }
    }
    if (found) {
        fields.removeAt(foundatIndex);
    }
    fields.append(field);
    std::sort(fields.begin(), fields.end(), std::less<KICADLibSchematicDeviceField>());
}

QList<std::unique_ptr<sKicadGraphSymbol>> KICADLibSchematicDevice::getDrawSymbols() {
    QList<std::unique_ptr<sKicadGraphSymbol>> result;
    result.append(sexp_graphical_symbols);
    for (auto &l : sexp_units) {
        result.append(l.sexp_graphical_symbols);
    }

    return result;
}
#endif
QString KICADLibSchematicDevice::get_property_value_str(QString key) const {
    key = key.toLower();
    key.front() = key.front().toUpper(); //lower case except first chararcter
    auto value = sexp_properties[key];
    return value.value;
}

sSymbolProperty KICADLibSchematicDevice::get_property_value(QString key) const {
    key = key.toLower();
    key.front() = key.front().toUpper(); //lower case except first chararcter
    return sexp_properties[key];
}

void KICADLibSchematicDevice::set_property_value(QString key, QString value) {
    key = key.toLower();
    key.front() = key.front().toUpper(); //lower case except first chararcter

    if (sexp_properties.contains(key)) {
        sexp_properties[key].value = value;
    } else {
        sSymbolProperty prop;
        prop.key = key;
        prop.value = value;
        sexp_properties.insert(key, prop);
    }
}

void KICADLibSchematicDevice::set_property_value(const sSymbolProperty &value) {
    QString key = value.key;
    key = key.toLower();
    key.front() = key.front().toUpper(); //lower case except first chararcter
    if (sexp_properties.contains(key)) {
        sexp_properties[key] = value;
    } else {
        sexp_properties.insert(key, value);
    }
}

void KICADLibSchematicDevice::set_symbol_name(QString name) {
    symbol_name = name;
}

QString KICADLibSchematicDevice::get_symbol_name() const {
    return symbol_name;
}

int KICADLibSchematicDevice::get_num_of_graphical_symbols() const {
    return sexp_graphical_symbols.size();
}

void KICADLibSchematicDevice::paint(QPainter &painter, const QPointF &centerPoint) const {
    qDebug() << "paint";
    for (const auto &da : sexp_graphical_symbols) {
        da->paint(painter, centerPoint, sexp_pin_names);
    }
    for (const auto &du : sexp_units) {
        for (const auto &da : du.sexp_graphical_symbols) {
            da->paint(painter, centerPoint, sexp_pin_names);
        }
    }
}

sexpresso::Sexp KICADLibSchematicDevice::to_sexp() const {
    /*
    (symbol
     "LIBRARY_ID" | "UNIT_ID"
         [(extends "LIBRARY_ID")]
         [(pin_numbers hide)]
         [(pin_names [(offset OFFSET)] hide)]
     (in_bom yes | no)
     (on_board yes | no)
     SYMBOL_PROPERTIES...
     GRAPHIC_ITEMS...
     PINS...
     UNITS...
         [(unit_name "UNIT_NAME")]
     )
     */
    sexpresso::Sexp sexp{};

    sexp.addChild("symbol \"" + symbol_name.toStdString() + "\"");

    if (sexp_extend.has_value()) {
        sexp.addChild(sexpresso::Sexp("extends \"" + sexp_extend.value().toStdString() + "\""));
    }
    if (sexp_pin_numbers_hide) {
        sexp.addChild(sexpresso::Sexp("pin_numbers hide"));
    }
    if (sexp_pin_names.has_value()) {
        sexp.addChild(sexp_pin_names.value().to_sexp());
    }
    if (!sexp_extend.has_value()) {
        auto in_bom_s = std::string(sexp_in_bom ? "yes" : "no");
        auto on_board_s = std::string(sexp_on_board ? "yes" : "no");
        sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("in_bom"), in_bom_s}));
        sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("on_board"), on_board_s}));
    }

    for (const sSymbolProperty &p : sexp_properties) {
        sexp.addChild(p.to_sexp());
    }
    if (!sexp_extend.has_value()) {
        //The optional extends token attribute defines the "LIBRARY_ID" of another symbol inside the
        //current library from which to derive a new symbol. Extended symbols currently can only have
        //different SYMBOL_PROPERTIES than their parent symbol.

        for (const auto &g : sexp_graphical_symbols) {
            sexp.addChild(g->to_sexp());
        }
        for (const auto &u : sexp_units) {
            sexp.addChild(u.to_sexp());
        }
    }
    return sexp;
}

FillType charToFillType(QString c) {
    FillType result;
    if (c[0] == QChar('F')) {
        result = FillType::foregroundColor;
    } else if (c[0] == QChar('f')) {
        result = FillType::backgroundColor;
    } else if (c[0] == QChar('N')) {
        result = FillType::none;
    } else {
        qDebug() << "filltype field not correct. defaulted.";
        result = FillType::none;
    }
    return result;
}
#if 0
KICADLibSchematicDrawElement::KICADLibSchematicDrawElement(QString str) {
    positions.clear();
    // orientation = U (up) D (down) R (right) L (left).
    auto decodeOrientation = [](QChar ch) {
        KicadSymbolOrientation_t orientation = ksr_none;
        if (ch == 'U') {
            orientation = ksr_up;
        } else if (ch == 'D') {
            orientation = ksr_down;
        } else if (ch == 'R') {
            orientation = ksr_right;
        } else if (ch == 'L') {
            orientation = ksr_left;
        }
        return orientation;
    };

    unit = 0;
    convert = 0;
    thickness = 0;

    originalText = str;
    QStringList params = splitParams(str);
    if (str[0] == 'P') {
        drawtype = DrawType::polygon;
        const int numberOfPoints = params[1].toInt();
        unit = params[2].toInt();
        convert = params[3].toInt();
        thickness = params[4].toInt();
        for (int i = 0; i < numberOfPoints; i++) {
            QPoint p;
            p.setX(params[5 + i * 2].toInt());
            p.setY(params[6 + i * 2].toInt());
            positions.append(p);
        }
        const int fillIndex = 5 + numberOfPoints * 2;
        if (fillIndex < params.count()) {
            cc_filled = charToFillType(params[fillIndex]);
        } else {
            qDebug() << "field not correct. defaulted.";
            cc_filled = FillType::none;
        }
    } else if (str[0] == 'S') {
        drawtype = DrawType::rectangle;
        start.setX(params[1].toInt());
        start.setY(params[2].toInt());
        end.setX(params[3].toInt());
        end.setY(params[4].toInt());
        unit = params[5].toInt();
        convert = params[6].toInt();
        if (7 < params.count()) {
            thickness = params[7].toInt();
        } else {
            thickness = 1;
            qDebug() << "field not correct. defaulted.";
        }
        if (8 < params.count()) {
            cc_filled = charToFillType(params[8]);
        } else {
            qDebug() << "field not correct. defaulted.";
            cc_filled = FillType::none;
        }
        /*
    * Rectangle
    Format:
    S startx starty endx endy unit convert thickness cc
    With
    • unit = 0 if common to the parts; if not, number of part (1. .n).
    • convert = 0if common to all parts. If not, number of the part (1. .n).
    • thickness = thickness of the outline.
    • cc = N F or F ( F = filled Rectangle,; f = . filled Rectangle, N = transparent
    background) Example: S 0 50.900.900 0 1 0 f
    */

    } else if (str[0] == 'C') {
        drawtype = DrawType::circle;
        QPoint p;
        p.setX(params[1].toInt());
        p.setY(params[2].toInt());
        positions.append(p);
        radius = params[3].toInt();
        unit = params[4].toInt();
        convert = params[5].toInt();

        if (6 < params.count()) {
            thickness = params[6].toInt();
        } else {
            thickness = 1;
            qDebug() << "thickness field not correct. defaulted.";
        }

        if (7 < params.count()) {
            cc_filled = charToFillType(params[7]);
        } else {
            qDebug() << "filltype not correct. defaulted.";
            cc_filled = FillType::none;
        }
    } else if (str[0] == 'A') {
        drawtype = DrawType::arc;
        QPoint p;
        p.setX(params[1].toInt());
        p.setY(params[2].toInt());
        positions.append(p);

        radius = params[3].toInt();

        angle_start = params[4].toInt() / 10.0;
        angle_end = params[5].toInt() / 10.0;

        unit = params[6].toInt();
        convert = params[7].toInt();

        thickness = params[8].toInt();

        cc_filled = FillType::none;
        if (params.length() > 9) {
            cc_filled = charToFillType(params[9]);
        }

        start = QPoint(0, 0);
        end = QPoint(0, 0);

        if (params.length() > 10) {
            start.setX(params[10].toInt());
        }
        if (params.length() > 11) {
            start.setY(params[11].toInt());
        }
        if (params.length() > 12) {
            end.setX(params[12].toInt());
        }
        if (params.length() > 13) {
            end.setY(params[13].toInt());
        }

    } else if (str[0] == 'T') {
        drawtype = DrawType::text;
        QPoint p;
        if (params[1] == "0") {
            textDirection = TextDirection::Horizontal;
        } else {
            textDirection = TextDirection::Vertical;
        }
        p.setX(params[2].toInt());
        p.setY(params[3].toInt());
        positions.append(p);
        textSize = params[4].toInt();
        // texttype??
        unit = params[6].toInt();
        convert = params[7].toInt();
        text = params[8];
        bool italic = false;
        bool bold = false;
        textStyle = TextStyle::normal;
        text_hjustify = ksfj_center;
        text_vjustify = ksfj_center;
        if (params.length() > 9) {
            if (params[9].toUpper() == "ITALIC") {
                italic = true;
            }
        }
        if (params.length() > 10) {
            if (params[10].toUpper() == "1") {
                bold = true;
            }
        }
        if (italic && bold) {
            textStyle = TextStyle::bold_italic;
        } else if (italic && !bold) {
            textStyle = TextStyle::italic;
        } else if (!italic && bold) {
            textStyle = TextStyle::bold;
        } else {
            textStyle = TextStyle::normal;
        }
        if (params.length() > 11) {
            text_hjustify = decodeJustify(params[11][0]);
        }
        if (params.length() > 12) {
            text_vjustify = decodeJustify(params[12][0]);
        }
    } else if (str[0] == 'X') {
        /*
         * 2.3.4 - Description of pins
        Format:
        X name number posx posy length orientation Snum Snom unit convert Etype
    [shape]. With: • orientation = U (up) D (down) R (right) L (left). • name = name
    (without space) of the pin. if ~: no name • number = n pin number (4 characters
    maximum). • length = pin length. • Snum = pin number text size. • Snom = pin
    name text size. • unit = 0 if common to all parts. If not, number of the part
    (1. .n). • convert = 0 if common to the representations, if not 1 or 2. • Etype
    = electric type (1 character) • shape = if present: pin shape (clock,
    inversion...). Example: X TO 1 - 200 0.150 R 40 40 1 1 P X K 2.200 0.150 L 40 40
    1 1 P X 0 1 0 0 0 R 40 40 1 1 W NC X ~ 2 0 - 250 200 U 40 40 1 1 P Etype list:
        INPUT           I
        OUTPUT          O
        BIDI            B
        TRISTATE        T
        PASSIVE         P
        UNSPECIFIED     U
        POWER INPUT     W
        POWER OUTPUT    w
        OPEN COLLECTOR  C
        OPEN EMITTER    E
        NOT CONNECTED   N

        Shape list:
        • If invisible pin, the shape identifier starts by N
        • Next character is:
        Line                None(default)
        Inverted            I
        Clock               C
        Inverted clock      CI
        Input low           L
        Clock low           CL
        Output low          V
        Falling edge clock  F
        Non Logic           X

        Example:
    A clock is coded C if visible, and NC if invisible.
    */

        drawtype = DrawType::pin;
        name = params[1];
        number = params[2].toInt();
        QPoint pos;
        pos.setX(params[3].toInt());
        pos.setY(params[4].toInt());
        positions.append(pos);
        length = params[5].toInt();
        orientation = decodeOrientation(params[6].toStdString()[0]);
        pinNumberTextSize = params[7].toInt();
        pinNameTextSize = params[8].toInt();
        unit = params[9].toInt();
        convert = params[10].toInt();
        etype.decode(params[11].toStdString()[0]);
        if (params.count() > 12) {
            shape.decode(params[12]);
        } else {
            shape.setShape(PShape::Line);
        }

    } else {
        drawtype = DrawType::none;
    }
}

QString KICADLibSchematicDrawElement::encode() {
    return originalText;
}
#endif
#if 0
KICADLibSchematicDrawElement *KICADLibSchematicDrawElement::getDrawSmybol()
{
    switch(getDrawType()){
    case DrawType::none:
        return NULL;
    case DrawType::arc:
        return new KICADLibSchematicDrawElement_arc{paramStr};

    case DrawType::circle:
        return new KICADLibSchematicDrawElement_circle{paramStr};
    case DrawType::pin:
        return new KICADLibSchematicDrawElement_pin{paramStr};

    case DrawType::polygon:

        return new KICADLibSchematicDrawElement_polygon{paramStr};
    case DrawType::rectangle:
        return  new KICADLibSchematicDrawElement_rectangle{paramStr};
    case DrawType::text:
        return new KICADLibSchematicDrawElement_text{paramStr};
    }

    return 0;
}

DrawType KICADLibSchematicDrawElement::getDrawType() {
    return drawtype;
}

void ElectricalType::decode(char ch) {
    switch (ch) {
        case 'I':
            t = sElectricalType::input;
            break;
        case 'O':
            t = sElectricalType::output;
            break;
        case 'B':
            t = sElectricalType::bidirectional;
            break;
        case 'T':
            t = sElectricalType::tri_state;
            break;
        case 'P':
            t = sElectricalType::passive;
            break;
        case 'U':
            t = sElectricalType::unspecified;
            break;
        case 'W':
            t = sElectricalType::power_in;
            break;
        case 'w':
            t = sElectricalType::power_out;
            break;
        case 'C':
            t = sElectricalType::open_collector;
            break;
        case 'E':
            t = sElectricalType::open_emitter;
            break;
        case 'N':
            t = sElectricalType::no_connect;
            break;
    }
}

sElectricalType ElectricalType::getType() {
    return t;
}

void PinShape::decode(QString str) {
    if (str == "") {
        s = PShape::Line;
    } else if (str == "I") {
        s = PShape::Inverted;
    } else if (str == "C") {
        s = PShape::Clock;
    } else if (str == "CI") {
        s = PShape::Inverted_clock;
    } else if (str == "L") {
        s = PShape::Input_low;
    } else if (str == "CL") {
        s = PShape::Clock_low;
    } else if (str == "V") {
        s = PShape::OutputLow;
    } else if (str == "F") {
        s = PShape::FallingEdgeClock;
    } else if (str == "X") {
        s = PShape::NonLocgic;
    } else {
        s = PShape::None;
    }
}

PShape PinShape::getShape() {
    return s;
}

void PinShape::setShape(PShape s) {
    this->s = s;
}
#endif

#if 0
KICADLibDCMEntry::KICADLibDCMEntry() {
    clear();
}

void KICADLibDCMEntry::clear() {
    name = "";
    description = "";
    keywords = "";
    datasheetlink = "";
}

bool KICADLibDCMEntry::hasFields() {
    return (description != "") || (keywords != "") || (datasheetlink != "");
}
#endif

KICADLibFootprintLibrary::KICADLibFootprintLibrary() {}

void KICADLibFootprintLibrary::scan(QString path) {
    this->path = path;
    QDirIterator it_root(this->path, QDir::Dirs, QDirIterator::NoIteratorFlags);

    footprintnames.clear();
    while (it_root.hasNext()) {
        QString name = it_root.next();
        if (name.endsWith(".pretty")) {
            QDirIterator it_fp(name, QDir::Files, QDirIterator::NoIteratorFlags);

            while (it_fp.hasNext()) {
                QString name_fp = it_fp.next();
                QFileInfo fi(name_fp);
                // qDebug() << fi.baseName();
                footprintnames.append(fi.baseName());
            }
        }
    }
    footprintnames.sort();
}

QStringList KICADLibFootprintLibrary::getFootprintList() {
    return footprintnames;
}

sKicadStrokeDefinition::sKicadStrokeDefinition() {
    width = 1;
    type = sKicadStrokeDefinition::solid;
    color = COLOR_FOREGROUND;
}

sKicadStrokeDefinition::sKicadStrokeDefinition(const sexpresso::Sexp &from) {
    /*
  (stroke
    (width WIDTH)
    (type TYPE)
    (color R G B A)
  )
)*/
    color = COLOR_FOREGROUND;
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "stroke");
    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];

        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "width") {
            qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            width = QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble();
        } else if (node_type == "type") {
            auto ts = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            if (ts == "dash") {
                type = dash;
            } else if (ts == "dash_dot") {
                type = dash_dot;
            } else if (ts == "dash_dot_dot") {
                type = dash_dot_dot;
            } else if (ts == "dot") {
                type = dot;
            } else if (ts == "default") {
                type = default_;
            } else if (ts == "solid") {
                type = solid;
            } else {
                assert(0);
            }
        } else if (node_type == "color") {
            color.setRed(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toInt());
            color.setGreen(QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toInt());
            color.setBlue(QString::fromStdString(sexp_lib_node.value.sexp[3].getString()).toInt());
            color.setAlpha(QString::fromStdString(sexp_lib_node.value.sexp[4].getString()).toInt());
        } else {
            assert(0);
        }
    }
}

QPen sKicadStrokeDefinition::get_pen() const {
    QPen result;

    result.setColor(color);
    result.setWidth(width);
    result.setJoinStyle(Qt::RoundJoin);
    result.setCapStyle(Qt::RoundCap);

    switch (type) {
        case TypeEnum::dash:
            result.setStyle(Qt::DashLine);
            break;
        case TypeEnum::dash_dot:
            result.setStyle(Qt::DashDotLine);
            break;
        case TypeEnum::dash_dot_dot:
            result.setStyle(Qt::DashDotDotLine);
            break;
        case TypeEnum::dot:
            result.setStyle(Qt::DashDotLine);
            break;
        case TypeEnum::default_:
            result.setStyle(Qt::SolidLine);
            break;
        case TypeEnum::solid:
            result.setStyle(Qt::SolidLine);
            break;
    }

    return result;
}

sexpresso::Sexp sKicadStrokeDefinition::to_sexp() const {
    /*
  (stroke
    (width WIDTH)
    (type TYPE)
    (color R G B A)
  )
)*/
    std::string type_s;
    switch (type) {
        case dash:
            type_s = "dash";
            break;
        case dash_dot:
            type_s = "dash_dot";
            break;
        case dash_dot_dot:
            type_s = "dash_dot_dot";
            break;
        case dot:
            type_s = "dot";
            break;
        case default_:
            type_s = "default";
            break;
        case solid:
            type_s = "solid";
            break;
    }

    sexpresso::Sexp sexp{};
    sexp.addChild("stroke");
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("width"), std::to_string(width)}));
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("type"), type_s}));
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("color"),      //
                                                               std::to_string(color.red()),   //
                                                               std::to_string(color.green()), //
                                                               std::to_string(color.blue()),  //
                                                               std::to_string(color.alpha())}));

    return sexp;
}

sKicadTextJustify::sKicadTextJustify() {}

sKicadTextJustify::sKicadTextJustify(const sexpresso::Sexp &from) {
    /*
     [(justify [left | right] [top | bottom] [mirror])]
     */
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "justify");
    is_mirror = false;
    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        assert(sexp_lib_node.kind == sexpresso::SexpValueKind::STRING);
        auto node_type = sexp_lib_node.value.str;

        if (node_type == "left") {
            // qDebug() << node_type;
            lr = left;
        } else if (node_type == "right") {
            // qDebug() << node_type;
            lr = right;
        } else if (node_type == "top") {
            // qDebug() << node_type;
            tb = top;
        } else if (node_type == "bottom") {
            // qDebug() << node_type;
            tb = bottom;
        } else if (node_type == "mirror") {
            // qDebug() << node_type;
            is_mirror = true;
        }
    }
}

sexpresso::Sexp sKicadTextJustify::to_sexp() const {
    /*
[(justify [left | right] [top | bottom] [mirror])]
)
   )*/
    std::string tb_s;
    std::string lr_s;
    std::string mirror_s;
    if (lr.has_value()) {
        switch (lr.value()) {
            case left:
                lr_s = "left";
                break;
            case right:
                lr_s = "right";
                break;
        }
    }
    if (tb.has_value()) {
        switch (tb.value()) {
            case top:
                tb_s = "top";
                break;
            case bottom:
                tb_s = "bottom";
                break;
        }
    }
    if (is_mirror) {
        mirror_s = "mirror";
    }

    sexpresso::Sexp sexp{};
    sexp.addChild("justify " + lr_s + " " + tb_s + " " + mirror_s);
    return sexp;
}

sKicadPointList::sKicadPointList(const sexpresso::Sexp &from) {
    /*
    (pts
       (xy X Y)
       ...
       (xy X Y)
       )
    )*/
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "pts");
    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];

        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "xy") {
            auto p = QPointF(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble(),
                             QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toDouble());
            pts.append(p);
        } else {
            assert(0);
        }
    }
}

sKicadPointList::sKicadPointList() {}

sexpresso::Sexp sKicadPointList::to_sexp() const {
    /*
(pts
   (xy X Y)
   ...
   (xy X Y)
   )
)*/
    sexpresso::Sexp sexp{};
    sexp.addChild("pts");
    for (auto &v : pts) {
        sexp.addChild(sexpresso::Sexp("xy " + std::to_string(v.x()) + " " + std::to_string(v.y())));
    }
    return sexp;
}

sKicadFillDefinition::sKicadFillDefinition() {
    type = none;
}

sKicadFillDefinition::sKicadFillDefinition(FillType &from) {
    switch (from) {
        case FillType::backgroundColor:
            type = background;
            break;
        case FillType::foregroundColor:
            type = outline;
            break;
        case FillType::none:
            type = none;
            break;
    }
}

sKicadFillDefinition::sKicadFillDefinition(const sexpresso::Sexp &from) {
    /*
  (fill
    (type none | outline | background)
  )
)*/

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "fill");
    auto sexp_type = from.value.sexp[1];
    QString type_s = QString::fromStdString(sexp_type.value.sexp[0].getString());
    assert(type_s == "type");
    QString ts = QString::fromStdString(sexp_type.value.sexp[1].getString());
    if (ts == "none") {
        type = none;
    } else if (ts == "outline") {
        type = outline;
    } else if (ts == "background") {
        type = background;
    } else {
        assert(0);
    }
}

QBrush sKicadFillDefinition::get_brush() const {
    switch (type) {
        case TypeEnum::none: {
            QBrush b;
            b.setStyle(Qt::NoBrush);
            return b;
        }
        case TypeEnum::outline: {
            QBrush brushForeground;
            brushForeground.setColor(COLOR_FOREGROUND);
            brushForeground.setStyle(Qt::SolidPattern);
            return brushForeground;
        }
        case TypeEnum::background: {
            QBrush brushBackground;
            brushBackground.setColor(COLOR_BACKGROUND);
            brushBackground.setStyle(Qt::SolidPattern);
            return brushBackground;
        }
    }
    assert(0);
    return QBrush();
}

sexpresso::Sexp sKicadFillDefinition::to_sexp() const {
    /*
  (fill
    (type none | outline | background)
  )
)*/
    std::string type_s;
    switch (type) {
        case none:
            type_s = "none";
            break;
        case outline:
            type_s = "outline";
            break;
        case background:
            type_s = "background";
            break;
    }

    sexpresso::Sexp sexp{};
    sexp.addChild("fill");
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("type"), sexpresso::Sexp(type_s)}));
    return sexp;
}

sKicadTextSize::sKicadTextSize() {
    height = 8;
    width = -1;
}

sKicadTextSize::sKicadTextSize(const sexpresso::Sexp &from) {
    /*
      (size HEIGHT WIDTH)
*/
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "size");
    height = QString::fromStdString(from.value.sexp[1].getString()).toDouble();
    width = QString::fromStdString(from.value.sexp[2].getString()).toDouble();
}

sexpresso::Sexp sKicadTextSize::to_sexp() const {
    /*
      (size HEIGHT WIDTH)
*/
    sexpresso::Sexp sexp{};
    sexp.addChild("size " + std::to_string(height) + " " + std::to_string(width));
    return sexp;
}

sKicadPinName::sKicadPinName() {}

sKicadPinName::sKicadPinName(const sexpresso::Sexp &from) {
    /*
    [(pin_names [(offset OFFSET)] hide)]
        */
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "pin_names");
    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        // assert(sexp_lib_node.kind == sexpresso::SexpValueKind::STRING);
        //assert(sexp_lib_node.kind == sexpresso::SexpValueKind::SEXP);
        std::string node_type;
        if (sexp_lib_node.kind == sexpresso::SexpValueKind::STRING) {
            node_type = sexp_lib_node.value.str;
        } else {
            node_type = sexp_lib_node.value.sexp[0].toString();
        }

        if (node_type == "offset") {
            offset = QString::fromStdString(sexp_lib_node.value.sexp[1].toString()).toDouble();
        } else if (node_type == "hide") {
            //qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            is_hidden = true;
        } else {
            assert(0);
        }
    }
}

sexpresso::Sexp sKicadPinName::to_sexp() const {
    /*
    [(pin_names [(offset OFFSET)] hide)]
        */
    sexpresso::Sexp sexp{};
    auto s = std::string();
    sexp.addChild("pin_names");
    if (offset.has_value()) {
        sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("offset"), std::to_string(offset.value())}));
    }
    if (is_hidden) {
        sexp.addChild("hide");
    }
    return sexp;
}
sKicadPositionIdentifier::sKicadPositionIdentifier() {}

sKicadPositionIdentifier::sKicadPositionIdentifier(const sexpresso::Sexp &from) {
    /*
  (at
    X
    Y
    [ANGLE]
  )
     */

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "at");
    pos = QPointF(QString::fromStdString(from.value.sexp[1].getString()).toDouble(), QString::fromStdString(from.value.sexp[2].getString()).toDouble());
    if (from.value.sexp.size() > 3) {
        angle = QString::fromStdString(from.value.sexp[3].getString()).toDouble();
    }
}

sexpresso::Sexp sKicadPositionIdentifier::to_sexp() const {
    /*
  (at
    X
    Y
    [ANGLE]
  )
Symbol text ANGLEs are stored in tenth’s of a degree. All other ANGLEs are stored in degrees.
     */
    sexpresso::Sexp sexp{};
    auto s = std::string("0.0");
    if (angle.has_value()) {
        s = std::to_string(angle.value());
    }

    sexp.addChild("at " + std::to_string(pos.x()) + " " + std::to_string(pos.y()) + " " + s);
    return sexp;
}

sKicadTextEffects::sKicadTextEffects() {}

sKicadTextEffects::sKicadTextEffects(const sexpresso::Sexp &from) {
    /*
  (effects
    (font
      [(face FACE_NAME)]
      (size HEIGHT WIDTH)
      [(thickness THICKNESS)]
      [bold]
      [italic]
      [(line_spacing LINE_SPACING)]
    )
[(justify [left | right] [top | bottom] [mirror])]
[hide]
)
 )*/

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "effects");
    is_hide = false;
    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];

        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "font") {
            //qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            font = sKicadTextFont(sexp_lib_node);
        } else if (node_type == "justify") {
            //qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            justify = sKicadTextJustify(sexp_lib_node);
        } else if (node_type == "hide") {
            //qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            is_hide = true;
        }
    }
}

QFont sKicadTextEffects::get_font() const {
    QFont result;
    if (font.face.has_value()) {
        result.setFamily(font.face.value());
    }
    const double size_factor = 25;
    //result.setPointSize(font.size.height * size_factor);
    result.setPointSize(round(font.size.height * size_factor));
    if (font.is_bold) {
        result.setBold(true);
    }
    if (font.is_italic) {
        result.setBold(true);
    }
    return result;
}

sexpresso::Sexp sKicadTextEffects::to_sexp() const {
    /*
  (effects
    (font
      [(face FACE_NAME)]
      (size HEIGHT WIDTH)
      [(thickness THICKNESS)]
      [bold]
      [italic]
      [(line_spacing LINE_SPACING)]
    )
 [(justify [left | right] [top | bottom] [mirror])]
 [hide]
)
)*/

    sexpresso::Sexp sexp{};
    sexp.addChild("effects");
    sexp.addChild(font.to_sexp());
    if (justify.has_value()) {
        sexp.addChild(justify.value().to_sexp());
    }
    if (is_hide) {
        sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("hide"), sexpresso::Sexp("yes")}));
    }
    return sexp;
}

#if 1
static void draw_arc_defined_by_three_points(QPainter &painter, QPointF start, QPointF mid, QPointF end, bool is_filled) {
    //from https://github.com/peteristhegreat/ThreePointsCircle/tree/master
    auto lineME = QLineF(mid, end);
    auto lineSE = QLineF(start, end);
    auto lineMS = QLineF(mid, start);

    auto rad = qAbs(lineME.length() / (2 * qSin(qDegreesToRadians(lineSE.angleTo(lineMS)))));

    auto bisectorBC = QLineF(lineME.pointAt(0.5), lineME.p2());
    bisectorBC.setAngle(lineME.normalVector().angle());
    QPointF center;
    auto bisectorBA = QLineF(lineMS.pointAt(0.5), lineMS.p2());
    bisectorBA.setAngle(lineMS.normalVector().angle());
    bisectorBA.intersects(bisectorBC, &center);

    auto circle = QRectF(center.x() - rad, center.y() - rad, rad * 2, rad * 2);
    // painter.drawEllipse(circle);

    auto lineCS = QLineF(center, start);
    auto lineCM = QLineF(center, mid);
    auto lineCE = QLineF(center, end);
#if 0
    auto p = painter.pen();

    p.setColor(QColorConstants::DarkYellow);
    p.setWidth(1);
    painter.setPen(p);

    painter.drawLine(lineCS);
    painter.drawLine(lineCM);
    painter.drawLine(lineCE);
#endif
    auto startAngle = lineCS.angle();
    auto spanAngle = lineCS.angleTo(lineCE);
#if 1
    // Make sure that the span angle covers all three points with the second point in the middle
    if (qAbs(spanAngle) < qAbs(lineCS.angleTo(lineCM)) || qAbs(spanAngle) < qAbs(lineCM.angleTo(lineCE))) {
        // swap the end point and invert the spanAngle
        startAngle = lineCE.angle();
        spanAngle = 360 - spanAngle;
    }
#endif
    //    int w = 10;
    //   auto boundingRectTemp = circle.adjusted(-w, -w, w, w);
    // p.setColor(QColorConstants::Black);
    // p.setWidth(10);
    // painter.setPen(p);
    if (is_filled) {
        painter.drawChord(circle, startAngle * 16, spanAngle * 16);
    } else {
        painter.drawArc(circle, startAngle * 16, spanAngle * 16);
    }
#if 0
    p.setColor(QColorConstants::DarkMagenta);
    p.setWidth(1);
    painter.setPen(p);
    painter.drawEllipse(circle);
#endif
    (void)startAngle;
    (void)lineCM;
}
#endif

sKicadGraphSymbolArc::sKicadGraphSymbolArc(const sexpresso::Sexp &from) {
    /*  (arc
    (start X Y)
    (mid X Y)
    (end X Y)
    STROKE_DEFINITION
    FILL_DEFINITION
  )*/
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "arc");

    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "start") {
            start.setX(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble());
            start.setY(QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toDouble());
        } else if (node_type == "mid") {
            mid.setX(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble());
            mid.setY(QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toDouble());
        } else if (node_type == "end") {
            end.setX(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble());
            end.setY(QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toDouble());
        } else if (node_type == "stroke") {
            stroke = sKicadStrokeDefinition(sexp_lib_node);
        } else if (node_type == "fill") {
            fill = sKicadFillDefinition(sexp_lib_node);
        } else {
            assert(0);
        }
    }
}
const int PAINT_SIZE_FACTOR = 20;

void sKicadGraphSymbolArc::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
    //   setBackgroundRole(QPalette::Base);
    //  setAutoFillBackground(true);
    (void)sexp_pin_names;

    painter.setPen(stroke.get_pen());
    painter.setBrush(fill.get_brush());
    if (fill.type == sKicadFillDefinition::none) {
        draw_arc_defined_by_three_points(painter, (start * PAINT_SIZE_FACTOR) + centerPoint, (mid * PAINT_SIZE_FACTOR) + centerPoint,
                                         (end * PAINT_SIZE_FACTOR) + centerPoint, false);
    } else {
        draw_arc_defined_by_three_points(painter, (start * PAINT_SIZE_FACTOR) + centerPoint, (mid * PAINT_SIZE_FACTOR) + centerPoint,
                                         (end * PAINT_SIZE_FACTOR) + centerPoint, true);
    };
}

sexpresso::Sexp sKicadGraphSymbolArc::to_sexp() const {
    /*  (arc
    (start X Y)
    (mid X Y)
    (end X Y)
    STROKE_DEFINITION
    FILL_DEFINITION
  )*/
    sexpresso::Sexp sexp{};
    sexp.addChild("arc");
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("start"),                   //
                                                               sexpresso::Sexp(std::to_string(start.x())), //
                                                               sexpresso::Sexp(std::to_string(start.y()))}));

    //  sexp.addChild(sexpresso::Sexp("start " + std::to_string(start.x()) + " " + std::to_string(start.y())));
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("mid"),                   //
                                                               sexpresso::Sexp(std::to_string(mid.x())), //
                                                               sexpresso::Sexp(std::to_string(mid.y()))}));

    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("end"),                   //
                                                               sexpresso::Sexp(std::to_string(end.x())), //
                                                               sexpresso::Sexp(std::to_string(end.y()))}));

    sexp.addChild(stroke.to_sexp());
    sexp.addChild(fill.to_sexp());
    return sexp;
}

sKicadGraphSymbolCircle::sKicadGraphSymbolCircle(const sexpresso::Sexp &from) {
    /*
 (circle
    (center X Y)
    (radius RADIUS)
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "circle");

    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "center") {
            center.setX(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble());
            center.setY(QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toDouble());

        } else if (node_type == "radius") {
            //qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            radius = QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble();
        } else if (node_type == "stroke") {
            stroke = sKicadStrokeDefinition(sexp_lib_node);
        } else if (node_type == "fill") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            fill = sKicadFillDefinition(sexp_lib_node);
        } else {
            assert(0);
        }
    }
}

void sKicadGraphSymbolCircle::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
    (void)sexp_pin_names;
    //   setBackgroundRole(QPalette::Base);
    //  setAutoFillBackground(true);

    painter.setPen(stroke.get_pen());
    painter.setBrush(fill.get_brush());
    painter.drawEllipse((center * PAINT_SIZE_FACTOR) + centerPoint, (radius * PAINT_SIZE_FACTOR), (radius * PAINT_SIZE_FACTOR));
}

sexpresso::Sexp sKicadGraphSymbolCircle::to_sexp() const {
    /*
 (circle
    (center X Y)
    (radius RADIUS)
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */
    sexpresso::Sexp sexp{};
    sexp.addChild("circle");
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("center"),                   //
                                                               sexpresso::Sexp(std::to_string(center.x())), //
                                                               sexpresso::Sexp(std::to_string(center.y()))}));

    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{
        sexpresso::Sexp("radius"),               //
        sexpresso::Sexp(std::to_string(radius)), //
    }));
    sexp.addChild(stroke.to_sexp());
    sexp.addChild(fill.to_sexp());
    return sexp;
}

sKicadGraphSymbolCurve::sKicadGraphSymbolCurve(const sexpresso::Sexp &from) {
    /*
  (bezier
    COORDINATE_POINT_LIST
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "bezier");

    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "pts") {
            points = sKicadPointList(sexp_lib_node);
        } else if (node_type == "stroke") {
            stroke = sKicadStrokeDefinition(sexp_lib_node);
        } else if (node_type == "fill") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            fill = sKicadFillDefinition(sexp_lib_node);
        } else {
            assert(0);
        }
    }
}

void sKicadGraphSymbolCurve::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
    (void)sexp_pin_names;
    painter.setPen(stroke.get_pen());
    painter.setBrush(fill.get_brush());
    auto myPath = QPainterPath();
    myPath.moveTo(points.pts[0] + centerPoint);
    for (int i = 1; i < points.pts.size(); i++) {
        myPath.lineTo((points.pts[1] * PAINT_SIZE_FACTOR) + centerPoint);
    }
    painter.drawPath(myPath);
}

sexpresso::Sexp sKicadGraphSymbolCurve::to_sexp() const {
    /*
  (bezier
    COORDINATE_POINT_LIST
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */
    sexpresso::Sexp sexp{};
    sexp.addChild("bezier");
    sexp.addChild(points.to_sexp());
    sexp.addChild(stroke.to_sexp());
    sexp.addChild(fill.to_sexp());
    return sexp;
}

sKicadGraphSymbolLine::sKicadGraphSymbolLine(const sexpresso::Sexp &from) {
    /*
  (polyline
    COORDINATE_POINT_LIST
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "polyline");

    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "pts") {
            points = sKicadPointList(sexp_lib_node);
        } else if (node_type == "stroke") {
            stroke = sKicadStrokeDefinition(sexp_lib_node);
        } else if (node_type == "fill") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            fill = sKicadFillDefinition(sexp_lib_node);
        } else {
            assert(0);
        }
    }
}

void sKicadGraphSymbolLine::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
    (void)sexp_pin_names;
    painter.setPen(stroke.get_pen());
    painter.setBrush(fill.get_brush());
    auto myPath = QPainterPath();
    myPath.moveTo(points.pts[0] * PAINT_SIZE_FACTOR + centerPoint);
    for (int i = 1; i < points.pts.size(); i++) {
        myPath.lineTo((points.pts[i] * PAINT_SIZE_FACTOR) + centerPoint);
    }
    painter.drawPath(myPath);
}

sexpresso::Sexp sKicadGraphSymbolLine::to_sexp() const {
    /*
  (polyline
    COORDINATE_POINT_LIST
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */
    sexpresso::Sexp sexp{};
    sexp.addChild("polyline");
    sexp.addChild(points.to_sexp());
    sexp.addChild(stroke.to_sexp());
    sexp.addChild(fill.to_sexp());
    return sexp;
}

sKicadGraphSymbolRectangle::sKicadGraphSymbolRectangle(const sexpresso::Sexp &from) {
    /*
  (rectangle
    (start X Y)
    (end X Y)
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "rectangle");

    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "start") {
            start.setX(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble());
            start.setY(QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toDouble());
        } else if (node_type == "end") {
            end.setX(QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble());
            end.setY(QString::fromStdString(sexp_lib_node.value.sexp[2].getString()).toDouble());
        } else if (node_type == "stroke") {
            stroke = sKicadStrokeDefinition(sexp_lib_node);
        } else if (node_type == "fill") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            fill = sKicadFillDefinition(sexp_lib_node);
        } else {
            assert(0);
        }
    }
}

void sKicadGraphSymbolRectangle::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
    (void)sexp_pin_names;
    painter.setPen(stroke.get_pen());
    painter.setBrush(fill.get_brush());
    painter.drawRect(QRectF((start * PAINT_SIZE_FACTOR) + centerPoint, (end * PAINT_SIZE_FACTOR) + centerPoint));
}

sexpresso::Sexp sKicadGraphSymbolRectangle::to_sexp() const {
    /*
  (rectangle
    (start X Y)
    (end X Y)
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */
    sexpresso::Sexp sexp{};
    sexp.addChild("rectangle");
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("start"),                   //
                                                               sexpresso::Sexp(std::to_string(start.x())), //
                                                               sexpresso::Sexp(std::to_string(start.y()))}));

    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("end"),                   //
                                                               sexpresso::Sexp(std::to_string(end.x())), //
                                                               sexpresso::Sexp(std::to_string(end.y()))}));

    sexp.addChild(stroke.to_sexp());
    sexp.addChild(fill.to_sexp());
    return sexp;
}

sKicadGraphSymbolText::sKicadGraphSymbolText(const sexpresso::Sexp &from) {
    /*
  (text
    "TEXT"
    POSITION_IDENTIFIER
    (effects TEXT_EFFECTS)
  )
     */
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "text");
    text = QString::fromStdString(from.value.sexp[1].getString());
    for (unsigned int i = 2; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "at") {
            at = sKicadPositionIdentifier(sexp_lib_node);
        } else if (node_type == "effects") {
            text_effects = sKicadTextEffects(sexp_lib_node);
        } else {
            assert(0);
        }
    }
}

void sKicadGraphSymbolText::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
    (void)sexp_pin_names;
    if (text_effects.is_hide == false) {
        painter.setFont(text_effects.get_font());
        painter.drawText((at.pos * PAINT_SIZE_FACTOR) + centerPoint, text);
    }
}

sexpresso::Sexp sKicadGraphSymbolText::to_sexp() const {
    /*
  (text
    "TEXT"
    POSITION_IDENTIFIER
    (effects TEXT_EFFECTS)
  )
     */
    sexpresso::Sexp sexp{};
    sexp.addChild("text");
    sexp.addChild("\"" + text.toStdString() + "\"");
    sexp.addChild(at.to_sexp());
    sexp.addChild(text_effects.to_sexp());

    return sexp;
}

sKicadGraphSymbolPin::sKicadGraphSymbolPin(const sexpresso::Sexp &from) {
    /*
  (pin
    PIN_ELECTRICAL_TYPE
    PIN_GRAPHIC_STYLE
    POSITION_IDENTIFIER
    (length LENGTH)
    (name "NAME" TEXT_EFFECTS)
    (number "NUMBER" TEXT_EFFECTS)
  )*/

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "pin");

    auto e_type_s = QString::fromStdString(from.value.sexp[1].getString());
    if (e_type_s == "input") {
        electrical_type = sElectricalType::input;
    } else if (e_type_s == "output") {
        electrical_type = sElectricalType::output;
    } else if (e_type_s == "bidirectional") {
        electrical_type = sElectricalType::bidirectional;
    } else if (e_type_s == "tri_state") {
        electrical_type = sElectricalType::tri_state;
    } else if (e_type_s == "passive") {
        electrical_type = sElectricalType::passive;
    } else if (e_type_s == "free") {
        electrical_type = sElectricalType::free;
    } else if (e_type_s == "unspecified") {
        electrical_type = sElectricalType::unspecified;
    } else if (e_type_s == "power_in") {
        electrical_type = sElectricalType::power_in;
    } else if (e_type_s == "power_out") {
        electrical_type = sElectricalType::power_out;
    } else if (e_type_s == "open_collector") {
        electrical_type = sElectricalType::open_collector;
    } else if (e_type_s == "open_emitter") {
        electrical_type = sElectricalType::open_emitter;
    } else if (e_type_s == "no_connect") {
        electrical_type = sElectricalType::no_connect;
    } else {
        assert(0);
    }

    auto graph_syle_s = QString::fromStdString(from.value.sexp[2].getString());
    if (graph_syle_s == "none") {
        grapical_style = sGraphicalStyle::none;
    } else if (graph_syle_s == "line") {
        grapical_style = sGraphicalStyle::line;
    } else if (graph_syle_s == "inverted") {
        grapical_style = sGraphicalStyle::inverted;
    } else if (graph_syle_s == "clock") {
        grapical_style = sGraphicalStyle::clock;
    } else if (graph_syle_s == "inverted_clock") {
        grapical_style = sGraphicalStyle::inverted_clock;
    } else if (graph_syle_s == "input_low") {
        grapical_style = sGraphicalStyle::input_low;
    } else if (graph_syle_s == "clock_low") {
        grapical_style = sGraphicalStyle::clock_low;
    } else if (graph_syle_s == "output_low") {
        grapical_style = sGraphicalStyle::output_low;
    } else if (graph_syle_s == "falling_edge_clock") {
        grapical_style = sGraphicalStyle::falling_edge_clock;
    } else if (graph_syle_s == "non_logic") {
        grapical_style = sGraphicalStyle::non_logic;
    } else {
        assert(0);
    }

    for (unsigned int i = 3; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        QString node_type = "";
        if (sexp_lib_node.isSexp()) {
            node_type = QString::fromStdString(sexp_lib_node.value.sexp[0].getString());
        } else {
            node_type = QString::fromStdString(sexp_lib_node.value.str);
        }
        if (node_type == "at") {
            assert(sexp_lib_node.isSexp());
            at = sKicadPositionIdentifier(sexp_lib_node);
        } else if (node_type == "length") {
            assert(sexp_lib_node.isSexp());
            length = QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble();
        } else if (node_type == "name") {
            assert(sexp_lib_node.isSexp());
            name = sKicadPinNameNumber(sexp_lib_node);
        } else if (node_type == "number") {
            assert(sexp_lib_node.isSexp());
            number = sKicadPinNameNumber(sexp_lib_node);
        } else if (node_type == "hide") {
            is_hide = true;
            //assert(0);
            //#error

        } else {
            assert(0);
        }
    }
}

void sKicadGraphSymbolPin::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
#if 1

    QPointF position_start;
    QPointF position_end;
    position_start = at.pos;
    position_end.setY(position_start.y() + length * qSin(qDegreesToRadians(at.angle.value_or(0))));
    position_end.setX(position_start.x() + length * qCos(qDegreesToRadians(at.angle.value_or(0))));

    float text_rotation_deg = 0;
    int text_flags = Qt::AlignVCenter;
    QRectF text_rect;
    const auto default_margin = 10;
    double margin = default_margin;
    bool is_hidden = false;
    if (sexp_pin_names.has_value()) {
        margin = sexp_pin_names.value().offset.value_or(default_margin) * PAINT_SIZE_FACTOR;
        is_hidden = sexp_pin_names.value().is_hidden;
    }
    // margin = 0;
    QPointF position_text = position_end * PAINT_SIZE_FACTOR;

    if (0 <= at.angle && at.angle < 90) { //right
        position_text.rx() += margin;
        text_flags |= Qt::AlignLeft;
        text_rotation_deg = 0;
    } else if (90 <= at.angle && at.angle < 180) { //up
        text_flags |= Qt::AlignLeft;
        position_text.ry() += margin;
        text_rotation_deg = 90;
    } else if (180 <= at.angle && at.angle < 270) { //left
        position_text.rx() -= margin;
        text_flags |= Qt::AlignRight;
        text_rotation_deg = 0;
    } else if (270 <= at.angle && at.angle < 360) { //down
        position_text.ry() -= margin;
        text_flags |= Qt::AlignRight;
        text_rotation_deg = 90;
    } else {
        assert(0);
    }

    text_rect.setTopLeft(position_text + centerPoint);
    text_rect.setWidth(1);
    text_rect.setHeight(1);

    position_start *= PAINT_SIZE_FACTOR;
    position_end *= PAINT_SIZE_FACTOR;
    position_start += centerPoint;
    position_end += centerPoint;
    painter.drawLine(position_start, position_end);
    QRectF connectCircle;
    const int CONNNECTCIRCLE_DIAMETER = 20;
    connectCircle.setTop(position_start.y() - CONNNECTCIRCLE_DIAMETER / 2);
    connectCircle.setLeft(position_start.x() - CONNNECTCIRCLE_DIAMETER / 2);
    connectCircle.setWidth(CONNNECTCIRCLE_DIAMETER);
    connectCircle.setHeight(CONNNECTCIRCLE_DIAMETER);
    painter.drawEllipse(connectCircle);

    if (grapical_style == sGraphicalStyle::inverted) {
        if (at.angle == 0) { //right
            connectCircle.setLeft(position_end.x() - CONNNECTCIRCLE_DIAMETER);
            connectCircle.setTop(position_end.y() - CONNNECTCIRCLE_DIAMETER / 2);
        } else if (at.angle == 90) { //up
            connectCircle.setLeft(position_end.x() - CONNNECTCIRCLE_DIAMETER / 2);
            connectCircle.setTop(position_end.y() - CONNNECTCIRCLE_DIAMETER);
        } else if (at.angle == 270) { //down
            connectCircle.setTop(position_end.y());
            connectCircle.setLeft(position_end.x() - CONNNECTCIRCLE_DIAMETER / 2);
        } else { //left
            connectCircle.setLeft(position_end.x());
            connectCircle.setTop(position_end.y() - CONNNECTCIRCLE_DIAMETER / 2);
        }
        connectCircle.setWidth(CONNNECTCIRCLE_DIAMETER);
        connectCircle.setHeight(CONNNECTCIRCLE_DIAMETER);
        painter.drawEllipse(connectCircle);
    }

    if ((name.label != "~")) {
        painter.setPen(name.get_pen_name());
        painter.setFont(name.effects.get_font());

        painter.save();
        QString text = name.label;
        if (text.startsWith("~")) {
            auto f = painter.font();
            f.setOverline(true);
            painter.setFont(f);
            text = text.removeFirst();
        }
        painter.translate(text_rect.topLeft());
        painter.rotate(text_rotation_deg);

        QRectF boundingRect;

        painter.drawText(QRectF(0, 0, 1, 1), text_flags, text, &boundingRect);
        painter.drawText(boundingRect, text_flags, text);
        painter.drawRect(text_rect);
        //painter.rotate(-text_rotation_deg);
        painter.restore();
        auto f = painter.font();
        f.setOverline(false);
        painter.setFont(f);
        text = text.removeFirst();
        (void)text_rotation_deg;
        (void)is_hidden;
    }
#else
    (void)painter;
    (void)centerPoint;
    (void)sexp_pin_names;

#endif
}

sexpresso::Sexp sKicadGraphSymbolPin::to_sexp() const {
    /*
  (pin
    PIN_ELECTRICAL_TYPE
    PIN_GRAPHIC_STYLE
    POSITION_IDENTIFIER
    (length LENGTH)
    (name "NAME" TEXT_EFFECTS)
    (number "NUMBER" TEXT_EFFECTS)
  )

     enum class sKicadPinElectricalType {
         input,
         output,
         bidirectional,
         tri_state,
         passive,
         free,
         unspecified,
         power_in,
         power_out,
         open_collector,
         open_emitter,
         no_connect
     };

     (pin power_in line
        (at -6.35 -10.16 90)
        (length 0) hide
        (name "GND"
            (effects
                 (font
                     (size 1.27 1.27)
                 )
            )
            )
             (number "7"
                 (effects
                     (font
                         (size 1.27 1.27)
                     )
            )
        )
    )
     */

    std::string et_s;
    switch (electrical_type) {
        case sElectricalType::input:
            et_s = "input";
            break;
        case sElectricalType::output:
            et_s = "output";
            break;
        case sElectricalType::bidirectional:
            et_s = "bidirectional";
            break;
        case sElectricalType::tri_state:
            et_s = "tri_state";
            break;
        case sElectricalType::passive:
            et_s = "passive";
            break;
        case sElectricalType::free:
            et_s = "free";
            break;
        case sElectricalType::unspecified:
            et_s = "unspecified";
            break;
        case sElectricalType::power_in:
            et_s = "power_in";
            break;
        case sElectricalType::power_out:
            et_s = "power_out";
            break;
        case sElectricalType::open_collector:
            et_s = "open_collector";
            break;
        case sElectricalType::open_emitter:
            et_s = "open_emitter";
            break;
        case sElectricalType::no_connect:
            et_s = "no_connect";
            break;
    }

    //enum class sGraphicalStylesKicadGraphSymbolPin { none = 0, line, inverted, clock, inverted_clock, input_low, clock_low, output_low, falling_edge_clock, non_logic };
    std::string gs_s;
    switch (grapical_style) {
        case sGraphicalStyle::none:
            gs_s = "none";
            break;
        case sGraphicalStyle::line:
            gs_s = "line";
            break;
        case sGraphicalStyle::inverted:
            gs_s = "inverted";
            break;
        case sGraphicalStyle::clock:
            gs_s = "clock";
            break;
        case sGraphicalStyle::inverted_clock:
            gs_s = "inverted_clock";
            break;
        case sGraphicalStyle::input_low:
            gs_s = "input_low";
            break;
        case sGraphicalStyle::clock_low:
            gs_s = "clock_low";
            break;
        case sGraphicalStyle::output_low:
            gs_s = "output_low";
            break;
        case sGraphicalStyle::falling_edge_clock:
            gs_s = "falling_edge_clock";
            break;
        case sGraphicalStyle::non_logic:
            gs_s = "non_logic";
            break;
    }
    sexpresso::Sexp sexp{};
    sexp.addChild("pin " + et_s + " " + gs_s);

    sexp.addChild(at.to_sexp());
    sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("length"), sexpresso::Sexp(std::to_string(length))}));
    //  sexp.addChild(sexpresso::Sexp("length " + std::to_string(length)));
    if (is_hide.value_or(false)) {
        sexp.addChild(sexpresso::Sexp("hide"));
        //to be tested
    }
    sexp.addChild(name.to_sexp());
    sexp.addChild(number.to_sexp());
    return sexp;
}

sSymbolProperty::sSymbolProperty() {}

sSymbolProperty::sSymbolProperty(const sexpresso::Sexp &from) {
    /*
          (property
              "KEY"
              "VALUE"
              (id N)
              POSITION_IDENTIFIER
              TEXT_EFFECTS
            }
        */
    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "property");
    key = QString::fromStdString(from.value.sexp[1].getString());
    value = QString::fromStdString(from.value.sexp[2].getString());
    for (unsigned int i = 3; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];

        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "id") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            id = QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toInt();
        } else if (node_type == "at") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            at = sKicadPositionIdentifier(sexp_lib_node);
        } else if (node_type == "effects") {
            //   qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            text_effects = sKicadTextEffects(sexp_lib_node);
        }
    }
}

void sSymbolProperty::set_design(FieldDesignSettingsItem design_item, bool overwrite_text_position) {
#if 0
    QPoint position;//
    int dimension;//
    KicadSymbolFieldOrientation_t orientation;//
    bool visible;//
    KicadSymbolFieldJustify_t hjustify;//
    KicadSymbolFieldJustify_t vjustify;//
    bool FontstyleItalic;//
    bool FontstyleBold;//
#endif
    if (overwrite_text_position) {
        at.pos = design_item.position;

        if (design_item.orientation == ksfo_horizontal) {
            at.angle = 0;
        } else if (design_item.orientation == ksfo_vertical) {
            at.angle = 90;
        }
        sKicadTextJustify justify;
        if (design_item.hjustify == ksfj_top) {
            justify.tb = sKicadTextJustify::top;
        } else if (design_item.hjustify == ksfj_bottom) {
            justify.tb = sKicadTextJustify::bottom;
        } else if (design_item.hjustify == ksfj_center) {
            justify.tb.reset();
        } else {
            assert(0);
        }
        if (design_item.vjustify == ksfj_right) {
            justify.lr = sKicadTextJustify::right;
        } else if (design_item.vjustify == ksfj_left) {
            justify.lr = sKicadTextJustify::left;
        } else if (design_item.vjustify == ksfj_center) {
            justify.lr.reset();
        } else {
            assert(0);
        }
        if (justify.lr.has_value() || justify.tb.has_value()) {
            text_effects.justify = justify;
        } else {
            text_effects.justify.reset();
        }
    }

    text_effects.is_hide = !design_item.visible;
    text_effects.font.is_bold = design_item.FontstyleBold;
    text_effects.font.is_italic = design_item.FontstyleItalic;
    text_effects.font.size.height = design_item.dimension;
}

sexpresso::Sexp sSymbolProperty::to_sexp() const {
    /*
      (property
          "KEY"
          "VALUE"
          (id N)
          POSITION_IDENTIFIER
          TEXT_EFFECTS
        }


             Key         Ordinal 	Description                         Empty Allowed
             Reference   0           Symbol reference designator         No
             Value       1           Symbol value string                 No
             Footprint   2           Symbol footprint library identifier Yes
             Datasheet   3           Symbol datasheet link               Yes



          Reserved Symbol Property Keys
              ki_keywords,
              ki_description,
              ki_locked
              ki_fp_filters

     */
    sexpresso::Sexp sexp{};
    sexp.addChild("property \"" + key.toStdString() + "\" \"" + value.toStdString() + "\"");
    if (id.has_value()) {
        sexp.addChild(sexpresso::Sexp("id " + std::to_string(id.value())));
    }
    sexp.addChild(at.to_sexp());
    sexp.addChild(text_effects.to_sexp());

    return sexp;
}

//const sexpresso::Sexp sKicadStrokeDefinition::to_sexp() {}

sexpresso::Sexp sKicadGraphSymbol::to_sexp() const {
    sexpresso::Sexp sexp{};
    assert(0);
    return sexp;
}

void sKicadGraphSymbol::paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const {
    (void)painter;
    (void)centerPoint;
    (void)sexp_pin_names;
}

sKicadTextFont::sKicadTextFont() {}

sKicadTextFont::sKicadTextFont(const sexpresso::Sexp &from) {
    /*

     (font
       [(face FACE_NAME)]
       (size HEIGHT WIDTH)
       [(thickness THICKNESS)]
       [bold]
       [italic]
       [(line_spacing LINE_SPACING)]
     )
*/

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert(symbol_s == "font");
    is_bold = false;
    is_italic = false;
    for (unsigned int i = 1; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "face") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            face = QString::fromStdString(sexp_lib_node.value.sexp[1].getString());

        } else if (node_type == "size") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            size = sKicadTextSize(sexp_lib_node);
        } else if (node_type == "thickness") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            thickness = QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble();
        } else if (node_type == "bold") {
            //  qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            is_bold = true;
        } else if (node_type == "italic") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            is_italic = true;
        } else if (node_type == "line_spacing") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            line_spacing = QString::fromStdString(sexp_lib_node.value.sexp[1].getString()).toDouble();
        }
    }
}

sexpresso::Sexp sKicadTextFont::to_sexp() const {
    sexpresso::Sexp sexp{};
    sexp.addChild("font");
    if (face.has_value()) {
        sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("face"), //
                                                                   sexpresso::Sexp(face.value().toStdString())}));
    }
    sexp.addChild(size.to_sexp());
    if (thickness.has_value()) {
        sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("thickness"), //
                                                                   sexpresso::Sexp(std::to_string(thickness.value()))}));
    }
    if (is_bold) {
        sexp.addChild(sexpresso::Sexp("bold"));
    }
    if (is_italic) {
        sexp.addChild(sexpresso::Sexp("italic"));
    }
    if (line_spacing.has_value()) {
        sexp.addChild(sexpresso::Sexp(std::vector<sexpresso::Sexp>{sexpresso::Sexp("line_spacing"), //
                                                                   sexpresso::Sexp(std::to_string(line_spacing.value()))}));
    }
    return sexp;
}

sKicadPinNameNumber::sKicadPinNameNumber() {}

sKicadPinNameNumber::sKicadPinNameNumber(QString type, QString &label) {
    node_type = type;
    this->label = label;
}

sKicadPinNameNumber::sKicadPinNameNumber(const sexpresso::Sexp &from) {
    /*

(name "NAME" TEXT_EFFECTS)
(number "NUMBER" TEXT_EFFECTS)
     */

    QString symbol_s = QString::fromStdString(from.value.sexp[0].getString());
    assert((symbol_s == "number") || (symbol_s == "name"));
    node_type = QString::fromStdString(from.value.sexp[0].getString());
    label = QString::fromStdString(from.value.sexp[1].getString());
    for (unsigned int i = 2; i < from.value.sexp.size(); i++) {
        auto &sexp_lib_node = from.value.sexp[i];
        auto node_type = sexp_lib_node.value.sexp[0].getString();

        if (node_type == "effects") {
            // qDebug() << QString::fromStdString(sexp_lib_node.value.sexp[1].getString());
            effects = sKicadTextEffects(sexp_lib_node);
        } else {
            assert(0);
        }
    }
}

QPen sKicadPinNameNumber::get_pen_name() const {
    auto result = QPen(QColor(32, 96, 96));
    return result;
}

sexpresso::Sexp sKicadPinNameNumber::to_sexp() const {
    sexpresso::Sexp sexp{};
    sexp.addChild(node_type.toStdString() + " \"" + label.toStdString() + "\"");
    sexp.addChild(effects.to_sexp());
    return sexp;
}
