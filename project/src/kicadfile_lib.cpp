#include "kicadfile_lib.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

QStringList splitParams(const QString & params)
{
    QStringList list;
    QString arg;
    enum { Idle, Arg, QuotedArg } state = Idle;
    foreach (QChar const c, params) {

        switch (state) {
        case Idle:
            if (c == '"'){
                state = QuotedArg;
            }else if (!c.isSpace()){
                arg += c;
                state = Arg;
            }
            break;
        case Arg:
            if (c == '"'){
                state = QuotedArg;
            }else if (!c.isSpace()){
                arg += c;
            }else {
                list << arg;
                arg.clear();
                state = Idle;
            }
            break;
        case QuotedArg:
            if (c == '"'){
                //state = arg.isEmpty() ? Idle : Arg;
                state = Arg;
            }else{
                arg += c;
            }
            break;
        }

    }
    list << arg;
    //qDebug() << list;
    return list;
}

KICADLibSchematicDeviceDefinition::KICADLibSchematicDeviceDefinition(){

}

void KICADLibSchematicDeviceDefinition::decode(QString str){
    QStringList params = splitParams(str);
    name = params[1];
    reference = params[2];
    text_offset = params[4].toInt();
    drawPinNumber = params[5] == "Y";
    drawPinName = params[6] == "Y";
    unitCount = params[7].toInt();
    unitLocked = params[8]=="L";
    optionFlag_IsPowersymbol = params[9] == "P";

}

QString KICADLibSchematicDeviceDefinition::encode()
{
    QString result;
    result = "DEF ";
    result += name +" ";
    result += reference +" 0 ";// the zero is field "unused"
    result += QString::number(text_offset) +" ";
    if (drawPinNumber){
        result += "Y ";
    }else{
        result += "N ";
    }

    if (drawPinName){
        result += "Y ";
    }else{
        result += "N ";
    }

    result += QString::number(unitCount) +" ";

    if (unitLocked){
        result += "L ";
    }else{
        result += "F ";
    }

    if (optionFlag_IsPowersymbol){
        result += "P";
    }else{
        result += "N";
    }
    return result;
}

void KICADLibSchematicDeviceDefinition::clear()
{
    name = "";
    reference = "";
    text_offset = 0;
    drawPinNumber = false;
    drawPinName = false;
    unitCount = 0;
    unitLocked = false;
    optionFlag_IsPowersymbol = false;
}

bool KICADLibSchematicDeviceDefinition::isValid()
{
    bool result = true;
    if(name==""){
        result = false;
    }

    if(reference==""){
        result = false;
    }
    return result;
}



KICADLibSchematicDeviceField::KICADLibSchematicDeviceField(){
    clear();
}

void KICADLibSchematicDeviceField::decode(QString str){
    auto decodeJustify = [] (QChar ch) {
        KicadSymbolFieldJustify_t justify = ksfj_left;
        if (ch == 'L'){
            justify = ksfj_left;
        }else if (ch == 'R'){
            justify = ksfj_right;
        }else if (ch == 'C'){
            justify = ksfj_center;
        }else if (ch == 'B'){
            justify = ksfj_bottom;
        }else if (ch == 'T'){
            justify = ksfj_top;
        }
        return justify;
    };

    QStringList params = splitParams(str);
    QString fi = params[0];
    fieldIndex.setRawIndex(fi.right(1).toInt());
    text = params[1];
    position.setX(params[2].toInt());
    position.setY(params[3].toInt());
    dimension = params[4].toInt();
    if (params[5].toUpper() == "H"){
        orientation = ksfo_horizontal;
    }else{
        orientation = ksfo_vertical;
    }

    visible = params[6] == "V";
    if (7 < params.count()){
        QString hjus = params[7].toUpper();
        hjustify = decodeJustify(hjus[0]);
    }else{
        qDebug() << "field not correct. defaulted.";
        hjustify = ksfj_left;
    }
    if (8 < params.count()){
        QString para = params[8].toUpper();
        vjustify = decodeJustify(para[0]);

        FontstyleItalic = false;
        FontstyleBold = false;
        if (para[1] == 'I'){
            FontstyleItalic = true;
        }
        if (para[2] == 'B'){
            FontstyleBold = true;
        }
    }else{
        qDebug() << "field not correct. defaulted.";
        vjustify = ksfj_top;
        FontstyleItalic = false;
        FontstyleBold = false;
    }


    if (9 < params.count()){
        name = params[9];
    }else{
        name = "";
    }
}


QString KICADLibSchematicDeviceField::encode()
{
    auto encodeJustify = [] (KicadSymbolFieldJustify_t justify) {
        QChar ch=0;
        if (justify == ksfj_left){
            ch = 'L';
        }else if (justify == ksfj_right){
            ch = 'R';
        }else if (justify == ksfj_center){
            ch = 'C';
        }else if (justify == ksfj_bottom){
            ch = 'B';
        }else if (justify == ksfj_top){
            ch = 'T';
        }
        return ch;
    };

    QString result="F";
    result += QString::number(fieldIndex.getRawIndex())+" ";

    result += "\""+text+"\" ";
    result += QString::number(position.x())+" ";
    result += QString::number(position.y())+" ";
    result += QString::number(dimension)+" ";
    if(orientation == ksfo_horizontal){
        result += "H ";
    }else{
        result += "V ";
    }

    if(visible){
        result += "V ";
    }else{
        result += "I ";
    }
    result += QString(encodeJustify(hjustify))+" ";
    result += encodeJustify(vjustify);
    if(FontstyleItalic){
        result += "I";
    }else{
        result += "N";
    }

    if(FontstyleBold){
        result += "B ";
    }else{
        result += "N ";
    }

    result += "\""+name+"\"";
    return result;
}

void KICADLibSchematicDeviceField::clear()
{
    fieldIndex.setRawIndex(0);
    text = "";
    position = QPoint(0,0);
    dimension = 1;
    orientation = ksfo_horizontal;
    visible = false;
    hjustify = ksfj_left ;
    vjustify = ksfj_top;
    FontstyleItalic = false;
    FontstyleBold = false;
    name = "";
}

void KICADLibSchematicDeviceField::setDesign(FieldDesignSettingsItem designItem, bool overwriteTextPosition)
{
    if (overwriteTextPosition){
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

bool KICADLibSchematicDeviceField::operator<(const KICADLibSchematicDeviceField &R) const
{

    return fieldIndex.rawIndex < R.fieldIndex.rawIndex;
}

bool KICADLibSchematicDeviceField::operator>(const KICADLibSchematicDeviceField &R) const
{
    return fieldIndex.rawIndex > R.fieldIndex.rawIndex;
}



KICADLibSchematicDevice::KICADLibSchematicDevice(){

}

KICADLibSchematicDeviceLibrary::KICADLibSchematicDeviceLibrary()
{

}

void KICADLibSchematicDeviceLibrary::clear()
{
    fileName = "";
    symbolList.clear();

}

QString KICADLibDCMFile::getdcmFileNameFromLibFileName(QString libfileName)
{
    QFileInfo fi(libfileName);
#if 0
    qDebug() << "orig:"<<libfileName;
    qDebug() << "baseName:"<<fi.baseName();
    qDebug() << "absolutePath:"<<fi.absolutePath();
    qDebug() << "bundleName:"<<fi.bundleName();
#endif
    return QString(fi.absolutePath())+QDir().separator()+fi.baseName()+".dcm";
}

void KICADLibDCMFile::loadFile(QString libfileName)
{
    clear();
    QString dcmFileName = getdcmFileNameFromLibFileName(libfileName);

    if (QFileInfo::exists(dcmFileName)){
        QFile dcmFile(dcmFileName);
        dcmFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream dcmStream(&dcmFile);
         KICADLibDCMEntry dcmEntry;
        while(!dcmStream.atEnd()){
            QString line = dcmStream.readLine();

            //qDebug() << line;
            line = line.trimmed();
            QChar line_c = line[0];
            QString data = line.mid(1).trimmed();

            if (line.startsWith("$CMP")){
                dcmEntry.name = line.split(" ")[1].trimmed();
                //qDebug() << "dcmEntry.name" << dcmEntry.name;
                dcmEntry.description = "";
                dcmEntry.keywords = "";
                dcmEntry.datasheetlink = "";
            }else if (line_c == 'D'){
                dcmEntry.description = data;
            }else if (line_c== 'K'){
                dcmEntry.keywords = data;
            }else if(line_c == 'F'){
                dcmEntry.datasheetlink = data;
            }else if(line.startsWith("$ENDCMP")){
                dcmEntries.append(dcmEntry);
                componentNames.append(dcmEntry.name);
            }

        }
    }

}

KICADLibDCMEntry KICADLibDCMFile::getEntryByName(QString name, bool &ok)
{
    KICADLibDCMEntry result;
    int index = componentNames.indexOf(name);
    if (index != -1){
        result = dcmEntries[index];
        ok = true;
       // qDebug() << "found dcm";
    }else{
        ok = false;
        //qDebug() << "not found dcm " + name << componentNames.count();
    }
    return result;
}

void KICADLibDCMFile::clear()
{
    componentNames.clear();
    dcmEntries.clear();
}




void KICADLibSchematicDeviceLibrary::loadFile(QString fileName)
{
    enum class SECTION{none,def,field,fplist,draw,alias};
    clear();
    this->fileName=fileName;
    KICADLibDCMFile dcmFile;
    dcmFile.loadFile(fileName);
    QFile libFile(fileName);
    libFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream textStream(&libFile);
    symbolList.clear();
    KICADLibSchematicDevice libDevice;
    SECTION section=SECTION::none;
    while (!textStream.atEnd())
    {
        QString line = textStream.readLine();
        line = line.trimmed();
        if (section == SECTION::fplist){

            if (line.startsWith("$ENDFPLIST")){
                section=SECTION::none;

            }else{
                libDevice.fpList.append(line.trimmed());
                //qDebug() << line;
            }
        }else if(section == SECTION::draw){
            if (line.startsWith("ENDDRAW")){
                section=SECTION::none;

            }else{
                KICADLibSchematicDrawElement drawElement(line);
                libDevice.drawSymbols.append(drawElement);
            }
        }else{
            if (line.startsWith("DEF")){
                section=SECTION::def;
                libDevice.clear();
                libDevice.def.decode(line);

            }

            if (line.startsWith("F")){
                section=SECTION::field;
                KICADLibSchematicDeviceField field;
                field.decode(line);
                libDevice.fields.append(field);
            }

            if (line.startsWith("$FPLIST")){
                section=SECTION::fplist;
            }

            if (line.startsWith("DRAW")){
                section=SECTION::draw;

            }

            if (line.startsWith("ENDDEF")){
                section=SECTION::none;
                bool ok = false;
                QString nextName=libDevice.def.name;
                libDevice.dcmEntry = dcmFile.getEntryByName(nextName,ok);
                if (libDevice.dcmEntry.name == ""){
                    QString nextName=libDevice.def.name;
                    if (nextName[0]== '~' ){
                        nextName = nextName.mid(1);
                    }
                    libDevice.dcmEntry = dcmFile.getEntryByName(nextName,ok);
                }
               // qDebug() << libDevice.dcmEntry.description;
                symbolList.append(libDevice);
            }
        }
    }
    (void)section;
}

bool KICADLibSchematicDeviceLibrary::saveFile(QString fileName)
{
    QFile file(fileName);
    QFile fileDCM(KICADLibDCMFile::getdcmFileNameFromLibFileName(fileName));

    bool filedcmok=fileDCM.open(QIODevice::WriteOnly | QIODevice::Truncate);

    bool result;
    if ( file.open(QIODevice::WriteOnly | QIODevice::Truncate) && filedcmok){
        result = true;
        QTextStream ts(&file);
        QTextStream dcms(&fileDCM);
        ts << "EESchema-LIBRARY Version 2.3"<< endl;
        dcms << "EESchema-DOCLIB  Version 2.0"<< endl;
        for(int i=0;i<symbolList.count();i++){


            ts << symbolList[i].def.encode()<< endl;
            for (int n=0;n<symbolList[i].fields.count();n++){
                ts << symbolList[i].fields.encode(n)<< endl;
            }
            if (symbolList[i].fpList.count()){
                ts << "$FPLIST"<< endl;
                for(int n=0;n<symbolList[i].fpList.count();n++){
                    ts << " " << symbolList[i].fpList[n] << endl;
                }

                ts << "$ENDFPLIST"<< endl;
            }
            if (symbolList[i].alias.count()){
                QString alias_str = "ALIAS";
                for (int n=0;n<symbolList[i].alias.count();n++){
                    alias_str += " "+symbolList[i].alias[n];
                }
                ts << alias_str<< endl;;
            }
            ts << "DRAW"<< endl;;
            for (int n=0;n<symbolList[i].drawSymbols.count();n++){
                ts << symbolList[i].drawSymbols[n].encode()<< endl;;
            }
            ts << "ENDDRAW"<< endl;;
            ts << "ENDDEF"<< endl;;

            if(symbolList[i].dcmEntry.hasFields()){
                QString dcmName=symbolList[i].def.name;
                if (dcmName[0]== '~' ){
                    dcmName = dcmName.mid(1);
                }
                dcms << "$CMP "+dcmName<< endl;
                if (symbolList[i].dcmEntry.description.count()){
                    dcms << "D "<< symbolList[i].dcmEntry.description << endl;
                }
                if (symbolList[i].dcmEntry.keywords.count()){
                    dcms << "K "<< symbolList[i].dcmEntry.keywords << endl;
                }
                if (symbolList[i].dcmEntry.datasheetlink.count()){
                    dcms << "F "<< symbolList[i].dcmEntry.datasheetlink << endl;
                }
                dcms << "$ENDCMP"<< endl;

            }
        }
    }else{
        result = false;
    }
    file.close();
    fileDCM.close();
    return result;
}

QString KICADLibSchematicDeviceLibrary::getName()
{
    QFileInfo fi(fileName);
    return fi.fileName();
}

QList<KICADLibSchematicDevice> KICADLibSchematicDeviceLibrary::getSymbolList()
{
    return symbolList;
}

int KICADLibSchematicDeviceLibrary::indexOf(QString deviceName)
{
    int result = -1;
    for(int i=0;i< symbolList.count();i++){
        if (symbolList[i].def.name == deviceName){
            result = i;
        }
    }
    return result;
}

void KICADLibSchematicDeviceLibrary::insertDevice(KICADLibSchematicDevice device)
{
    int index = indexOf(device.def.name);
    if (index != -1){
        symbolList.removeAt(index);
    }
    symbolList.append(device);
}


void KICADLibSchematicDevice::clear()
{
    fields.clear();
    def.clear();
    fpList.clear();
    alias.clear();
    drawSymbols.clear();
    dcmEntry.clear();
}

bool KICADLibSchematicDevice::isValid()
{
    bool result = true;

    if(fields.count()==0){
        result = false;
    }

#if 0
    if(drawSymbols.count()==0){
        result = false;
    }
#endif

    if(def.isValid()==false){
        result = false;
    }
    return result;

}

KICADLibSchematicDeviceField KicadFieldList::getFieldbyIndex(int index)
{
    KICADLibSchematicDeviceField result;

    for(int i=0;i< fields.count();i++){
        if (fields[i].fieldIndex.getRawIndex() == index){
            result = fields[i];
            break;
        }
    }
    return result;
}

int KicadFieldList::count()
{
    return fields.count();
}

void KicadFieldList::append(KICADLibSchematicDeviceField field)
{
    fields.append(field);
}

QString KicadFieldList::encode(int index)
{
    return fields[index].encode();
}

void KicadFieldList::clear()
{
    fields.clear();
}

void KicadFieldList::removeAllAboveIndex(int index)
{
    int i=0;
    while(i < fields.count()){
        if (fields[i].fieldIndex.getRawIndex() >= index){
            fields.removeAt(i);
        }else{
            i++;
        }
    }
}

void KicadFieldList::setField(KICADLibSchematicDeviceField field)
{
    bool found = false;
    int foundatIndex = 0;
    for(int i=0;i< fields.count();i++){
        if (field.fieldIndex.getRawIndex() == fields[i].fieldIndex.getRawIndex()){
            found = true;
            foundatIndex = i;
            break;
        }
    }
    if(found){
        fields.removeAt(foundatIndex);
    }
    fields.append(field);
    qSort(fields.begin(), fields.end(), qLess<KICADLibSchematicDeviceField>());
}



QList<KICADLibSchematicDrawElement> KICADLibSchematicDevice::getDrawSymbols()
{
    return drawSymbols;
}

KICADLibFieldIndex::KICADLibFieldIndex()
{
    rawIndex =0;
}

void KICADLibFieldIndex::setRawIndex(int rawindex)
{
    this->rawIndex = rawindex;

}

int KICADLibFieldIndex::getRawIndex()
{
    return rawIndex;
}

QString KICADLibFieldIndex::getFieldIndexDescription()
{
    switch(rawIndex){
    case 1: return "value";
            break;
    case 0: return "reference";
            break;
    case 2: return "footprint";
            break;
    case 3: return "datasheet";
            break;
    }
    return "";
}

KICADLibSchematicDrawElement::KICADLibSchematicDrawElement(QString str)
{
    //orientation = U (up) D (down) R (right) L (left).
    auto decodeOrientation = [] (QChar ch) {
        KicadSymbolOrientation_t orientation = ksr_none;
        if (ch == 'U'){
            orientation = ksr_up;
        }else if (ch == 'D'){
            orientation = ksr_down;
        }else if (ch == 'R'){
            orientation = ksr_right;
        }else if (ch == 'L'){
            orientation = ksr_left;
        }
        return orientation;
    };

    originalText = str;
    QStringList params = splitParams(str);
    if (str[0] == 'P'){
        drawtype = DrawType::polygon;
    }else if (str[0] == 'S'){
        drawtype = DrawType::rectangle;
        start.setX(params[1].toInt());
        start.setY(params[2].toInt());
        end.setX(params[3].toInt());
        end.setY(params[4].toInt());
        unit = params[5].toInt();
        convert = params[6].toInt();
        if (7 < params.count()){
            thickness = params[7].toInt();
        }else{
            thickness = 1;
            qDebug() << "field not correct. defaulted.";
        }
        if (8 < params.count()){
            cc_filled = params[8].toUpper() == "F";
        }else{
            qDebug() << "field not correct. defaulted.";
            cc_filled = false;
        }
        /*
 * Rectangle
Format:
S startx starty endx endy unit convert thickness cc
With
• unit = 0 if common to the parts; if not, number of part (1. .n).
• convert = 0if common to all parts. If not, number of the part (1. .n).
• thickness = thickness of the outline.
• cc = N F or F ( F = filled Rectangle,; f = . filled Rectangle, N = transparent background)
Example:
S 0 50.900.900 0 1 0 f
*/

    }else if (str[0] == 'C'){
        drawtype = DrawType::circle;
    }else if (str[0] == 'A'){
        drawtype = DrawType::arc;
    }else if (str[0] == 'T'){
        drawtype = DrawType::text;
    }else if (str[0] == 'X'){
        /*
         * 2.3.4 - Description of pins
        Format:
        X name number posx posy length orientation Snum Snom unit convert Etype [shape].
        With:
        • orientation = U (up) D (down) R (right) L (left).
        • name = name (without space) of the pin. if ~: no name
        • number = n pin number (4 characters maximum).
        • length = pin length.
        • Snum = pin number text size.
        • Snom = pin name text size.
        • unit = 0 if common to all parts. If not, number of the part (1. .n).
        • convert = 0 if common to the representations, if not 1 or 2.
        • Etype = electric type (1 character)
        • shape = if present: pin shape (clock, inversion...).
        Example:
        X TO 1 - 200 0.150 R 40 40 1 1 P
        X K 2.200 0.150 L 40 40 1 1 P
        X 0 1 0 0 0 R 40 40 1 1 W NC
        X ~ 2 0 - 250 200 U 40 40 1 1 P
        Etype list:
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
        position.setX(params[3].toInt());
        position.setY(params[4].toInt());
        length = params[5].toInt();
        orientation = decodeOrientation(params[6].toStdString()[0]);
        pinNumberTextSize = params[7].toInt();
        pinNameTextSize = params[8].toInt();
        unit = params[9].toInt();
        convert = params[10].toInt();
        etype.decode(params[11].toStdString()[0]);
        if (params.count() > 12){
            shape.decode(params[12]);
        }else{
            shape.setShape(PShape::Line);
        }


    }else{
        drawtype = DrawType::none;
    }

}

QString KICADLibSchematicDrawElement::encode()
{
    return originalText;
}
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
#endif

DrawType KICADLibSchematicDrawElement::getDrawType()
{
 return drawtype;
}





void ElectricalType::decode(char ch)
{
    switch(ch){
    case 'I': t=ElType::Input;          break;
    case 'O': t=ElType::Output;         break;
    case 'B': t=ElType::Bidirectional;  break;
    case 'T': t=ElType::Tristate;       break;
    case 'P': t=ElType::Passive;        break;
    case 'U': t=ElType::Unsepcified;    break;
    case 'W': t=ElType::Power_in;       break;
    case 'w': t=ElType::Power_out;      break;
    case 'C': t=ElType::Open_collector; break;
    case 'E': t=ElType::Open_emitter;   break;
    case 'N': t=ElType::Not_connected;  break;
    }
}

ElType ElectricalType::getType()
{
    return t;
}

void PinShape::decode(QString str)
{
    if (str == ""){
        s=PShape::Line;
    }else if(str=="I"){
        s=PShape::Inverted;
    }else if(str=="C"){
        s=PShape::Clock;
    }else if(str=="CI"){
        s=PShape::Inverted_clock;
    }else if(str=="L"){
        s=PShape::Input_low;
    }else if(str=="CL"){
        s=PShape::Clock_low;
    }else if(str=="V"){
        s=PShape::OutputLow;
    }else if(str=="F"){
        s=PShape::FallingEdgeClock;
    } else if(str=="X"){
        s=PShape::NonLocgic;
    }else{
        s=PShape::None;
    }
}

PShape PinShape::getShape()
{
    return s;
}

void PinShape::setShape(PShape s)
{
    this->s = s;
}



KICADLibDCMEntry::KICADLibDCMEntry()
{
    clear();
}

void KICADLibDCMEntry::clear()
{
    name = "";
    description = "";
    keywords = "";
    datasheetlink = "";
}

bool KICADLibDCMEntry::hasFields()
{
    return (description != "") || (keywords != "") || (datasheetlink != "");
}

KICADLibFootprintLibrary::KICADLibFootprintLibrary()
{

}

void KICADLibFootprintLibrary::scan(QString path)
{
    this->path = path;
    QDirIterator it_root(this->path, QDir::Dirs, QDirIterator::NoIteratorFlags);

    footprintnames.clear();
    while (it_root.hasNext()) {
        QString name = it_root.next();
        if (name.endsWith(".pretty")){

            QDirIterator it_fp(name, QDir::Files, QDirIterator::NoIteratorFlags);

            while (it_fp.hasNext()) {
                QString name_fp = it_fp.next();
                QFileInfo fi(name_fp);
                //qDebug() << fi.baseName();
                footprintnames.append(fi.baseName());
            }
        }
    }
    footprintnames.sort();
}

QStringList KICADLibFootprintLibrary::getFootprintList()
{
    return footprintnames;
}
