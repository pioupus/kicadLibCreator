#ifndef LIBCREATORSETTINGS_H
#define LIBCREATORSETTINGS_H
#include "kicadfile_lib.h"
#include <QString>
#include <QWidget>

class LibCreatorSettings {
    public:
    LibCreatorSettings();

    void loadSettings(QString filename);
    void saveSettings();

    bool useFuzzyOctopartQueries;
    bool useAbsolutePathForDatasheetField;

    QString path_sourceLibrary;
    QString path_targetLibrary;
    QString path_footprintLibrary;
    QString path_datasheet;
    QString path_3dmodel;

    bool def_draw_pin_number;
    bool def_draw_pin_name;
    int def_text_offset;

    bool fieldDesign_overwriteTextPosition;
    bool fieldDesign_overwritePinSettings;

    bool useHashAsDatasheetFilename;
    QString apikey;
    QList<FieldDesignSettingsItem> fieldDesigns;

    void complainAboutSettings(QWidget *parent);

    private:
    QString settingsFileName;
};

#endif // LIBCREATORSETTINGS_H
