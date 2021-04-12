#ifndef DESIGNSETTINGS_H
#define DESIGNSETTINGS_H

#include "libcreatorsettings.h"
#include <QDialog>

namespace Ui {
    class DesignSettings;
}

class DesignSettings : public QDialog {
    Q_OBJECT

    public:
    explicit DesignSettings(LibCreatorSettings &creatorSettings, QWidget *parent = 0);

    ~DesignSettings();

    private:
    void setPositionEnabled();
    private slots:
    void on_buttonBox_accepted();

    void on_chb_overwrite_orig_position_toggled(bool checked);

    void on_chb_overwrite_pin_settings_toggled(bool checked);

    private:
    Ui::DesignSettings *ui;
    LibCreatorSettings &settings;
};

#endif // DESIGNSETTINGS_H
