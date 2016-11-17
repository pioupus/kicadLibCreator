#include "libcreatorsettings.h"
#include <QSettings>

LibCreatorSettings::LibCreatorSettings()
{

}

void LibCreatorSettings::loadSettings(QString filename)
{
    QSettings settings(filename, QSettings::IniFormat);
    settingsFileName = filename;
    libraryPath = settings.value("libraryPath","/home/arne/programming/kicad_lib/kicad-library/library/").toString();
}

void LibCreatorSettings::saveSettings()
{
    QSettings settings(settingsFileName, QSettings::IniFormat);
    settings.setValue("libraryPath",libraryPath);
}

