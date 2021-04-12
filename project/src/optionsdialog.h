#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "libcreatorsettings.h"
#include <QDialog>

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog {
    Q_OBJECT

    public:
    explicit OptionsDialog(LibCreatorSettings &creatorSettings, QWidget *parent = 0);
    ~OptionsDialog();

    void setSettings(LibCreatorSettings settings);
    private slots:
    void on_buttonBox_accepted();

    private:
    Ui::OptionsDialog *ui;
    LibCreatorSettings &settings;
};

#endif // OPTIONSDIALOG_H
