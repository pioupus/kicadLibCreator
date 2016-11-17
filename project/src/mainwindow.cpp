#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <QJsonDocument>
#include <QDirIterator>
#include "kicadfile_lib.h"
#include "../octopartkey.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    octopartInterface(OCTOPARTKEY,parent)

{
    ui->setupUi(this);
    connect(&octopartInterface, SIGNAL(octopart_request_finished()),
            this, SLOT(octopart_request_finished()));

    libCreatorSettings.loadSettings("kicadLibCreatorSettings.ini");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event){
    octopartCategorieCache.save();
    libCreatorSettings.saveSettings();
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
        libraryPaths.clear();
        QDirIterator it(libCreatorSettings.libraryPath, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            QString name = it.next();
            QFileInfo fi(name);
            if (fi.suffix()=="lib"){
                libraryPaths.append(name);

            }

        }
        libraryPaths.sort(Qt::CaseInsensitive);
        for(int i=0;i<libraryPaths.count();i++){
            QFileInfo fi(libraryPaths[i]);
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

    }
}

void MainWindow::on_list_input_libraries_currentRowChanged(int currentRow)
{
    qDebug() << "row" << currentRow;
    qDebug() << "rowcount" << libraryPaths.count();
    KICADLibSchematicDeviceLibrary schemDevLib;

    schemDevLib.loadFile(libraryPaths[currentRow]);

    QList<KICADLibSchematicDevice> symList = schemDevLib.getSymbolList();
    ui->list_input_devices->clear();
    ui->list_input_devices->setCurrentRow(0);
    for(int i=0;i<symList.count();i++){
        ui->list_input_devices->addItem(symList[i].def.name);
    }

}

