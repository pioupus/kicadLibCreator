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
#include "ruleeditor.h"
#include "optionsdialog.h"
#include <QDesktopServices>
#include <QCompleter>
#include <assert.h>
#include <QCryptographicHash>
#include "textfile.h"
#include "designsettings.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    octopartInterface("",parent)

{
    ui->setupUi(this);
    connect(ui->comboBox->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(oncmbOctoQueryEnterPressed()));

    connect(&octopartInterface, SIGNAL(octopart_request_finished()),
            this, SLOT(octopart_request_finished()));

    connect(&octopartInterface, SIGNAL(octopart_request_started()),
            this, SLOT(octopart_request_started()));

    connect(&octopartInterface, SIGNAL(setProgressbar(int,int)),
            this, SLOT(setProgressbar(int, int)));

    libCreatorSettings.loadSettings("kicadLibCreatorSettings.ini");
    fpLib.scan(libCreatorSettings.path_footprintLibrary);

    octopartInterface.setAPIKey(libCreatorSettings.apikey);
    partCreationRuleList.loadFromFile("partCreationRules.ini");
    querymemory.loadQueryList(ui->comboBox);
    libCreatorSettings.complainAboutSettings(this);
    progressbar = new QProgressBar(ui->statusBar);
    progressbar->setVisible(false);
    ui->statusBar->addPermanentWidget(progressbar);
    selectedOctopartMPN.setDebugPrintMpn(true);
    // ui->lblSpinner->setVisible(false);
    ui->cbtn_exact_match->setChecked(!libCreatorSettings.useFuzzyOctopartQueries);
}

void MainWindow::setProgressbar(int progress, int total)
{

    progressbar->setValue(progress);
    progressbar->setMaximum(total);
}

void MainWindow::octopart_request_started(){
    progressbar->setValue(0);
    progressbar->setMaximum(100);
    progressbar->setVisible(true);
    ui->statusBar->showMessage("Sending request..", 500);
}

void MainWindow::octopart_request_finished()
{
    progressbar->setVisible(false);
  //  qDebug() << "httpFinished";
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


    resetSearchQuery(true);
    octopartInterface.sendMPNQuery(octopartCategorieCache, ui->comboBox->currentText(),!ui->cbtn_exact_match->isChecked());
    queryResults.clear();
    queryResults.append(octopartInterface.octopartResult_QueryMPN);

    ui->tableOctopartResult->setRowCount(queryResults.length());

    for(int i=0;i<queryResults.length();i++){
        QTableWidgetItem *newMPNItem = new QTableWidgetItem(queryResults[i].getMpn());
        QTableWidgetItem *newManufacturerItem = new QTableWidgetItem(queryResults[i].manufacturer);
        QTableWidgetItem *newFootprint = new QTableWidgetItem(queryResults[i].footprint);
        QTableWidgetItem *newCategorie = new QTableWidgetItem("");
        QTableWidgetItem *newDescription = new QTableWidgetItem(queryResults[i].description);
        QTableWidgetItem *newExtras = new QTableWidgetItem("");
        QTableWidgetItem *newOctopartURL = new QTableWidgetItem("link");
        QTableWidgetItem *newProductURL = new QTableWidgetItem("");
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
        if (queryResults[i].urlProduct.count()){
            newProductURL->setText("link");
        }else{
            newProductURL->setText("no link");
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
        ui->tableOctopartResult->setItem(i,7,newProductURL);

    }
    ui->tableOctopartResult->resizeColumnsToContents();
    ui->tableOctopartResult->resizeRowsToContents();
    //octopartInterface.sendOctoPartRequest("SN74S74N");

}
void MainWindow::openHttpLink(QString url)
{
    if (url.count()){
        ui->statusBar->showMessage("Open Browser..", 2000);
        QDesktopServices::openUrl(QUrl(url));
    }
}

void MainWindow::tableOctopartResult_linkclicked(int row, int column)
{
    if (column == 6){
        if (row < queryResults.count()){
            openHttpLink(queryResults[row].urlOctoPart);
        }
    }else if(column == 7){
        if (row < queryResults.count()){
            openHttpLink(queryResults[row].urlProduct);
        }
    }
}



void MainWindow::on_tableOctopartResult_cellDoubleClicked(int row, int column)
{
    if (column >= 6){
        tableOctopartResult_linkclicked(row,column);
    }else{
        ui->tabWidget->setCurrentIndex(1);
    }
}

void MainWindow::on_tableOctopartResult_cellClicked(int row, int column)
{
    tableOctopartResult_linkclicked(row,column);

}

void MainWindow::on_lbl_productlink_linkActivated(const QString &link)
{
    openHttpLink(link);
}

void MainWindow::on_tableOctopartResult_cellActivated(int row, int column)
{

    (void)column;
    (void)row;
}

void MainWindow::on_tableOctopartResult_itemSelectionChanged()
{
    resetSearchQuery(false);
    int row = ui->tableOctopartResult->currentRow();
   // qDebug() << "row selected:"<<row<<"queryResults.count():" << queryResults.count();
    if (row < queryResults.count()){
        selectedOctopartMPN.copyFrom(queryResults[row]);
    }
}

void MainWindow::on_comboBox_editTextChanged(const QString &arg1)
{
    (void)arg1;
    if (!ui->comboBox->property("ignoreChanges").toBool()){
        resetSearchQuery(true);
    }
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1)
{
    (void)arg1;
    //on_comboBox_editTextChanged(arg1);
}

void MainWindow::resetSearchQuery(bool resetAlsoTable)
{
    if (resetAlsoTable){
        ui->tableOctopartResult->clear();
        ui->tableOctopartResult->setRowCount(0);
        ui->tableOctopartResult->setColumnCount(8);
        QStringList hHeader;
        hHeader << "MPN" << "Manufacturer" << "Description" << "Footprint" << "Categories" << "Extras" << "Octopart" << "Product";
        ui->tableOctopartResult->setHorizontalHeaderLabels(hHeader);
        queryResults.clear();
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
                //qDebug() << "proceed because just one line found.";
                selectedOctopartMPN.copyFrom(queryResults[0]);
            }
        }
#if 0
        qDebug() << "query result count: "<<queryResults.count();
        for(int i=0;i< queryResults.count();i++){
            qDebug() << "query result["<<i<<"] ="<< queryResults[i].toString();
        }
        qDebug() << "current query result: "<<selectedOctopartMPN.toString();
#endif
        if (selectedOctopartMPN.getMpn() == ""){
            ui->tabWidget->setCurrentIndex(0);
            ui->statusBar->showMessage("Please select a part from Octopart query first", 2000);
        }else{
            ui->lbl_sourceOctopart->setText("Selected Octopart MPN: "+selectedOctopartMPN.getMpn());
            querymemory.addQuery(selectedOctopartMPN.getMpn());
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
            ui->list_input_libraries->clear();
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
            ui->statusBar->showMessage("Please select a source device from Kicad library first", 2000);
        }else{
            if (selectedOctopartMPN.getMpn() == ""){
                ui->tabWidget->setCurrentIndex(0);
                ui->statusBar->showMessage("Please select a part from Octopart query first", 2000);
            }else{
                querymemory.addQuery(selectedOctopartMPN.getMpn());
                ui->cmb_targetFootprint->clear();
                ui->cmb_targetFootprint->addItems(fpLib.getFootprintList());
                ui->lblSourceDevice->setText(currentSourceLib.getName()+"/"+currentSourceDevice.def.name);

                ui->lbl_productlink->setText("<a href=\""+selectedOctopartMPN.urlProduct+"\">link</a>");
                ui->lbl_productlink->setVisible(selectedOctopartMPN.urlProduct.count() > 0);
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


                if (ui->cmb_targetRuleName->currentText().count()==0){
                    QList<PartCreationRule> possibleRules = partCreationRuleList.findRuleByCategoryID(selectedOctopartMPN.categories);
                    for (auto possibleRule : possibleRules){
                        ui->cmb_targetRuleName->setCurrentText(possibleRule.name);
                        break;
                    }
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
    assert(sl.count() > 0);
    auto devLibFinds = ui->list_input_libraries->findItems(sl[0],Qt::MatchExactly);
    if (devLibFinds.count()){
        auto devLib = devLibFinds[0];
        ui->list_input_libraries->setCurrentItem(devLib);
    }else{
        found = false;
    }

    if (found){
        assert(sl.count() > 1);
        QString partname = sl[1];
        for (int i = 2; i< sl.count()-1; i++){
            partname += "/"+sl[i];
        }
        auto devFinds = ui->list_input_devices->findItems(partname,Qt::MatchExactly);
        if (devFinds.count()){
            auto dev = devFinds[0];
            ui->list_input_devices->setCurrentItem(dev);
            on_list_input_devices_currentRowChanged(ui->list_input_devices->currentRow());
        }else{
            found = false;
        }
    }
    if (found){
        assert(sl.count() > 2);
        int rulePosition = sl.count()-1;
        ui->cmb_targetRuleName->setCurrentText(sl[rulePosition]);
        ui->tabWidget->setCurrentIndex(2);
    }
}





void MainWindow::on_list_input_libraries_currentRowChanged(int currentRow)
{
   // qDebug() << "row" << currentRow;
   // qDebug() << "rowcount" << sourceLibraryPaths.count();

    if ((currentRow > -1) && currentRow < sourceLibraryPaths.count()){
        currentSourceLib.loadFile(sourceLibraryPaths[currentRow]);

        QList<KICADLibSchematicDevice> symList = currentSourceLib.getSymbolList();
        ui->list_input_devices->clear();
        ui->list_input_devices->setCurrentRow(0);
        for(int i=0;i<symList.count();i++){
            ui->list_input_devices->addItem(symList[i].def.name);
        }
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

void MainWindow::on_list_input_devices_itemDoubleClicked(QListWidgetItem *item)
{
    (void)item;
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::showDatasheet()
{
    QString targetpath = getDataSheetFileName(false);
    QString url = "file:///"+targetpath;
#ifdef Q_OS_WIN32
    if (libCreatorSettings.useHashAsDatasheetFilename){
        url += ".lnk";
    }
#endif
    if (QDesktopServices::openUrl(QUrl(url))==false){
        ui->statusBar->showMessage("Could not open file with default brower: "+url,5000);
    }
}

void loadFieldDesign(KICADLibSchematicDeviceField &deviceField,  const LibCreatorSettings &settings){
    const QList<FieldDesignSettingsItem> fieldDesigns = settings.fieldDesigns;
    int designIndex = deviceField.fieldIndex.getRawIndex();
    if (designIndex < fieldDesigns.count()){
        FieldDesignSettingsItem item = fieldDesigns[designIndex];
        deviceField.setDesign(item,settings.fieldDesign_overwriteTextPosition);
    }
}

KICADLibSchematicDevice MainWindow::createDevicePropertiesFromGui( KICADLibSchematicDevice &sourceDevice, QString realDatasheetPath)
{
    KICADLibSchematicDevice result = sourceDevice;
    KICADLibSchematicDeviceField deviceField;

    KICADLibSchematicDeviceField deviceField_reference = sourceDevice.fields.getFieldbyIndex(0);
    KICADLibSchematicDeviceField deviceField_value = sourceDevice.fields.getFieldbyIndex(1);

    deviceField.clear();
    deviceField = deviceField_reference;
    deviceField.text = ui->edt_targetDesignator->text().trimmed();
    deviceField.name = "";
    deviceField.fieldIndex.setRawIndex(0);
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);

    deviceField.clear();
    deviceField.name = "";
    deviceField.text = ui->edt_targetName->text().trimmed();
    deviceField.text =  deviceField.text.replace(" ","_");
    result.def.name = deviceField.text;
    deviceField.fieldIndex.setRawIndex(1);
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);

    deviceField.clear();
    deviceField.name = "footprint";
    deviceField.text = ui->cmb_targetFootprint->currentText().trimmed();
    deviceField.fieldIndex.setRawIndex(2);
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);

    deviceField.clear();
    deviceField.name = "datasheet";

    if (libCreatorSettings.useAbsolutePathForDatasheetField){
        deviceField.text = realDatasheetPath;
    }else{
        QDir d(libCreatorSettings.path_datasheet);
        deviceField.text = d.relativeFilePath(realDatasheetPath);
        qDebug() << deviceField.text;
    }
    deviceField.fieldIndex.setRawIndex(3);
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);

    deviceField.clear();
    deviceField.fieldIndex.setRawIndex(4);
    deviceField.name = "key";
    deviceField.text = ui->edt_targetID->text().trimmed();//QUuid::createUuid().toByteArray().toHex();
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);

    deviceField.clear();
    deviceField.name = "mpn";
    deviceField.text = ui->edt_targetMPN->text().trimmed();
    deviceField.fieldIndex.setRawIndex(5);
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);

    deviceField.clear();
    deviceField.name = "manufacturer";
    deviceField.text = ui->edt_targetManufacturer->text().trimmed();
    deviceField.fieldIndex.setRawIndex(6);
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);


    deviceField.clear();
    deviceField = deviceField_value;
    deviceField.name = "DisplayValue";
    deviceField.text = ui->edt_targetDisplayValue->text().trimmed();
    deviceField.fieldIndex.setRawIndex(7);
    loadFieldDesign(deviceField,libCreatorSettings);
    result.fields.setField(deviceField);

    result.fields.removeAllAboveIndex(8);
    result.fpList.clear();//we want our own footprints..

    result.dcmEntry.description = ui->edt_targetDescription->text();
    result.def.reference =  ui->edt_targetDesignator->text();
    if (libCreatorSettings.fieldDesign_overwritePinSettings){
        result.def.drawPinName = libCreatorSettings.def_draw_pin_name;
        result.def.drawPinNumber = libCreatorSettings.def_draw_pin_number;
        result.def.text_offset = libCreatorSettings.def_text_offset;
    }
    return result;
}

QString MainWindow::cleanUpFileNameNode(QString filename,bool allowSeparatorLikeChars){
    filename = filename.replace("=","_");
    filename = filename.replace("[","_");
    filename = filename.replace("]","_");
    filename = filename.replace("\"","_");
    //dont want to destroy pathes like C:\...
    QChar second_char;
    if (filename.count() > 1){
        second_char = filename[1];
    }else{
        second_char = '0';
    }
    filename = filename.replace(":","_");
    if ((filename.count() > 1) && (second_char != '0')){
        filename[1] = second_char;
    }
    filename = filename.replace(";","_");
    filename = filename.replace("#","_");
    filename = filename.replace(",","_");
    filename = filename.replace("?","_");
    filename = filename.replace("*","_");
    filename = filename.replace("<","_");
    filename = filename.replace(">","_");
    filename = filename.replace("|","_");
    filename = filename.replace("&","_");
    filename = filename.replace(" ","_");
    if(!allowSeparatorLikeChars){
        filename = filename.replace("/","_");
        filename = filename.replace("\\","_");
    }
    filename = filename.toLower();
    return filename;
}

QString MainWindow::cleanUpAndJoinFileName(QString filename_root, QString filename_relative)
{
    QString result;
    result = cleanUpFileNameNode(filename_relative,true);
    if (filename_root.count()){
        result = filename_root+QDir::separator()+result;
    }
    result = result.replace(QString(QDir::separator())+QDir::separator(),QDir::separator());
    return result;
}





QString MainWindow::getDataSheetFileName(bool relativePath){

    QString targetpath;
    QString targetpath_root;
    targetpath = ui->edt_targetDatasheet->text();
    if (relativePath){
        targetpath_root = "";
    }else{
        targetpath_root = libCreatorSettings.path_datasheet;
    }

    targetpath = cleanUpAndJoinFileName(targetpath_root, targetpath);
    return targetpath;
}

QString MainWindow::downloadDatasheet(bool force){
    RESTRequest restRequester;
    QBuffer buf;
    QMultiMap<QString, QString> params;

    connect(&restRequester, SIGNAL(http_request_finished()),
            this, SLOT(octopart_request_finished()));

    connect(&restRequester, SIGNAL(http_request_started()),
            this, SLOT(octopart_request_started()));

    connect(&restRequester, SIGNAL(setProgressbar(int,int)),
            this, SLOT(setProgressbar(int, int)));

    QString targetpath = getDataSheetFileName(false);
    QFileInfo fi(targetpath);
    bool proceed = true;
    if (!fi.isAbsolute()){
        if (QMessageBox::warning(this,"path is relative","Datasheet target path \n\n"+targetpath+"\n\nis not an absolute path. The datasheet will be downloaded relative to the execution path of this application.\n\n"
                                                                                 "You should change this in the settings. Do you want to proceed?",QMessageBox::Yes | QMessageBox::No, QMessageBox::No)==QMessageBox::No){
            proceed = false;
        }

    }
    QString p = fi.absolutePath();
    QString datasheetSavePath = targetpath;




    if ((force || !fi.exists() || libCreatorSettings.useHashAsDatasheetFilename) && proceed ){
        ui->pushButton_3->setText("downloading..");

        QDir().mkpath(p);
        restRequester.startRequest(selectedOctopartMPN.urlDataSheet,params,&buf);

        if (libCreatorSettings.useHashAsDatasheetFilename){
            QCryptographicHash hash(QCryptographicHash::Md5);
            buf.open(QIODevice::ReadOnly);
            hash.addData(buf.readAll());
            buf.close();
            QByteArray hashresult = hash.result();
            QString hash_str = hashresult.toHex();
            datasheetSavePath = fi.absolutePath()+QDir::separator()+hash_str+"."+fi.suffix();
        }
        QFile file(datasheetSavePath);
        ui->pushButton_3->setText("download and show");

        if (file.open(QIODevice::WriteOnly)){
            buf.open(QIODevice::ReadOnly);
            file.write(buf.readAll());
            if (libCreatorSettings.useHashAsDatasheetFilename){

                QString linksuffix="";
            #ifdef Q_OS_WIN32
                linksuffix = ".lnk";
            #endif
                if (!file.link(targetpath+linksuffix)){
                    ui->statusBar->showMessage("couldnt create link "+targetpath+linksuffix,5000);
                }
            }
            file.close();
            ui->statusBar->showMessage("saved datasheet to "+datasheetSavePath,5000);

        }else{
            ui->statusBar->showMessage("couldnt open file "+datasheetSavePath,5000);
            qDebug() << "couldnt open file "+datasheetSavePath;
            datasheetSavePath = "";
        }

    }else{
        if (proceed == false){
            datasheetSavePath = "";
        }else{

        }

    }
    setDatasheetButton();
    return datasheetSavePath;
}



void MainWindow::on_pushButton_3_clicked()
{
    QString filepath=downloadDatasheet(true);
    if (filepath.count()){
        QFileInfo fi(filepath);
        bool isTextFile = false;
        if (fi.size() < 1000){
            //test if textfile or not.
            QFile file(filepath);
            isTextFile = true;
            if (file.open(QIODevice::ReadOnly)){
                QByteArray buf = file.readAll();
                for(auto b : buf){
                    if ((uint8_t)b > 127){
                        isTextFile = false;
                    }
                }
            }
        }
        if(!isTextFile){
            showDatasheet();
        }else{
            TextFile *textfile = new TextFile(this);
            textfile->setText(filepath);
            textfile->show();
        }
    }
}

void MainWindow::on_btnCreatePart_clicked()
{
    QString realDatasheetPath = downloadDatasheet(false);

    KICADLibSchematicDeviceLibrary targetLib;
    KICADLibSchematicDevice deviceToBeCreated = createDevicePropertiesFromGui(currentSourceDevice,realDatasheetPath);

    QString targetLibName  = ui->cmb_targetLibrary->currentText().trimmed();
    if (targetLibName.count()==0){
        QMessageBox::critical(this,"Empty library name","Please specify a library name.");
        return;
    }

    QFileInfo fi(targetLibName);

    if (fi.suffix() == "lib"){
        targetLibName= targetLibName;
    }else{
        targetLibName= targetLibName+".lib";
    }
    targetLibName = cleanUpAndJoinFileName(libCreatorSettings.path_targetLibrary,targetLibName);

    if (deviceToBeCreated.isValid()){
        targetLib.loadFile(targetLibName);
        bool proceed = true;
        if (targetLib.indexOf(deviceToBeCreated.def.name)>-1){
            QMessageBox msgBox;
            msgBox.setText("Part duplicate");
            msgBox.setInformativeText("A part with this name already exists. Overwrite old part?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Ok);
            proceed =  QMessageBox::Ok == msgBox.exec();
        }
        if(proceed){

            targetLib.insertDevice(deviceToBeCreated);
            if (targetLib.saveFile(targetLibName)){
                ui->statusBar->showMessage("Inserted part "+deviceToBeCreated.def.name+" to library: \""+targetLibName+"\"",10000);
            }else{
                ui->statusBar->showMessage("Could not save library \""+targetLibName+"\"",10000);
            }
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

void insertStandVariableValue(QMap<QString, QString> &variables, QString name, QString value, bool allowEmptyValue){
    if (value.count() || allowEmptyValue){
        if ((value != "example") || (!variables.contains(name))){
            variables.insert(name,value);
        }
    }
}

void MainWindow::insertStandardVariablesToMap(QMap<QString, QString> &variables, QString footprint, QString reference, QString ruleName, QString mpn,
                                              QString manufacturer, QString description, QString OctoFootprint, QString OctoFootprintMetric_IPC){
    insertStandVariableValue(variables,"%source.footprint%",footprint,false);
    insertStandVariableValue(variables,"%source.ref%",reference,false);
    insertStandVariableValue(variables,"%rule.name%",ruleName,false);
    insertStandVariableValue(variables,"%rule.name.saveFilename%",cleanUpFileNameNode(ruleName,false),false);

    insertStandVariableValue(variables,"%octo.mpn%",mpn,true);
    insertStandVariableValue(variables,"%octo.manufacturer%",manufacturer,true);
    insertStandVariableValue(variables,"%octo.description%",description,true);
    insertStandVariableValue(variables,"%octo.footprint%",OctoFootprint,true);
    insertStandVariableValue(variables,"%octo.metric-ipc-footprint%",OctoFootprintMetric_IPC,true);

    insertStandVariableValue(variables,"%octo.mpn.saveFilename%",cleanUpFileNameNode(mpn,false),true);
    insertStandVariableValue(variables,"%octo.manufacturer.saveFilename%",cleanUpFileNameNode(manufacturer,false),true);

    variables.insert("%target.id%",QString::number(QDateTime::currentMSecsSinceEpoch()));
}

QMap<QString, QString> MainWindow::createVariableMap(){
    QMap<QString, QString> variables = selectedOctopartMPN.getQueryResultMap();
    QString fp = currentSourceDevice.fields.getFieldbyIndex(3).text;
    QString footprint;
    QString reference = currentSourceDevice.def.reference;
    QString ruleName = ui->cmb_targetRuleName->currentText();
    QString mpn = selectedOctopartMPN.getMpn();
    QString manufacturer = selectedOctopartMPN.manufacturer;
    QString description = selectedOctopartMPN.description;
    QString OctoFootprint = selectedOctopartMPN.footprint;
    QString OctoFootprintMetric_IPC = selectedOctopartMPN.footprint;
    if (fp.count()){
       footprint = currentSourceDevice.fields.getFieldbyIndex(3).text;
    }

    if (OctoFootprintMetric_IPC == "01005"){
        OctoFootprintMetric_IPC = "0402";
    }else if (OctoFootprintMetric_IPC == "0201"){
        OctoFootprintMetric_IPC = "0603";
    }else if (OctoFootprintMetric_IPC == "0402"){
        OctoFootprintMetric_IPC = "1005";
    }else if (OctoFootprintMetric_IPC == "0603"){
        OctoFootprintMetric_IPC = "1608";
    }else if (OctoFootprintMetric_IPC == "0805"){
        OctoFootprintMetric_IPC = "2012";
    }else if (OctoFootprintMetric_IPC == "1008"){
        OctoFootprintMetric_IPC = "2520";
    }else if (OctoFootprintMetric_IPC == "1206"){
        OctoFootprintMetric_IPC = "3216";
    }else if (OctoFootprintMetric_IPC == "1210"){
        OctoFootprintMetric_IPC = "3225";
    }else if (OctoFootprintMetric_IPC == "1806"){
        OctoFootprintMetric_IPC = "4516";
    }else if (OctoFootprintMetric_IPC == "1812"){
        OctoFootprintMetric_IPC = "4532";
    }else if (OctoFootprintMetric_IPC == "2010"){
        OctoFootprintMetric_IPC = "5025";
    }else if (OctoFootprintMetric_IPC == "2512"){
        OctoFootprintMetric_IPC = "6332";
    }else if (OctoFootprintMetric_IPC == "TSSOP"){
            OctoFootprintMetric_IPC = "SOP";
    }else if (OctoFootprintMetric_IPC == "TSOP"){
        OctoFootprintMetric_IPC = "SOP";
    }else if (OctoFootprintMetric_IPC == "PSOP"){
        OctoFootprintMetric_IPC = "SOP";
    }else if (OctoFootprintMetric_IPC == "SSOP"){
        OctoFootprintMetric_IPC = "SOP";
    }else if (OctoFootprintMetric_IPC == "QSOP"){
        OctoFootprintMetric_IPC = "SOP";
    }else if (OctoFootprintMetric_IPC == "VSOP"){
        OctoFootprintMetric_IPC = "SOP";
    }else if (OctoFootprintMetric_IPC == "SMB"){
        OctoFootprintMetric_IPC = "DIOM5336X240";
    }else if (OctoFootprintMetric_IPC == "SMC"){
        OctoFootprintMetric_IPC = "DIOM7959X241";
    }else if (OctoFootprintMetric_IPC == "SMA"){
        OctoFootprintMetric_IPC = "DIOM5025X231";
    }



    insertStandardVariablesToMap(variables, footprint,reference,ruleName, mpn, manufacturer,description, OctoFootprint, OctoFootprintMetric_IPC );


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
    PartCreationRule currentRule =  partCreationRuleList.getRuleByNameForAppliaction(ui->cmb_targetRuleName->currentText());

  //  if (currentRule.name != "")
    // even if name is empty a global rule may still be active.
    {

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

}




void MainWindow::on_edt_targetDatasheet_textChanged(const QString &arg1)
{
    (void)arg1;
    setDatasheetButton();
}

void MainWindow::on_btn_show_datasheet_clicked()
{

}

void MainWindow::on_actionOptions_triggered()
{
    OptionsDialog diag(libCreatorSettings);
    diag.exec();
    octopartInterface.setAPIKey(libCreatorSettings.apikey);
    libCreatorSettings.complainAboutSettings(this);
}



void MainWindow::on_actionDesign_settings_triggered()
{
    DesignSettings diag(libCreatorSettings,this);
    diag.exec();

}


