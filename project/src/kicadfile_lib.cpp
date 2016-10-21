#include "kicadfile_lib.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>


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



KICADLibSchematicDeviceField::KICADLibSchematicDeviceField(){

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
    QString hjus = params[7].toUpper();
    hjustify = decodeJustify(hjus[0]);
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

    if (params.count() > 9){
        name = params[9];
    }else{
        name = "";
    }
}


KICADLibSchematicDevice::KICADLibSchematicDevice(){

}

KICADLibSchematicDeviceLibrary::KICADLibSchematicDeviceLibrary()
{

}


void KICADLibSchematicDeviceLibrary::loadFile(QString fileName)
{
    enum class SECTION{none,def,field,fplist,draw,alias};
    this->fileName=fileName;
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
                symbolList.append(libDevice);
            }
        }
    }
    (void)section;
}

QList<KICADLibSchematicDevice> KICADLibSchematicDeviceLibrary::getSymbolList()
{
    return symbolList;
}


void KICADLibSchematicDevice::clear()
{

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
    paramStr = str;
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
        thickness = params[7].toInt();
        cc_filled = params[8].toUpper() == "F";
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
        pinNumberTextSize = params[6].toInt();
        pinNameTextSize = params[7].toInt();
        unit = params[8].toInt();
        convert = params[9].toInt();
        etype.decode(params[10].toStdString()[0]);
        shape.decode(params[11]);


    }else{
        drawtype = DrawType::none;
    }

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
    case 'I': t=ElectricalType::Input;          break;
    case 'O': t=ElectricalType::Output;         break;
    case 'B': t=ElectricalType::Bidirectional;  break;
    case 'T': t=ElectricalType::Tristate;       break;
    case 'P': t=ElectricalType::Passive;        break;
    case 'U': t=ElectricalType::Unsepcified;    break;
    case 'W': t=ElectricalType::Power_in;       break;
    case 'w': t=ElectricalType::Power_out;      break;
    case 'C': t=ElectricalType::Open_collector; break;
    case 'E': t=ElectricalType::Open_emitter;   break;
    case 'N': t=ElectricalType::Not_connected;  break;
    }
}

void PinShape::decode(QString s)
{
    if (s == ""){
        s=PinShape::Line;
    }else if(s=="I"){
        s=PinShape::Inverted;
    }else if(s=="C"){
        s=PinShape::Clock;
    }else if(s=="CI"){
        s=PinShape::Inverted_clock;
    }else if(s=="L"){
        s=PinShape::Input_low;
    }else if(s=="CL"){
        s=PinShape::Clock_low;
    }else if(s=="V"){
        s=PinShape::OutputLow;
    }else if(s=="F"){
        s=PinShape::FallingEdgeClock;
    } if(s=="X"){
        s=PinShape::NonLocgic;
    }else{
        s=PinShape::None;
    }
}
