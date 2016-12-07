#include "optionsdialog.h"
#include "ui_optionsdialog.h"


OptionsDialog::OptionsDialog(LibCreatorSettings &creatorSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
  settings(creatorSettings)
{
    ui->setupUi(this);
    ui->edt_api_key->setText( settings.apikey);
    ui->edt_datasheet_path->setText( settings.path_datasheet);
    ui->edt_footprint_path->setText(  settings.path_footprintLibrary);
    ui->edt_source_path->setText( settings.path_sourceLibrary);
    ui->edt_target_path->setText( settings.path_targetLibrary);

    if (settings.useVagueOctopartQueries){
        ui->rbt_query_vague->setChecked(true);
    }else{
        ui->rbt_query_exact->setChecked(true);
    }

}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_buttonBox_accepted()
{
    settings.apikey = ui->edt_api_key->text();
    settings.path_datasheet = ui->edt_datasheet_path->text();
    settings.path_sourceLibrary = ui->edt_source_path->text();
    settings.path_targetLibrary = ui->edt_target_path->text();
    settings.path_footprintLibrary = ui->edt_footprint_path->text();
    settings.useVagueOctopartQueries = ui->rbt_query_vague->isChecked();
    settings.saveSettings();
}
