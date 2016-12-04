#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <QJsonDocument>
#include <QDirIterator>
#include <QUuid>
#include <QPalette>
#include "kicadfile_lib.h"
#include "../octopartkey.h"
#include "ruleeditor.h"
#include "optionsdialog.h"
#include <QDesktopServices>
#include <QCompleter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    octopartInterface("",parent)

{
    ui->setupUi(this);
    connect(&octopartInterface, SIGNAL(octopart_request_finished()),
            this, SLOT(octopart_request_finished()));

    connect(ui->comboBox->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(oncmbOctoQueryEnterPressed()));

    libCreatorSettings.loadSettings("kicadLibCreatorSettings.ini");
    fpLib.scan(libCreatorSettings.path_footprintLibrary);
    octopartInterface.setAPIKey(libCreatorSettings.apikey);
    partCreationRuleList.loadFromFile("partCreationRules.ini");
    querymemory.loadQueryList(ui->comboBox);
    // ui->lblSpinner->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event){
    octopartCategorieCache.save();
    libCreatorSettings.saveSettings();
    querymemory.save();
    partCreationRuleList.saveFile("partCreationRules.ini");
    event->accept();
}


void MainWindow::on_pushButton_clicked() {

    //RC0805JR-0722KL
    resetSearchQuery(true);
    octopartInterface.sendMPNQuery(octopartCategorieCache, ui->comboBox->currentText());
    queryResults.clear();
    queryResults.append(octopartInterface.octopartResult_QueryMPN);

    ui->tableOctopartResult->setRowCount(queryResults.length());

    for(int i=0;i<queryResults.length();i++){
        QTableWidgetItem *newMPNItem = new QTableWidgetItem(queryResults[i].mpn);
        QTableWidgetItem *newManufacturerItem = new QTableWidgetItem(queryResults[i].manufacturer);
        QTableWidgetItem *newFootprint = new QTableWidgetItem(queryResults[i].footprint);
        QTableWidgetItem *newCategorie = new QTableWidgetItem("");
        QTableWidgetItem *newDescription = new QTableWidgetItem(queryResults[i].description);
        QTableWidgetItem *newExtras = new QTableWidgetItem("");
        QTableWidgetItem *newOctopartURL = new QTableWidgetItem("link");
        newOctopartURL->setData(Qt::UserRole,queryResults[i].urlOctoPart);
        newMPNItem->setData(Qt::UserRole,i);
        QString categorie_str;
        for(int n=0;n<queryResults[i].categories.count();n++){
            for(int m=0;m<queryResults[i].categories[n].categorieNameTree.count();m++){
                categorie_str += queryResults[i].categories[n].categorieNameTree[m] +", ";
            }
            categorie_str += "\n";
        }
        QString ds;
        if (queryResults[i].urlDataSheet == ""){
            ds = "Datasheets: no\n";
        }else{
            ds = "Datasheets: yes\n";
        }
        if (queryResults[i].url3DModel == ""){
            ds += "3DModel: no";
        }else{
            ds += "3DModel: yes";
        }
        newExtras->setText(ds);
        newCategorie->setText(categorie_str);
        ui->tableOctopartResult->setItem(i,0,newMPNItem);
        ui->tableOctopartResult->setItem(i,1,newManufacturerItem);
        ui->tableOctopartResult->setItem(i,2,newDescription);
        ui->tableOctopartResult->setItem(i,3,newFootprint);
        ui->tableOctopartResult->setItem(i,4,newCategorie);
        ui->tableOctopartResult->setItem(i,5,newExtras);
        ui->tableOctopartResult->setItem(i,6,newOctopartURL);

    }
    ui->tableOctopartResult->resizeColumnsToContents();
    ui->tableOctopartResult->resizeRowsToContents();
    //octopartInterface.sendOctoPartRequest("SN74S74N");

}

void MainWindow::octopart_request_finished()
{

}

void MainWindow::on_tableOctopartResult_cellDoubleClicked(int row, int column)
{
    if (column == 6){
        if (row < queryResults.count()){
            QDesktopServices::openUrl(QUrl(queryResults[row].urlOctoPart));
        }
    }else{
        ui->tabWidget->setCurrentIndex(1);
    }
}



void MainWindow::on_tableOctopartResult_cellActivated(int row, int column)
{
    resetSearchQuery(false);
    if (row < queryResults.count()){
        selectedOctopartMPN = queryResults[row];
    }
    (void)column;
}

void MainWindow::on_comboBox_editTextChanged(const QString &arg1)
{
    (void)arg1;
    if (!ui->comboBox->property("ignoreChanges").toBool()){
        resetSearchQuery(true);
    }
}

void MainWindow::resetSearchQuery(bool resetAlsoTable)
{
    if (resetAlsoTable){
        ui->tableOctopartResult->clear();
        ui->tableOctopartResult->setRowCount(0);
        ui->tableOctopartResult->setColumnCount(7);
        QStringList hHeader;
        hHeader << "MPN" << "Manufacturer" << "Description" << "Footprint" << "Categories" << "Extras" << "Octopart";
        ui->tableOctopartResult->setHorizontalHeaderLabels(hHeader);
    }
    selectedOctopartMPN.clear();
    currentSourceDevice.clear();
    ui->cmb_targetFootprint->setCurrentIndex(-1);
    ui->cmb_targetLibrary->setCurrentIndex(-1);
    ui->cmb_targetRuleName->setCurrentIndex(-1);
    ui->edt_targetDatasheet->setText("");
    ui->edt_targetDescription->setText("");
    ui->edt_targetDesignator->setText("");
    ui->edt_targetDisplayValue->setText("");
    ui->edt_targetID->setText("");
    ui->edt_targetManufacturer->setText("");
    ui->edt_targetMPN->setText("");
    ui->edt_targetName->setText("");

}

void MainWindow::clearQuickLinks(QLayout *layout)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
            delete widget;
        if (QLayout* childLayout = item->layout())
            clearQuickLinks(childLayout);
        delete item;
    }
}



void MainWindow::on_tabWidget_currentChanged(int index)
{
     if (index == 0){
         querymemory.loadQueryList(ui->comboBox);
    }else if (index == 1){
        if (ui->tableOctopartResult->currentRow() == -1){
            if (queryResults.count() == 1){
                selectedOctopartMPN = queryResults[0];
            }
        }
        if (selectedOctopartMPN.mpn == ""){
            ui->tabWidget->setCurrentIndex(0);
        }else{
            querymemory.addQuery(selectedOctopartMPN.mpn);
            sourceLibraryPaths.clear();
            QDirIterator it(libCreatorSettings.path_sourceLibrary, QDirIterator::NoIteratorFlags);
            while (it.hasNext()) {
                QString name = it.next();
                QFileInfo fi(name);
                if (fi.suffix()=="lib"){
                    sourceLibraryPaths.append(name);
                }
            }
            //ui->cmb_targetRuleName->setCurrentText("");
            sourceLibraryPaths.sort(Qt::CaseInsensitive);
            for(int i=0;i<sourceLibraryPaths.count();i++){
                QFileInfo fi(sourceLibraryPaths[i]);
                ui->list_input_libraries->addItem(fi.fileName());
            }
            ui->scrollQuickLink->setWidgetResizable(true);


            clearQuickLinks(ui->verticalLayout_3);

            QList<PartCreationRule> possibleRules = partCreationRuleList.findRuleByCategoryID(selectedOctopartMPN.categories);

            for (auto possibleRule : possibleRules){
                QLabel *lbl = new QLabel(ui->scrollAreaWidgetContents);
                auto sl = possibleRule.links_source_device;
                for (auto s : sl){
                    QString ruleName = possibleRule.name;
                    lbl->setText("<a href=\""+s+"/"+ruleName+"\">"+s+" ("+ruleName+")</a>");
                    lbl->setTextFormat(Qt::RichText);
                    ui->verticalLayout_3->addWidget(lbl);
                    connect(lbl,SIGNAL(linkActivated(QString)),this,SLOT(onQuickLinkClicked(QString)));
                }
            }
        }

    }else if(index == 2){
        if (currentSourceDevice.def.name.count() == 0){
            ui->tabWidget->setCurrentIndex(1);
        }else{
            if (selectedOctopartMPN.mpn == ""){
                ui->tabWidget->setCurrentIndex(0);
            }else{
                querymemory.addQuery(selectedOctopartMPN.mpn);
                ui->cmb_targetFootprint->clear();
                ui->cmb_targetFootprint->addItems(fpLib.getFootprintList());
                ui->lblSourceDevice->setText(currentSourceLib.getName()+"/"+currentSourceDevice.def.name);

                QStringList targetLibList;
                ui->cmb_targetLibrary->clear();
                QDirIterator it(libCreatorSettings.path_targetLibrary, QDirIterator::NoIteratorFlags);
                while (it.hasNext()) {
                    QString name = it.next();
                    QFileInfo fi(name);
                    if (fi.suffix()=="lib"){
                        targetLibList.append(name);
                    }
                }
                targetLibList.sort(Qt::CaseInsensitive);
                for(int i=0;i<targetLibList.count();i++){
                    QFileInfo fi(targetLibList[i]);
                    ui->cmb_targetLibrary->addItem(fi.fileName());
                }

                loadRuleCombobox();
                on_btn_applyRule_clicked();
            }
        }
    }
}

void  MainWindow::loadRuleCombobox(){
    QString selectedRule = ui->cmb_targetRuleName->currentText();
    ui->cmb_targetRuleName->clear();
    ui->cmb_targetRuleName->addItems(partCreationRuleList.namesWithoutGlobal);
    ui->cmb_targetRuleName->setCurrentText(selectedRule);


}



void MainWindow::onQuickLinkClicked(QString s)
{
    bool found = true;
    auto sl = s.split("/");
    auto devLibFinds = ui->list_input_libraries->findItems(sl[0],Qt::MatchExactly);
    if (devLibFinds.count()){
        auto devLib = devLibFinds[0];
        ui->list_input_libraries->setCurrentItem(devLib);
    }else{
        found = false;
    }

    if (found){
        auto devFinds = ui->list_input_devices->findItems(sl[1],Qt::MatchExactly);
        if (devFinds.count()){
            auto dev = devFinds[0];
            ui->list_input_devices->setCurrentItem(dev);
            on_list_input_devices_currentRowChanged(ui->list_input_devices->currentRow());
        }else{
            found = false;
        }
    }
    if (found){
        ui->cmb_targetRuleName->setCurrentText(sl[2]);
        ui->tabWidget->setCurrentIndex(2);
    }
}





void MainWindow::on_list_input_libraries_currentRowChanged(int currentRow)
{
    qDebug() << "row" << currentRow;
    qDebug() << "rowcount" << sourceLibraryPaths.count();


    currentSourceLib.loadFile(sourceLibraryPaths[currentRow]);

    QList<KICADLibSchematicDevice> symList = currentSourceLib.getSymbolList();
    ui->list_input_devices->clear();
    ui->list_input_devices->setCurrentRow(0);
    for(int i=0;i<symList.count();i++){
        ui->list_input_devices->addItem(symList[i].def.name);
    }

}


void MainWindow::on_list_input_devices_currentRowChanged(int currentRow)
{
    QList<KICADLibSchematicDevice> symList = currentSourceLib.getSymbolList();
    if ((currentRow < symList.count()) && (currentRow >= 0)){
        currentSourceDevice = symList[currentRow];
    }else{
        currentSourceDevice.clear();
    }
}

void MainWindow::setCurrentDevicePropertiesFromGui()
{
    KICADLibSchematicDeviceField deviceField;

    deviceField.clear();
    deviceField.name = "key";
    deviceField.text = ui->edt_targetID->text();//QUuid::createUuid().toByteArray().toHex();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(4);
    currentSourceDevice.setField(deviceField);

    deviceField.clear();
    deviceField.name = "footprint";
    deviceField.text = ui->cmb_targetFootprint->currentText();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(2);
    currentSourceDevice.setField(deviceField);

    deviceField.clear();
    deviceField.name = "datasheet";
    deviceField.text = ui->edt_targetDatasheet->text();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(3);
    currentSourceDevice.setField(deviceField);

    deviceField.clear();
    deviceField.name = "mpn";
    deviceField.text = ui->edt_targetMPN->text();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(5);
    currentSourceDevice.setField(deviceField);

    deviceField.clear();
    deviceField.name = "manufacturer";
    deviceField.text = ui->edt_targetManufacturer->text();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(6);
    currentSourceDevice.setField(deviceField);

    deviceField.clear();
    currentSourceDevice.fields[1].visible = false;    //lets copy value field and replace it optically by disp value
    deviceField = currentSourceDevice.fields[1];

    deviceField.name = "DisplayValue";
    deviceField.text = ui->edt_targetDisplayValue->text();
    deviceField.visible = true;
    deviceField.fieldIndex.setRawIndex(7);
    currentSourceDevice.setField(deviceField);


    deviceField.clear();
    deviceField.name = "";
    deviceField.text = ui->edt_targetMPN->text();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(1);
    currentSourceDevice.setField(deviceField);


    currentSourceDevice.fpList.clear();//we want our own footprints..

    currentSourceDevice.dcmEntry.description = ui->edt_targetDescription->text();
    currentSourceDevice.def.reference =  ui->edt_targetDesignator->text();
    currentSourceDevice.def.name = ui->edt_targetMPN->text();
}

void MainWindow::downloadDatasheet(bool force){
    RESTRequest restRequester;
    QBuffer buf;
    QMultiMap<QString, QString> params;

    QString targetpath=libCreatorSettings.path_datasheet+QDir::separator()+ui->edt_targetDatasheet->text();
    targetpath=targetpath.replace(QString(QDir::separator())+QDir::separator(),QDir::separator());

    QFileInfo fi(targetpath);
    QString p = fi.absolutePath();
    if (force || !fi.exists()){
        ui->pushButton_3->setText("downloading..");
        QDir().mkpath(p);
        restRequester.startRequest(selectedOctopartMPN.urlDataSheet,params,&buf);
        QFile file(targetpath);
        ui->pushButton_3->setText("download");

        if (file.open(QIODevice::WriteOnly)){
            buf.open(QIODevice::ReadOnly);
            file.write(buf.readAll());
            file.close();

        }else{
            qDebug() << "couldnt open file";
        }
    }
    setDatasheetButton();
}

void MainWindow::on_pushButton_3_clicked()
{
downloadDatasheet(true);
}

void MainWindow::on_btnCreatePart_clicked()
{
    KICADLibSchematicDeviceLibrary targetLib;
    setCurrentDevicePropertiesFromGui();
    QFileInfo fi(ui->cmb_targetLibrary->currentText());
    QString targetLibName;
    if (fi.suffix() == "lib"){
        targetLibName= libCreatorSettings.path_targetLibrary+QDir::separator()+ui->cmb_targetLibrary->currentText();
    }else{
        targetLibName= libCreatorSettings.path_targetLibrary+QDir::separator()+ui->cmb_targetLibrary->currentText()+".lib";
    }

    downloadDatasheet(false);
    if (currentSourceDevice.isValid()){
        targetLib.loadFile(targetLibName);
        bool proceed = true;
        if (targetLib.indexOf(currentSourceDevice.def.name)>-1){
            QMessageBox msgBox;
            msgBox.setText("Part duplicate");
            msgBox.setInformativeText("A part with this name already exists. Overwrite old part?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Ok);
            proceed =  QMessageBox::Ok == msgBox.exec();
        }
        if(proceed){

            targetLib.insertDevice(currentSourceDevice);
            targetLib.saveFile(targetLibName);
        }
    }else{
        QMessageBox::warning(this, "current device invalid", "current device invalid");
    }
}



void MainWindow::on_lbl_targetFootprint_linkActivated(const QString &link)
{
    ui->cmb_targetFootprint->setCurrentText(link);
}


void MainWindow::on_cmb_targetFootprint_currentTextChanged(const QString &arg1)
{
    if (ui->cmb_targetFootprint->findText(arg1) == -1){
        QPalette pal = ui->edt_targetDescription->palette();
        pal.setColor(ui->cmb_targetFootprint->backgroundRole(), Qt::red);
        pal.setColor(ui->cmb_targetFootprint->foregroundRole(), Qt::red);
        ui->cmb_targetFootprint->setPalette(pal);
    }else{
        QPalette pal = ui->edt_targetDescription->palette();
        ui->cmb_targetFootprint->setPalette(pal);
    }
}



QMap<QString, QString> MainWindow::createVariableMap(){
    QMap<QString, QString> variables = selectedOctopartMPN.getQueryResultMap();
    QString fp = currentSourceDevice.getFieldbyIndex(3).text;
    if (fp.count()){
        variables.insert("%source.footprint%",currentSourceDevice.getFieldbyIndex(3).text);
    }
    if (currentSourceDevice.def.reference.count()){
        variables.insert("%source.ref%",currentSourceDevice.def.reference);
    }
    variables.insert("%target.id%",QString::number(QDateTime::currentMSecsSinceEpoch()));

    QMapIterator<QString, QString> ii(variables);


    return variables;
}

void MainWindow::on_actionEdit_triggered()
{
    RuleEditor ruleeditor(this);
    QStringList proposedCategories;
    QStringList proposedSourceDevices;
    ruleeditor.setRules(partCreationRuleList,  proposedCategories,  proposedSourceDevices);
    ruleeditor.exec();
    if(ruleeditor.result()){
        partCreationRuleList = ruleeditor.getRules();
        partCreationRuleList.modified();
        loadRuleCombobox();

    }

}

void MainWindow::on_btn_editRule_clicked()
{
    RuleEditor ruleeditor(this);
    QStringList proposedCategories;
    QStringList proposedSourceDevices;

    proposedSourceDevices << currentSourceLib.getName() + "/"+currentSourceDevice.def.name;
    for (auto cat: selectedOctopartMPN.categories){
        QString name = cat.categorie_uid+"~";
        for (auto str: cat.categorieNameTree){
            name += str+"/";
        }
        proposedCategories.append(name) ;
    }


    ruleeditor.setRules(partCreationRuleList,  proposedCategories,  proposedSourceDevices);
    ruleeditor.setCurrenRule(ui->cmb_targetRuleName->currentText());
    ruleeditor.setVariables(createVariableMap());
    ruleeditor.exec();
    if(ruleeditor.result()){
        partCreationRuleList = ruleeditor.getRules();
        partCreationRuleList.modified();
        loadRuleCombobox();
    }
}

void MainWindow::on_btn_applyRule_clicked()
{
    targetDevice = currentSourceDevice;
    PartCreationRule currentRule =  partCreationRuleList.getRuleByNameForAppliaction(ui->cmb_targetRuleName->currentText());

    if (currentRule.name != ""){

        QMap<QString, QString> variables = createVariableMap();

        PartCreationRuleResult creationRuleResult = currentRule.setKicadDeviceFieldsByRule(variables);

        ui->edt_targetDesignator->setText(creationRuleResult.designator);
        ui->edt_targetName->setText( creationRuleResult.name);
        ui->cmb_targetFootprint->setCurrentText( creationRuleResult.footprint);
        ui->lbl_targetFootprint->setText("<a href="+ creationRuleResult.footprint+">"+creationRuleResult.footprint+"</a>");
        ui->edt_targetDatasheet->setText( creationRuleResult.datasheet);
        ui->edt_targetID->setText( creationRuleResult.id);
        ui->edt_targetMPN->setText( creationRuleResult.mpn);
        ui->edt_targetManufacturer->setText( creationRuleResult.manufacturer);
        ui->edt_targetDescription->setText( creationRuleResult.description);
        ui->edt_targetDisplayValue->setText( creationRuleResult.display_value);
        ui->cmb_targetLibrary->setCurrentText( creationRuleResult.lib_name);

    }

}

void MainWindow::oncmbOctoQueryEnterPressed()
{
    on_pushButton_clicked();
}


void MainWindow::setDatasheetButton()
{
    QString targetpath=libCreatorSettings.path_datasheet+QDir::separator()+ui->edt_targetDatasheet->text();
    targetpath=targetpath.replace(QString(QDir::separator())+QDir::separator(),QDir::separator());
    QFileInfo fi(targetpath);
    ui->btn_show_datasheet->setEnabled(fi.exists());
}




void MainWindow::on_edt_targetDatasheet_textChanged(const QString &arg1)
{
    (void)arg1;
    setDatasheetButton();
}

void MainWindow::on_btn_show_datasheet_clicked()
{
    QString targetpath=libCreatorSettings.path_datasheet+QDir::separator()+ui->edt_targetDatasheet->text();
    targetpath=targetpath.replace(QString(QDir::separator())+QDir::separator(),QDir::separator());

    QDesktopServices::openUrl(QUrl(targetpath));
}

void MainWindow::on_actionOptions_triggered()
{
    OptionsDialog diag(libCreatorSettings);
    diag.exec();
    octopartInterface.setAPIKey(libCreatorSettings.apikey);
}




