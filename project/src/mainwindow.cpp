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

void MainWindow::onQuickLinkClicked(QString s)
{
(void)s;
}




void MainWindow::on_pushButton_2_clicked()
{
    octopartInterface.getCategorie(octopartCategorieCache,"a2f46ffe9b377933");
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
        sourceLibraryPaths.sort(Qt::CaseInsensitive);
        for(int i=0;i<sourceLibraryPaths.count();i++){
            QFileInfo fi(sourceLibraryPaths[i]);
            ui->list_input_libraries->addItem(fi.fileName());
        }
        ui->scrollQuickLink->setWidgetResizable(true);
        for(int i=0;i<selectedOctopartMPN.categories.count();i++ ){
            QStringList sl = selectedOctopartMPN.categories[i].quickLinks;
            for(int n=0;n<sl.count();n++ ){
                QLabel *lbl = new QLabel(ui->scrollAreaWidgetContents);
                QString s = sl[n];
                lbl->setText("<a href=\""+s+"\">"+s+"</a>");
                lbl->setTextFormat(Qt::RichText);
                ui->verticalLayout_3->addWidget(lbl);
                connect(lbl,SIGNAL(linkActivated(QString)),this,SLOT(onQuickLinkClicked(QString)));
            }
        }
    }else if(index == 2){
        ui->edt_targetMPN->setText(selectedOctopartMPN.mpn);
        ui->edt_targetManufacturer->setText(selectedOctopartMPN.manufacturer);
        ui->edt_targetDescription->setText(selectedOctopartMPN.description);
        ui->edt_targetName->setText(selectedOctopartMPN.mpn);
        ui->edt_targetDesignator->setText(currentDevice.def.reference);
        ui->cmb_targetFootprint->clear();
        ui->cmb_targetFootprint->addItems(fpLib.getFootprintList());

        ui->lbl_targetFootprint->setText("<a href=\""+selectedOctopartMPN.specs.value("case_package").displayValue+"\">"+selectedOctopartMPN.specs.value("case_package").displayValue+"</a>");

        ui->lbl_targetFootprint->setTextFormat(Qt::RichText);

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
        currentDevice = symList[currentRow];
    }else{
        currentDevice.clear();
    }
}

void MainWindow::setCurrentDevicePropertiesFromGui()
{
    KICADLibSchematicDeviceField deviceField;

    deviceField.clear();
    deviceField.name = "key";
    deviceField.text = QUuid::createUuid().toByteArray().toHex();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(4);
    currentDevice.setField(deviceField);

    deviceField.clear();
    deviceField.name = "footprint";
    deviceField.text = ui->cmb_targetFootprint->currentText();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(2);
    currentDevice.setField(deviceField);

    deviceField.clear();
    deviceField.name = "mpn";
    deviceField.text = ui->edt_targetMPN->text();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(5);
    currentDevice.setField(deviceField);

    deviceField.clear();
    deviceField.name = "manufacturer";
    deviceField.text = ui->edt_targetManufacturer->text();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(6);
    currentDevice.setField(deviceField);

    deviceField.clear();
    currentDevice.fields[1].visible = false;    //lets copy value field and replace it optically by disp value
    deviceField = currentDevice.fields[1];

    deviceField.name = "DisplayValue";
    deviceField.text = ui->edt_targetDisplayValue->text();
    deviceField.visible = true;
    deviceField.fieldIndex.setRawIndex(7);
    currentDevice.setField(deviceField);


    deviceField.clear();
    deviceField.name = "";
    deviceField.text = ui->edt_targetMPN->text();
    deviceField.visible = false;
    deviceField.fieldIndex.setRawIndex(1);
    currentDevice.setField(deviceField);


    currentDevice.dcmEntry.description = ui->edt_targetDescription->text();
    currentDevice.def.reference =  ui->edt_targetDesignator->text();
    currentDevice.def.name = ui->edt_targetMPN->text();
    //currentDevice.fields
}

void MainWindow::on_btnCreatePart_clicked()
{
    KICADLibSchematicDeviceLibrary targetLib;
    setCurrentDevicePropertiesFromGui();
    if (currentDevice.isValid()){
        targetLib.loadFile(libCreatorSettings.targetLibraryPath+"/sdfdf.lib");
        bool proceed = true;
        if (targetLib.indexOf(currentDevice.def.name)>-1){
            QMessageBox msgBox;
            msgBox.setText("Part duplicate");
            msgBox.setInformativeText("A part with this name already exists. Overwrite old part?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Ok);
            proceed =  QMessageBox::Ok == msgBox.exec();
        }
        if(proceed){

            targetLib.insertDevice(currentDevice);
            targetLib.saveFile(libCreatorSettings.targetLibraryPath+"/sdfdf.lib");
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
}
