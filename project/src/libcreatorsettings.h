#ifndef LIBCREATORSETTINGS_H
#define LIBCREATORSETTINGS_H
#include <QString>
#include <QWidget>

class LibCreatorSettings
{
public:
    LibCreatorSettings();

    void loadSettings(QString filename);
    void saveSettings();

    QString path_sourceLibrary;
    QString path_targetLibrary;
    QString path_footprintLibrary;
    QString path_datasheet;
    QString path_3dmodel;

    QString apikey;

    void complainAboutSettings(QWidget* parent);

private:
    QString settingsFileName;
};

#endif // LIBCREATORSETTINGS_H
