#include "kicadfile_lib.h"
#include <QFile>
#include <QTextStream>

KICADLibSchematicDeviceDefinition::KICADLibSchematicDeviceDefinition(){

}

void KICADLibSchematicDeviceDefinition::decode(QString str){
    QStringList params = str.split(" ");
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

KICADLibSchematicDevice::KICADLibSchematicDevice(){

}

KICADLibSchematicDeviceLibrary::KICADLibSchematicDeviceLibrary()
{

}


void KICADLibSchematicDeviceLibrary::loadFile(QString fileName)
{
    this->fileName=fileName;
    QFile libFile(fileName);
    libFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&libFile);
    symbolList.clear();
    KICADLibSchematicDevice libDevice;
    while (!textStream.atEnd())
    {
        QString line = textStream.readLine();
        line = line.trimmed();
        if (line.startsWith("DEF")){
            libDevice.clear();
            libDevice.def.decode(line);

        }

        if (line.startsWith("ENDDEF")){
            symbolList.append(libDevice);
        }
    }
}

QList<KICADLibSchematicDevice> KICADLibSchematicDeviceLibrary::getSymbolList()
{
    return symbolList;
}


void KICADLibSchematicDevice::clear()
{

}
