#include "designsettings.h"
#include "ui_designsettings.h"
#include <QDebug>

void setCheckboxByName(QFrame *parent, int itemIndex, QString name, bool value) {
    if (parent) {
        QCheckBox *cb = parent->findChild<QCheckBox *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            cb->setChecked(value);
        } else {
            qDebug() << QString("set: could not find QCheckBox: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
}

bool getCheckboxByName(QFrame *parent, int itemIndex, QString name) {
    bool result = false;
    if (parent) {
        QCheckBox *cb = parent->findChild<QCheckBox *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            result = cb->isChecked();
        } else {
            qDebug() << QString("get: could not find QCheckBox: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
    return result;
}

void setRadioButtonByName(QFrame *parent, int itemIndex, QString name, bool value) {
    if (parent) {
        QRadioButton *cb = parent->findChild<QRadioButton *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            cb->setChecked(value);
        } else {
            qDebug() << QString("set: could not find QRadioButton: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
}

void setRadioButtonEnableByName(QFrame *parent, int itemIndex, QString name, bool value) {
    if (parent) {
        QRadioButton *cb = parent->findChild<QRadioButton *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            cb->setEnabled(value);
        } else {
            qDebug() << QString("set: could not find QRadioButton: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
}

bool getRadioButtonByName(QFrame *parent, int itemIndex, QString name) {
    bool result = false;
    if (parent) {
        QRadioButton *cb = parent->findChild<QRadioButton *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            result = cb->isChecked();
        } else {
            qDebug() << QString("get: could not find QRadioButton: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
    return result;
}

void setSpinBoxByName(QFrame *parent, int itemIndex, QString name, int value) {
    if (parent) {
        QSpinBox *cb = parent->findChild<QSpinBox *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            cb->setValue(value);
        } else {
            qDebug() << QString("set: could not find QSpinBox: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
}

void setSpinBoxEnabledByName(QFrame *parent, int itemIndex, QString name, bool value) {
    if (parent) {
        QSpinBox *cb = parent->findChild<QSpinBox *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            cb->setEnabled(value);
        } else {
            qDebug() << QString("set: could not find QSpinBox: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
}
int getSpinBoxByName(QFrame *parent, int itemIndex, QString name) {
    int result = 0;
    if (parent) {
        QSpinBox *cb = parent->findChild<QSpinBox *>("F" + QString::number(itemIndex) + "_" + name);
        if (cb) {
            result = cb->value();
        } else {
            qDebug() << QString("get: could not find QSpinBox: ") + "F" + QString::number(itemIndex) + "_" + name;
        }
    }
    return result;
}

DesignSettings::DesignSettings(LibCreatorSettings &creatorSettings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DesignSettings)
    , settings(creatorSettings) {
    ui->setupUi(this);

    ui->DEF_drawPinName->setChecked(settings.def_draw_pin_name);
    ui->DEF_drawPinNumber->setChecked(settings.def_draw_pin_number);
    ui->DEF_textOffset->setValue(settings.def_text_offset);
    ui->chb_overwrite_orig_position->setChecked(settings.fieldDesign_overwriteTextPosition);
    ui->chb_overwrite_pin_settings->setChecked(settings.fieldDesign_overwritePinSettings);

    for (int i = 0; i < settings.fieldDesigns.count(); i++) {
        FieldDesignSettingsItem item = settings.fieldDesigns[i];
        QFrame *frame = ui->toolbox->findChild<QFrame *>("frame_f" + QString::number(item.index));

        if (frame) {
            setCheckboxByName(frame, item.index, "bold", item.FontstyleBold);
            setCheckboxByName(frame, item.index, "italic", item.FontstyleItalic);
            setCheckboxByName(frame, item.index, "visible", item.visible);

            setRadioButtonByName(frame, item.index, "hjust_center", item.hjustify == ksfj_center);
            setRadioButtonByName(frame, item.index, "hjust_left", item.hjustify == ksfj_left);
            setRadioButtonByName(frame, item.index, "hjust_right", item.hjustify == ksfj_right);

            setRadioButtonByName(frame, item.index, "vjust_bot", item.vjustify == ksfj_bottom);
            setRadioButtonByName(frame, item.index, "vjust_center", item.vjustify == ksfj_center);
            setRadioButtonByName(frame, item.index, "vjust_top", item.vjustify == ksfj_top);

            setRadioButtonByName(frame, item.index, "orient_h", item.orientation == ksfo_horizontal);
            setRadioButtonByName(frame, item.index, "orient_v", item.orientation == ksfo_vertical);

            setSpinBoxByName(frame, item.index, "pos_x", item.position.x());
            setSpinBoxByName(frame, item.index, "pos_y", item.position.y());
            setSpinBoxByName(frame, item.index, "size", item.dimension);

        } else {
            qDebug() << QString("could not find QFrame: ") + "frame_f" + QString::number(item.index);
        }
    }
    setPositionEnabled();
    ui->toolbox->setCurrentIndex(1);
}

DesignSettings::~DesignSettings() {
    delete ui;
}

void DesignSettings::setPositionEnabled() {
    ui->DEF_drawPinName->setEnabled(ui->chb_overwrite_pin_settings->isChecked());
    ui->DEF_drawPinNumber->setEnabled(ui->chb_overwrite_pin_settings->isChecked());
    ui->DEF_textOffset->setEnabled(ui->chb_overwrite_pin_settings->isChecked());

    for (int i = 0; i < settings.fieldDesigns.count(); i++) {
        FieldDesignSettingsItem item = settings.fieldDesigns[i];
        QFrame *frame = ui->toolbox->findChild<QFrame *>("frame_f" + QString::number(item.index));

        if (frame) {
            setSpinBoxEnabledByName(frame, item.index, "pos_x", ui->chb_overwrite_orig_position->isChecked());
            setSpinBoxEnabledByName(frame, item.index, "pos_y", ui->chb_overwrite_orig_position->isChecked());

            setRadioButtonEnableByName(frame, item.index, "hjust_center", ui->chb_overwrite_orig_position->isChecked());
            setRadioButtonEnableByName(frame, item.index, "hjust_left", ui->chb_overwrite_orig_position->isChecked());
            setRadioButtonEnableByName(frame, item.index, "hjust_right", ui->chb_overwrite_orig_position->isChecked());

            setRadioButtonEnableByName(frame, item.index, "vjust_bot", ui->chb_overwrite_orig_position->isChecked());
            setRadioButtonEnableByName(frame, item.index, "vjust_center", ui->chb_overwrite_orig_position->isChecked());
            setRadioButtonEnableByName(frame, item.index, "vjust_top", ui->chb_overwrite_orig_position->isChecked());

            setRadioButtonEnableByName(frame, item.index, "orient_h", ui->chb_overwrite_orig_position->isChecked());
            setRadioButtonEnableByName(frame, item.index, "orient_v", ui->chb_overwrite_orig_position->isChecked());
        } else {
            qDebug() << QString("could not find QFrame: ") + "frame_f" + QString::number(item.index);
        }
    }
}

void DesignSettings::on_buttonBox_accepted() {
    settings.def_draw_pin_name = ui->DEF_drawPinName->isChecked();
    settings.def_draw_pin_number = ui->DEF_drawPinNumber->isChecked();
    settings.def_text_offset = ui->DEF_textOffset->value();
    settings.fieldDesign_overwriteTextPosition = ui->chb_overwrite_orig_position->isChecked();
    settings.fieldDesign_overwritePinSettings = ui->chb_overwrite_pin_settings->isChecked();

    for (int i = 0; i < settings.fieldDesigns.count(); i++) {
        FieldDesignSettingsItem item = settings.fieldDesigns[i];
        QFrame *frame = ui->toolbox->findChild<QFrame *>("frame_f" + QString::number(item.index));

        if (frame) {
            item.FontstyleBold = getCheckboxByName(frame, item.index, "bold");
            item.FontstyleItalic = getCheckboxByName(frame, item.index, "italic");
            item.visible = getCheckboxByName(frame, item.index, "visible");

            if (getRadioButtonByName(frame, item.index, "hjust_center")) {
                item.hjustify = ksfj_center;
            } else if (getRadioButtonByName(frame, item.index, "hjust_left")) {
                item.hjustify = ksfj_left;
            } else {
                item.hjustify = ksfj_right;
            }

            if (getRadioButtonByName(frame, item.index, "vjust_bot")) {
                item.vjustify = ksfj_bottom;
            } else if (getRadioButtonByName(frame, item.index, "vjust_center")) {
                item.vjustify = ksfj_center;
            } else {
                item.vjustify = ksfj_top;
            }

            if (getRadioButtonByName(frame, item.index, "orient_h")) {
                item.orientation = ksfo_horizontal;
            } else {
                item.orientation = ksfo_vertical;
            }

            item.position.setX(getSpinBoxByName(frame, item.index, "pos_x"));
            item.position.setY(getSpinBoxByName(frame, item.index, "pos_y"));
            item.dimension = getSpinBoxByName(frame, item.index, "size");
            settings.fieldDesigns[i] = item;
        } else {
            qDebug() << QString("could not find QFrame: ") + "frame_f" + QString::number(item.index);
        }
    }
    settings.saveSettings();
}

void DesignSettings::on_chb_overwrite_orig_position_toggled(bool checked) {
    (void)checked;
    setPositionEnabled();
}

void DesignSettings::on_chb_overwrite_pin_settings_toggled(bool checked) {
    (void)checked;
    setPositionEnabled();
}
