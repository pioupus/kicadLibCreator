#ifndef LIBCREATORSETTINGS_H
#define LIBCREATORSETTINGS_H
#include <QString>

class LibCreatorSettings
{
public:
    LibCreatorSettings();

    void loadSettings(QString filename);
    void saveSettings();

    QString libraryPath;
private:
    QString settingsFileName;
};

#endif // LIBCREATORSETTINGS_H
