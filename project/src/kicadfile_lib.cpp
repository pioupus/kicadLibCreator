#include "kicadfile_lib.h"


KICADLibSchematicDeviceDefinition::KICADLibSchematicDeviceDefinition(){

}



KICADLibSchematicDeviceField::KICADLibSchematicDeviceField(){

}



KICADLibSchematicDeviceLibrary::KICADLibSchematicDeviceLibrary()
{

}


void KICADLibSchematicDeviceLibrary::loadFile(QString fileName)
{
    this->fileName=fileName;
}

QList<KICADLibSchematicDevice> KICADLibSchematicDeviceLibrary::getSymbolList()
{
    return symbolList;
}

