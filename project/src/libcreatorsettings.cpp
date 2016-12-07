#include "libcreatorsettings.h"
#include <QSettings>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

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

void LibCreatorSettings::complainAboutSettings(QWidget *parent)
{
    QString msg;
    if (!QDir(path_sourceLibrary).exists()){
        msg = "\nSource library path can't be found. (\""+path_sourceLibrary+"\")";
    }else{
        bool foundLibFile = false;
        QDirIterator it(path_sourceLibrary, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            QString name = it.next();
            QFileInfo fi(name);
            if (fi.suffix()=="lib"){
                foundLibFile = true;
                break;
            }
        }
        if (!foundLibFile){
            msg = "\nThere is no *.lib file in source library path. (\""+path_sourceLibrary+"\")";
        }
    }

    //contains lib files?
    if (!QDir(path_targetLibrary).exists()){
        msg += "\nTarget library path can't be found. (\""+path_targetLibrary+"\")";
    }
    if (!QDir(path_footprintLibrary).exists()){
        msg += "\nFootprint path can't be found. (\""+path_footprintLibrary+"\")";
    }
    if (!QDir(path_datasheet).exists()){
        msg += "\nDatasheet path can't be found. (\""+path_datasheet+"\")";
    }

    if (apikey.count()==0){
        msg += "\nOCtopart API key is empty.";
    }

    if (msg.count()) {
        QMessageBox::critical(parent,"wrong settings","please check your settings. There are some issues:\n"+msg);
    }
}

