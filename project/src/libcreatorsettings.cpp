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
    useFuzzyOctopartQueries = settings.value("FuzzyOctoPartQuery",true).toBool();
    useAbsolutePathForDatasheetField = settings.value("AbsolutePathForDatasheetField",true).toBool();

    settings.beginGroup("FieldDesign");
    fieldDesign_overwriteTextPosition = settings.value("overwriteTextPosition",false).toBool();
    settings.endGroup();


    settings.beginGroup("FieldDesign_DEF");
    def_draw_pin_number= settings.value("drawPinNumber",false).toBool();
    def_draw_pin_name  = settings.value("drawPinName",true).toBool();
    def_text_offset = settings.value("textOffset",2).toInt();
    settings.endGroup();

    fieldDesigns.clear();
    for (int i = 0; i<8;i++){
        FieldDesignSettingsItem item;
        bool defaultVisible = false;
        if ((i == 0) || (i == 7)){
            defaultVisible = true;
        }

        int defaultDimension = 50;

        settings.beginGroup("FieldDesign_F"+QString::number(i));
        item.index = i;
        item.position.setX(settings.value("position_x",0).toInt());
        item.position.setY(settings.value("position_y",0).toInt());
        item.dimension = settings.value("dimension",defaultDimension).toInt();
        item.orientation = (KicadSymbolFieldOrientation_t)settings.value("orientation",ksfo_horizontal).toInt();
        item.visible = settings.value("visible",defaultVisible).toBool();
        item.hjustify = (KicadSymbolFieldJustify_t)settings.value("hjustify",ksfj_left).toInt();
        item.vjustify = (KicadSymbolFieldJustify_t)settings.value("vjustify",ksfj_bottom).toInt();
        item.FontstyleItalic = settings.value("italic",false).toBool();
        item.FontstyleBold = settings.value("bold",false).toBool();
        settings.endGroup();
        fieldDesigns.append(item);
    }
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
    settings.setValue("FuzzyOctoPartQuery",useFuzzyOctopartQueries);
    settings.setValue("AbsolutePathForDatasheetField",useAbsolutePathForDatasheetField);

    settings.beginGroup("FieldDesign_DEF");
    settings.setValue("drawPinNumber",def_draw_pin_number);
    settings.setValue("drawPinName",def_draw_pin_name);
    settings.setValue("textOffset",def_text_offset);
    settings.endGroup();

    settings.beginGroup("FieldDesign");
     settings.setValue("overwriteTextPosition",fieldDesign_overwriteTextPosition);
    settings.endGroup();

    for (int i = 0; i<fieldDesigns.count();i++){
        FieldDesignSettingsItem item = fieldDesigns[i];

        settings.beginGroup("FieldDesign_F"+QString::number(item.index));

        settings.setValue("position_x", item.position.x());
        settings.setValue("position_y", item.position.y());
        settings.setValue("dimension",item.dimension );
        settings.setValue("orientation",item.orientation);
        settings.setValue("visible",item.visible);
        settings.setValue("hjustify",item.hjustify);
        settings.setValue("vjustify",item.vjustify);
        settings.setValue("italic",item.FontstyleItalic);
        settings.setValue("bold",item.FontstyleBold);
        settings.endGroup();
    }
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
            msg = "\nThere are no *.lib files in source library path. (\""+path_sourceLibrary+"\")";
        }
    }

    if (path_sourceLibrary.count()==0){
        msg += "\nSource library path is empty";
    }


    //contains lib files?
    if (!QDir(path_targetLibrary).exists()){
        msg += "\nTarget library path can't be found. (\""+path_targetLibrary+"\")";
    }
    if (path_targetLibrary.count()==0){
        msg += "\nTarget library path is empty";
    }


    if (!QDir(path_footprintLibrary).exists()){
        msg += "\nFootprint path can't be found. (\""+path_footprintLibrary+"\")";
    }
    if (path_footprintLibrary.count()==0){
        msg += "\nFootprint path is empty";
    }
    if (!QDir(path_datasheet).exists()){
        msg += "\nDatasheet path can't be found. (\""+path_datasheet+"\")";
    }
    if (path_datasheet.count()==0){
        msg += "\nDatasheet path is empty";
    }

    if (apikey.count()==0){
        msg += "\nOctopart API key is empty.";
    }

    if (msg.count()) {
        QMessageBox::critical(parent,"Wrong settings","Please check your settings. There are some issues:\n"+msg);
    }
}



FieldDesignSettingsItem::FieldDesignSettingsItem()
{
    index = 0;
    position = QPoint(0,0);
    dimension = 0;
    orientation = ksfo_horizontal;
    visible = false;
    hjustify = ksfj_center;
    vjustify = ksfj_center;
    FontstyleItalic = false;
    FontstyleBold = false;

}
