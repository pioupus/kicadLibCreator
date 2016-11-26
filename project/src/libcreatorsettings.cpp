#include "libcreatorsettings.h"
#include <QSettings>

LibCreatorSettings::LibCreatorSettings()
{

}

void LibCreatorSettings::loadSettings(QString filename)
{
    QSettings settings(filename, QSettings::IniFormat);
    settingsFileName = filename;
    sourceLibraryPath = settings.value("sourceLibraryPath","/home/arne/programming/kicad_lib/kicad-library/library/").toString();
    targetLibraryPath = settings.value("targetLibraryPath","/home/arne/programming/kicad_lib/lib_ak/").toString();
    footprintLibraryPath = settings.value("footprintLibraryPath","/home/arne/programming/kicad_lib/kicad_modules/kicad-pcblib/").toString();

}

void LibCreatorSettings::saveSettings()
{
    QSettings settings(settingsFileName, QSettings::IniFormat);
    settings.setValue("sourceLibraryPath",sourceLibraryPath);
    settings.setValue("targetLibraryPath",targetLibraryPath);
    settings.setValue("footprintLibraryPath",footprintLibraryPath);

}

