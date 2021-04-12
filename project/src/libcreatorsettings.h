#ifndef LIBCREATORSETTINGS_H
#define LIBCREATORSETTINGS_H
#include "kicadfile_lib.h"
#include <QRegularExpression>
#include <QString>
#include <QWidget>

class Supplier {
    public:
    enum Type { Digikey, Farnell, None };
    Supplier(Type t)
        : m_t(t) {}
    Supplier(QString sku) {
        if (QRegularExpression("\\b\\d{6,7}\\b").match(sku).hasMatch()) {
            m_t = Type::Farnell;
        } else if (QRegularExpression(".*-ND").match(sku).hasMatch()) {
            m_t = Type::Digikey;
        } else {
            m_t = Type::None;
        }
    }

    Supplier()
        : m_t(Type::None) {}
    Type type() const {
        return m_t;
    }
    QString toStr() const {
        switch (m_t) {
            case Type::Digikey:
                return "digikey";
            case Type::Farnell:
                return "farnell";
            case Type::None:
                return "none";
        }
        return "";
    }

    private:
    Type m_t;
};

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

    QString digikey_secret;
    QString digikey_clientID;
    int digikey_url;
    QString get_digikey_url_string() const;
    QString get_digikey_clientID() const;
    QString get_digikey_secret() const;

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
