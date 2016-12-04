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


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    octopartInterface(OCTOPARTKEY,parent)

{
    ui->setupUi(this);
    connect(&octopartInterface, SIGNAL(octopart_request_finished()),
            this, SLOT(octopart_request_finished()));

    libCreatorSettings.loadSettings("kicadLibCreatorSettings.ini");
    fpLib.scan(libCreatorSettings.footprintLibraryPath);

    partCreationRuleList.loadFromFile("partCreationRules.ini");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event){
    octopartCategorieCache.save();
    libCreatorSettings.saveSettings();
    partCreationRuleList.saveFile("partCreationRules.ini");
    event->accept();
}


void MainWindow::on_pushButton_clicked() {

    //RC0805JR-0722KL
    octopartInterface.sendMPNQuery(octopartCategorieCache, ui->comboBox->currentText());
    queryResults.clear();
    queryResults.append(octopartInterface.octopartResult_QueryMPN);

    ui->tableOctopartResult->clear();
    ui->tableOctopartResult->setRowCount(queryResults.length());
    ui->tableOctopartResult->setColumnCount(6);
    QStringList hHeader;
    hHeader << "MPN" << "Manufacturer" << "Description" << "Footprint" << "Categories" << "Octopart";
    ui->tableOctopartResult->setHorizontalHeaderLabels(hHeader);
    for(int i=0;i<queryResults.length();i++){
        QTableWidgetItem *newMPNItem = new QTableWidgetItem(queryResults[i].mpn);
        QTableWidgetItem *newManufacturerItem = new QTableWidgetItem(queryResults[i].manufacturer);
        QTableWidgetItem *newFootprint = new QTableWidgetItem(queryResults[i].footprint);
        QTableWidgetItem *newCategorie = new QTableWidgetItem("");
        QTableWidgetItem *newDescription = new QTableWidgetItem(queryResults[i].description);
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
        newCategorie->setText(categorie_str);
        ui->tableOctopartResult->setItem(i,0,newMPNItem);
        ui->tableOctopartResult->setItem(i,1,newManufacturerItem);
        ui->tableOctopartResult->setItem(i,2,newDescription);
        ui->tableOctopartResult->setItem(i,3,newFootprint);
        ui->tableOctopartResult->setItem(i,4,newCategorie);
        ui->tableOctopartResult->setItem(i,5,newOctopartURL);

    }
    ui->tableOctopartResult->resizeColumnsToContents();
    ui->tableOctopartResult->resizeRowsToContents();
    //octopartInterface.sendOctoPartRequest("SN74S74N");

}

void MainWindow::octopart_request_finished()
{

}


void MainWindow::on_tableOctopartResult_cellActivated(int row, int column)
{
    selectedOctopartMPN = queryResults[row];
    (void)column;
}



void MainWindow::on_pushButton_2_clicked()
{
    octopartInterface.getCategorie(octopartCategorieCache,"a2f46ffe9b377933");
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
    if (index == 1){
        sourceLibraryPaths.clear();
        QDirIterator it(libCreatorSettings.sourceLibraryPath, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            QString name = it.next();
            QFileInfo fi(name);
            if (fi.suffix()=="lib"){
                sourceLibraryPaths.append(name);

            }

        }
        ui->cmb_targetRuleName->setCurrentText("");
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

    }else if(index == 2){

        ui->cmb_targetFootprint->clear();
        ui->cmb_targetFootprint->addItems(fpLib.getFootprintList());

        QString selectedRule = ui->cmb_targetRuleName->currentText();
        ui->cmb_targetRuleName->clear();
        ui->cmb_targetRuleName->addItems(partCreationRuleList.namesWithoutGlobal);
        ui->cmb_targetRuleName->setCurrentText(selectedRule);
        on_btn_applyRule_clicked();
    }
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



void MainWindow::on_btnCreatePart_clicked()
{
    KICADLibSchematicDeviceLibrary targetLib;
    setCurrentDevicePropertiesFromGui();
    QFileInfo fi(ui->cmb_targetLibrary->currentText());
    QString targetLibName;
    if (fi.suffix() == "lib"){
        targetLibName= libCreatorSettings.targetLibraryPath+QDir::separator()+ui->cmb_targetLibrary->currentText();
    }else{
        targetLibName= libCreatorSettings.targetLibraryPath+QDir::separator()+ui->cmb_targetLibrary->currentText()+".lib";
    }

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
    }

}

QMap<QString, QString> MainWindow::createVariableMap(){
    QMap<QString, QString> variables = selectedOctopartMPN.getQueryResultMap();
    variables.insert("%source.footprint%",currentSourceDevice.fields[3].text);
    variables.insert("%source.ref%",currentSourceDevice.def.reference);

    variables.insert("%target.id%",QString::number(QDateTime::currentMSecsSinceEpoch()));

    QMapIterator<QString, QString> ii(variables);

    while (ii.hasNext()) {
        ii.next();
        qDebug() << ii.key() << ": " << ii.value();
    }
    return variables;
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
        ui->edt_targetDatasheet->setText(  creationRuleResult.datasheet);
        ui->edt_targetID->setText( creationRuleResult.id);
        ui->edt_targetMPN->setText( creationRuleResult.mpn);
        ui->edt_targetManufacturer->setText( creationRuleResult.manufacturer);
        ui->edt_targetDescription->setText( creationRuleResult.description);
        ui->edt_targetDisplayValue->setText( creationRuleResult.display_value);
        ui->cmb_targetLibrary->setCurrentText( creationRuleResult.lib_name);

    }

}
