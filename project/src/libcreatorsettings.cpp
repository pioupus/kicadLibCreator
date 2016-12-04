#include "libcreatorsettings.h"
#include <QSettings>

LibCreatorSettings::LibCreatorSettings()
{

}

void LibCreatorSettings::loadSettings(QString filename)
{
    QSettings settings(filename, QSettings::IniFormat);
    settingsFileName = filename;
    path_sourceLibrary = settings.value("sourceLibraryPath","").toString();
    path_targetLibrary = settings.value("targetLibraryPath","").toString();
    path_footprintLibrary = settings.value("footprintLibraryPath","").toString();
    path_datasheet = settings.value("datasheetPath","").toString();
    path_3dmodel = settings.value("3dModelPath","").toString();
    apikey = settings.value("OctoPartAPIkey","").toString();
}

void LibCreatorSettings::saveSettings()
{
    QSettings settings(settingsFileName, QSettings::IniFormat);
    settings.setValue("sourceLibraryPath",path_sourceLibrary);
    settings.setValue("targetLibraryPath",path_targetLibrary);
    settings.setValue("footprintLibraryPath",path_footprintLibrary);
    settings.setValue("datasheetPath",path_datasheet);
    settings.setValue("3dModelPath",path_3dmodel);
    settings.setValue("OctoPartAPIkey",apikey);

}

