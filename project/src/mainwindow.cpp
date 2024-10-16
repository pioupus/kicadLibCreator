#include "mainwindow.h"
#include "designsettings.h"
#include "kicadfile_lib.h"
#include "optionsdialog.h"
#include "ruleeditor.h"
#include "textfile.h"
#include "ui_mainwindow.h"
#include <QCompleter>
#include <QCryptographicHash>
#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QPalette>
#include <QUuid>
#include <assert.h>

void setCurrentComboText(QComboBox *cmb, QString text) {
    int i = cmb->findText(text);
    if (i > -1) {
        cmb->setCurrentIndex(i);
    } else {
        cmb->setEditText(text);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , octopartInterface("", parent)
    , digikeyInterface(this)
    , selectedOctopartMPN("")
    , appearance_settings("kicadLibCreatorAppearanceSettings.ini", QSettings::IniFormat)

{
    ui->setupUi(this);

    connect(ui->comboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(oncmbOctoQueryEnterPressed()));

    connect(&octopartInterface, SIGNAL(octopart_request_finished()), this, SLOT(search_request_finished()));
    connect(&octopartInterface, SIGNAL(octopart_request_started()), this, SLOT(search_request_started()));
    connect(&octopartInterface, SIGNAL(setProgressbar(int, int)), this, SLOT(setProgressbar(int, int)));

    connect(&digikeyInterface, SIGNAL(request_finished()), this, SLOT(search_request_finished()));
    connect(&digikeyInterface, SIGNAL(request_started()), this, SLOT(search_request_started()));
    connect(&digikeyInterface, SIGNAL(setProgressbar(int, int)), this, SLOT(setProgressbar(int, int)));

    libCreatorSettings.loadSettings("kicadLibCreatorSettings.ini");
    fpLib.scan(libCreatorSettings.path_footprintLibrary);

    digikeyInterface.setSettings(&libCreatorSettings);
    octopartInterface.setAPIKey(libCreatorSettings.octo_apikey);
    partCreationRuleList.loadFromFile("partCreationRules.ini");
    querymemory.loadQueryList(ui->comboBox);
    libCreatorSettings.complainAboutSettings(this);
    progressbar = new QProgressBar(ui->statusBar);
    progressbar->setVisible(false);
    ui->statusBar->addPermanentWidget(progressbar);
    selectedOctopartMPN.setDebugPrintMpn(true);
    // ui->lblSpinner->setVisible(false);
    ui->cbtn_exact_match->setChecked(!libCreatorSettings.useFuzzyOctopartQueries);

    renderarea = new RenderArea(ui->paintScrollArea);

    ui->scrollAreaLayout->addWidget(renderarea);

    apply_appearence_settings();
    //mainLayout->addWidget(renderArea, 0, 0, 1, 4);
}

void MainWindow::setProgressbar(int progress, int total) {
    progressbar->setValue(progress);
    progressbar->setMaximum(total);
}

void MainWindow::search_request_started() {
    progressbar->setValue(0);
    progressbar->setMaximum(100);
    progressbar->setVisible(true);
    ui->statusBar->showMessage("Sending request..", 500);
}

void MainWindow::search_request_finished() {
    progressbar->setVisible(false);
    //  qDebug() << "httpFinished";
}

MainWindow::~MainWindow() {
    save_appearence_settings();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    octopartCategorieCache.save();
    libCreatorSettings.saveSettings();
    querymemory.save();
    partCreationRuleList.saveFile("partCreationRules.ini");
    event->accept();
}

void MainWindow::on_pushButton_clicked() {
    resetSearchQuery(true);
    if (ui->cmb_search_supplier->currentText().toLower() == "digikey") {
        digikeyInterface.query(ui->comboBox->currentText());
        queryResults.clear();
        queryResults.append(digikeyInterface.result_QueryMPN);
        //octopartInterface.sendMPNQuery(octopartCategorieCache, ui->comboBox->currentText(), !ui->cbtn_exact_match->isChecked());
    } else if (ui->cmb_search_supplier->currentText().toLower() == "octopart") {
        octopartInterface.sendMPNQuery(octopartCategorieCache, ui->comboBox->currentText(), !ui->cbtn_exact_match->isChecked());
        queryResults.clear();
        queryResults.append(octopartInterface.octopartResult_QueryMPN);
    }

    ui->tableOctopartResult->setRowCount(queryResults.length());

    for (int i = 0; i < queryResults.length(); i++) {
        QTableWidgetItem *newMPNItem = new QTableWidgetItem(queryResults[i].getMpn());
        QTableWidgetItem *newManufacturerItem = new QTableWidgetItem(queryResults[i].manufacturer);
        QTableWidgetItem *newFootprint = new QTableWidgetItem(queryResults[i].footprint);
        QTableWidgetItem *newCategorie = new QTableWidgetItem("");
        QTableWidgetItem *newDescription = new QTableWidgetItem(queryResults[i].description);
        QTableWidgetItem *newExtras = new QTableWidgetItem("");
        QTableWidgetItem *newOctopartURL = new QTableWidgetItem("link");
        QTableWidgetItem *newProductURL = new QTableWidgetItem("");
        newOctopartURL->setData(Qt::UserRole, queryResults[i].urlOctoPart);
        newMPNItem->setData(Qt::UserRole, i);
        QString categorie_str;
        for (int n = 0; n < queryResults[i].categories.count(); n++) {
            for (int m = 0; m < queryResults[i].categories[n].categorieNameTree.count(); m++) {
                categorie_str += queryResults[i].categories[n].categorieNameTree[m] + ", ";
            }
            categorie_str += "\n";
        }
        QString ds;
        if (queryResults[i].urlDataSheet == "") {
            ds = "Datasheets: no\n";
        } else {
            ds = "Datasheets: yes\n";
        }
        if (queryResults[i].url3DModel == "") {
            ds += "3DModel: no";
        } else {
            ds += "3DModel: yes";
        }
        if (queryResults[i].urlProduct.size()) {
            newProductURL->setText("link");
        } else {
            newProductURL->setText("no link");
        }

        newExtras->setText(ds);
        newCategorie->setText(categorie_str);
        ui->tableOctopartResult->setItem(i, 0, newMPNItem);
        ui->tableOctopartResult->setItem(i, 1, newManufacturerItem);
        ui->tableOctopartResult->setItem(i, 2, newDescription);
        ui->tableOctopartResult->setItem(i, 3, newFootprint);
        ui->tableOctopartResult->setItem(i, 4, newCategorie);
        ui->tableOctopartResult->setItem(i, 5, newExtras);
        ui->tableOctopartResult->setItem(i, 6, newOctopartURL);
        ui->tableOctopartResult->setItem(i, 7, newProductURL);
    }
    ui->tableOctopartResult->resizeColumnsToContents();
    ui->tableOctopartResult->resizeRowsToContents();
    save_appearence_settings();
    //octopartInterface.sendOctoPartRequest("SN74S74N");
}
void MainWindow::openHttpLink(QString url) {
    if (url.size()) {
        ui->statusBar->showMessage("Open Browser..", 2000);
        QDesktopServices::openUrl(QUrl(url));
    }
}

void MainWindow::tableOctopartResult_linkclicked(int row, int column) {
    if (column == 6) {
        if (row < queryResults.count()) {
            openHttpLink(queryResults[row].urlOctoPart);
        }
    } else if (column == 7) {
        if (row < queryResults.count()) {
            openHttpLink(queryResults[row].urlProduct);
        }
    }
}

void MainWindow::on_tableOctopartResult_cellDoubleClicked(int row, int column) {
    if (column >= 6) {
        tableOctopartResult_linkclicked(row, column);
    } else {
        ui->tabWidget->setCurrentIndex(1);
    }
}

void MainWindow::on_tableOctopartResult_cellClicked(int row, int column) {
    tableOctopartResult_linkclicked(row, column);
}

void MainWindow::on_lbl_productlink_linkActivated(const QString &link) {
    openHttpLink(link);
}

void MainWindow::on_tableOctopartResult_cellActivated(int row, int column) {
    (void)column;
    (void)row;
}

void MainWindow::on_tableOctopartResult_itemSelectionChanged() {
    resetSearchQuery(false);
    int row = ui->tableOctopartResult->currentRow();
    // qDebug() << "row selected:"<<row<<"queryResults.count():" << queryResults.count();
    if (row < queryResults.count()) {
        selectedOctopartMPN.copyFrom(queryResults[row]);
    }
}

void MainWindow::on_comboBox_editTextChanged(const QString &arg1) {
    (void)arg1;
    if (!ui->comboBox->property("ignoreChanges").toBool()) {
        resetSearchQuery(true);
    }
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1) {
    (void)arg1;
    //on_comboBox_editTextChanged(arg1);
}

void MainWindow::resetSearchQuery(bool resetAlsoTable) {
    if (resetAlsoTable) {
        ui->tableOctopartResult->clear();
        ui->tableOctopartResult->setRowCount(0);
        ui->tableOctopartResult->setColumnCount(8);
        QStringList hHeader;
        hHeader << "MPN"
                << "Manufacturer"
                << "Description"
                << "Footprint"
                << "Categories"
                << "Extras"
                << "Octopart"
                << "Product";
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

void MainWindow::clearQuickLinks(QLayout *layout) {
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            delete widget;
        if (QLayout *childLayout = item->layout())
            clearQuickLinks(childLayout);
        delete item;
    }
}

void MainWindow::on_tabWidget_currentChanged(int index) {
    if (index == 0) {
        querymemory.loadQueryList(ui->comboBox);
    } else if (index == 1) {
        if (ui->tableOctopartResult->currentRow() == -1) {
            if (queryResults.count() == 1) {
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
        if (selectedOctopartMPN.getMpn() == "") {
            ui->tabWidget->setCurrentIndex(0);
            ui->statusBar->showMessage("Please select a part from Internet search first", 2000);
        } else {
            ui->lbl_sourceOctopart->setText("Selected internet search MPN: " + selectedOctopartMPN.getMpn());
            querymemory.addQuery(selectedOctopartMPN.getMpn());
            sourceLibraryPaths.clear();
            QDirIterator it(libCreatorSettings.path_sourceLibrary, QDirIterator::NoIteratorFlags);
            while (it.hasNext()) {
                QString name = it.next();
                QFileInfo fi(name);
                if (fi.suffix() == "kicad_sym") {
                    sourceLibraryPaths.append(name);
                }
            }
            //ui->cmb_targetRuleName->setCurrentText("");
            ui->list_input_libraries->clear();
            sourceLibraryPaths.sort(Qt::CaseInsensitive);
            for (int i = 0; i < sourceLibraryPaths.count(); i++) {
                QFileInfo fi(sourceLibraryPaths[i]);
                ui->list_input_libraries->addItem(fi.fileName());
            }
            ui->scrollQuickLink->setWidgetResizable(true);

            clearQuickLinks(ui->verticalLayout_3);

            QList<PartCreationRule> possibleRules = partCreationRuleList.findRuleByCategoryID(selectedOctopartMPN.categories);

            for (auto possibleRule : possibleRules) {
                QLabel *lbl = new QLabel(ui->scrollAreaWidgetContents);
                auto sl = possibleRule.links_source_device;
                for (auto s : sl) {
                    QString ruleName = possibleRule.name;
                    lbl->setText("<a href=\"" + s + "/" + ruleName + "\">" + s + " (" + ruleName + ")</a>");
                    lbl->setTextFormat(Qt::RichText);
                    ui->verticalLayout_3->addWidget(lbl);
                    connect(lbl, SIGNAL(linkActivated(QString)), this, SLOT(onQuickLinkClicked(QString)));
                }
            }
        }

    } else if (index == 2) {
        if (currentSourceDevice.get_symbol_name() == "") {
            ui->tabWidget->setCurrentIndex(1);
            ui->statusBar->showMessage("Please select a source device from Kicad library first", 2000);
        } else {
            if (selectedOctopartMPN.getMpn() == "") {
                ui->tabWidget->setCurrentIndex(0);
                ui->statusBar->showMessage("Please select a part from Octopart query first", 2000);
            } else {
                querymemory.addQuery(selectedOctopartMPN.getMpn());
                ui->cmb_targetFootprint->clear();
                ui->cmb_targetFootprint->addItems(fpLib.getFootprintList());
                ui->lblSourceDevice->setText(currentSourceLib.getName() + "/" + currentSourceDevice.get_symbol_name());

                ui->lbl_productlink->setText("<a href=\"" + selectedOctopartMPN.urlProduct + "\">link</a>");
                ui->lbl_productlink->setVisible(selectedOctopartMPN.urlProduct.size() > 0);
                QStringList targetLibList;
                ui->cmb_targetLibrary->clear();
                QDirIterator it(libCreatorSettings.path_targetLibrary, QDirIterator::NoIteratorFlags);
                while (it.hasNext()) {
                    QString name = it.next();
                    QFileInfo fi(name);
                    if (fi.suffix() == "kicad_sym") {
                        targetLibList.append(name);
                    }
                }
                targetLibList.sort(Qt::CaseInsensitive);
                for (int i = 0; i < targetLibList.count(); i++) {
                    QFileInfo fi(targetLibList[i]);
                    ui->cmb_targetLibrary->addItem(fi.fileName());
                }

                if (ui->cmb_targetRuleName->currentText().size() == 0) {
                    QList<PartCreationRule> possibleRules = partCreationRuleList.findRuleByCategoryID(selectedOctopartMPN.categories);
                    for (auto possibleRule : possibleRules) {
                        setCurrentComboText(ui->cmb_targetRuleName, possibleRule.name);
                        break;
                    }
                }
                loadRuleCombobox();
                on_btn_applyRule_clicked();
            }
        }
    }
}

void MainWindow::loadRuleCombobox() {
    QString selectedRule = ui->cmb_targetRuleName->currentText();
    ui->cmb_targetRuleName->clear();
    ui->cmb_targetRuleName->addItems(partCreationRuleList.namesWithoutGlobal);
    setCurrentComboText(ui->cmb_targetRuleName, selectedRule);
}

void MainWindow::onQuickLinkClicked(QString s) {
    bool found = true;
    auto sl = s.split("/");
    assert(sl.count() > 0);
    auto devLibFinds = ui->list_input_libraries->findItems(sl[0], Qt::MatchExactly);
    if (devLibFinds.count()) {
        auto devLib = devLibFinds[0];
        ui->list_input_libraries->setCurrentItem(devLib);
    } else {
        found = false;
    }

    if (found) {
        assert(sl.count() > 1);
        QString partname = sl[1];
        for (int i = 2; i < sl.count() - 1; i++) {
            partname += "/" + sl[i];
        }
        auto devFinds = ui->list_input_devices->findItems(partname, Qt::MatchExactly);
        if (devFinds.count()) {
            auto dev = devFinds[0];
            ui->list_input_devices->setCurrentItem(dev);
            on_list_input_devices_currentRowChanged(ui->list_input_devices->currentRow());
        } else {
            found = false;
        }
    }
    if (found) {
        assert(sl.count() > 2);
        int rulePosition = sl.count() - 1;

        setCurrentComboText(ui->cmb_targetRuleName, sl[rulePosition]);
        ui->tabWidget->setCurrentIndex(2);
    }
}

void MainWindow::on_list_input_libraries_currentRowChanged(int currentRow) {
    // qDebug() << "row" << currentRow;
    // qDebug() << "rowcount" << sourceLibraryPaths.count();

    if ((currentRow > -1) && currentRow < sourceLibraryPaths.count()) {
        currentSourceLib.loadFile(sourceLibraryPaths[currentRow]);

        auto symList = currentSourceLib.getSymbolList();
        ui->list_input_devices->clear();
        ui->list_input_devices->setCurrentRow(0);
        for (const auto &dev : symList) {
            ui->list_input_devices->addItem(dev.get_symbol_name());
        }
    }
}

void MainWindow::on_list_input_devices_currentRowChanged(int currentRow) {
    const auto &symList = currentSourceLib.getSymbolList();
    if ((currentRow < symList.count()) && (currentRow >= 0)) {
        currentSourceDevice = symList[currentRow];
        renderarea->paintSymbol(currentSourceDevice);
    } else {
        currentSourceDevice.clear();
    }
}

void MainWindow::on_list_input_devices_itemDoubleClicked(QListWidgetItem *item) {
    (void)item;
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::showDatasheet() {
    QString targetpath = getDataSheetFileName(false);
    QString url = "file:///" + targetpath;
#ifdef Q_OS_WIN32
    if (libCreatorSettings.useHashAsDatasheetFilename) {
        url += ".lnk";
    }
#endif
    if (QDesktopServices::openUrl(QUrl(url)) == false) {
        ui->statusBar->showMessage("Could not open file with default brower: " + url, 5000);
    }
}

void loadFieldDesign(sSymbolProperty &deviceField, const LibCreatorSettings &settings) {
    const int FIELD_INDEX_REFERENCE = 0;
    const int FIELD_INDEX_NAME = 1;
    const int FIELD_INDEX_FOOTPRINT = 2;
    const int FIELD_INDEX_DATASHEET = 3;
    const int FIELD_INDEX_UUID = 4;
    const int FIELD_INDEX_MPN = 5;
    const int FIELD_INDEX_MANUFACTURER = 6;
    const int FIELD_INDEX_DISPLAY_VALUE = 7;
    const QList<FieldDesignSettingsItem> fieldDesigns = settings.fieldDesigns;
    int designIndex = 0;
    if (deviceField.key.toLower() == "footprint") {
        designIndex = FIELD_INDEX_FOOTPRINT;
    } else if (deviceField.key.toLower() == "mpn") {
        designIndex = FIELD_INDEX_MPN;
    } else if (deviceField.key.toLower() == "reference") {
        designIndex = FIELD_INDEX_REFERENCE;
    } else if (deviceField.key.toLower() == "manufacturer") {
        designIndex = FIELD_INDEX_MANUFACTURER;
    } else if (deviceField.key.toLower() == "displayvalue") {
        designIndex = FIELD_INDEX_DISPLAY_VALUE;
    } else if (deviceField.key.toLower() == "datasheet") {
        designIndex = FIELD_INDEX_DATASHEET;
    } else if (deviceField.key.toLower() == "uuid") {
        designIndex = FIELD_INDEX_UUID;
    }

    if (designIndex < fieldDesigns.count()) {
        FieldDesignSettingsItem item = fieldDesigns[designIndex];
        deviceField.set_design(item, settings.fieldDesign_overwriteTextPosition);
    }
    (void)FIELD_INDEX_NAME;
}

KICADLibSchematicDevice MainWindow::createDevicePropertiesFromGui(KICADLibSchematicDevice &sourceDevice, QString realDatasheetPath) {
    KICADLibSchematicDevice result = sourceDevice;
    //  KICADLibSchematicDeviceField deviceField;

    auto deviceField_value = sourceDevice.get_property_value("Value");

    auto symbol_name = ui->edt_targetName->text().trimmed();
    symbol_name = symbol_name.replace(" ", "_");
    symbol_name = symbol_name.remove('\n');
    result.set_symbol_name(symbol_name);

    auto deviceField_reference = sourceDevice.get_property_value("Reference");
    assert(deviceField_reference.key == "Reference");
    loadFieldDesign(deviceField_reference, libCreatorSettings);
    deviceField_reference.value = ui->edt_targetDesignator->text().trimmed();
    result.set_property_value(deviceField_reference);

    auto deviceField_footprint = sourceDevice.get_property_value("Footprint");
    assert(deviceField_footprint.key == "Footprint");
    deviceField_footprint.value = ui->cmb_targetFootprint->currentText().trimmed();
    deviceField_footprint.value = deviceField_footprint.value.remove('\n');
    loadFieldDesign(deviceField_footprint, libCreatorSettings);
    result.set_property_value(deviceField_footprint);

    auto deviceField_datasheet = sourceDevice.get_property_value("Datasheet");
    assert(deviceField_datasheet.key == "Datasheet");
    if (libCreatorSettings.useAbsolutePathForDatasheetField) {
        deviceField_datasheet.value = realDatasheetPath;
    } else {
        QDir d(libCreatorSettings.path_datasheet);
        deviceField_datasheet.value = d.relativeFilePath(realDatasheetPath);
        qDebug() << deviceField_datasheet.value;
    }
    deviceField_datasheet.value = deviceField_datasheet.value.remove('\n');
    loadFieldDesign(deviceField_datasheet, libCreatorSettings);
    result.set_property_value(deviceField_datasheet);

    sSymbolProperty devicevield_uuid;
    devicevield_uuid.key = "uuid";                               //fieldindex 4
    devicevield_uuid.value = ui->edt_targetID->text().trimmed(); //QUuid::Uuid().toByteArray().toHex();
    devicevield_uuid.value = devicevield_uuid.value.remove('\n');
    loadFieldDesign(devicevield_uuid, libCreatorSettings);
    result.set_property_value(devicevield_uuid);

    sSymbolProperty devicevield_mpn;
    devicevield_mpn.key = "mpn"; //fieldindex 5
    devicevield_mpn.value = ui->edt_targetMPN->text().trimmed();
    devicevield_mpn.value = devicevield_mpn.value.remove('\n');
    loadFieldDesign(devicevield_mpn, libCreatorSettings);
    result.set_property_value(devicevield_mpn);

    sSymbolProperty devicevield_manufacturer;
    devicevield_manufacturer.key = "manufacturer"; //fieldindex 6
    devicevield_manufacturer.value = ui->edt_targetManufacturer->text().trimmed();
    devicevield_manufacturer.value = devicevield_manufacturer.value.remove('\n');
    loadFieldDesign(devicevield_manufacturer, libCreatorSettings);
    result.set_property_value(devicevield_manufacturer);

    sSymbolProperty devicevield_display_value;
    devicevield_display_value.key = "DisplayValue"; //fieldindex 7
    devicevield_display_value.value = ui->edt_targetDisplayValue->text().trimmed();
    devicevield_display_value.value = devicevield_display_value.value.remove('\n');
    loadFieldDesign(devicevield_display_value, libCreatorSettings);
    result.set_property_value(devicevield_display_value);

    //  result.fields.removeAllAboveIndex(8);

    auto fp_list = result.get_property_value("ki_fp_filters"); //we want our own footprints..
    if (fp_list.value != "") {
        result.set_property_value("ki_fp_filters", "");
    }

    result.set_property_value("Description", ui->edt_targetDescription->text());

    result.set_property_value("Reference", ui->edt_targetDesignator->text());
    if (libCreatorSettings.fieldDesign_overwritePinSettings) {
        sKicadPinName pin_name;
        pin_name.is_hidden = !libCreatorSettings.def_draw_pin_name;
        pin_name.offset = !libCreatorSettings.def_text_offset;
        result.sexp_pin_names = pin_name;
        result.sexp_pin_numbers_hide = !libCreatorSettings.def_draw_pin_number;
    }
    return result;
}

QString MainWindow::cleanUpFileNameNode(QString filename, bool allowSeparatorLikeChars) {
    filename = filename.replace("=", "_");
    filename = filename.replace("[", "_");
    filename = filename.replace("]", "_");
    filename = filename.replace("\"", "_");
    //dont want to destroy pathes like C:\...
    QChar second_char;
    if (filename.size() > 1) {
        second_char = filename[1];
    } else {
        second_char = '0';
    }
    filename = filename.replace(":", "_");
    if ((filename.size() > 1) && (second_char != '0')) {
        filename[1] = second_char;
    }
    filename = filename.replace(";", "_");
    filename = filename.replace("#", "_");
    filename = filename.replace(",", "_");
    filename = filename.replace("?", "_");
    filename = filename.replace("*", "_");
    filename = filename.replace("<", "_");
    filename = filename.replace(">", "_");
    filename = filename.replace("|", "_");
    filename = filename.replace("&", "_");
    filename = filename.replace(" ", "_");
    if (!allowSeparatorLikeChars) {
        filename = filename.replace("/", "_");
        filename = filename.replace("\\", "_");
    }
    filename = filename.toLower();
    return filename;
}

QString MainWindow::cleanUpAndJoinFileName(QString filename_root, QString filename_relative) {
    QString result;
    result = cleanUpFileNameNode(filename_relative, true);
    if (filename_root.size()) {
        result = filename_root + QDir::separator() + result;
    }
    result = result.replace(QString(QDir::separator()) + QDir::separator(), QDir::separator());
    return result;
}

QString MainWindow::getDataSheetFileName(bool relativePath) {
    QString targetpath;
    QString targetpath_root;
    targetpath = ui->edt_targetDatasheet->text();
    if (relativePath) {
        targetpath_root = "";
    } else {
        targetpath_root = libCreatorSettings.path_datasheet;
    }

    targetpath = cleanUpAndJoinFileName(targetpath_root, targetpath);
    return targetpath;
}

QString MainWindow::downloadDatasheet(bool force) {
    RESTRequest restRequester;
    QBuffer buf;
    QMultiMap<QString, QString> params;

    connect(&restRequester, SIGNAL(http_request_finished()), this, SLOT(octopart_request_finished()));

    connect(&restRequester, SIGNAL(http_request_started()), this, SLOT(octopart_request_started()));

    connect(&restRequester, SIGNAL(setProgressbar(int, int)), this, SLOT(setProgressbar(int, int)));

    QString targetpath = getDataSheetFileName(false);
    QFileInfo fi(targetpath);
    bool proceed = true;
    if (!fi.isAbsolute()) {
        if (QMessageBox::warning(this, "path is relative",
                                 "Datasheet target path \n\n" + targetpath +
                                     "\n\nis not an absolute path. The datasheet will be downloaded relative to the execution path of this application.\n\n"
                                     "You should change this in the settings. Do you want to proceed?",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            proceed = false;
        }
    }
    QString p = fi.absolutePath();
    QString datasheetSavePath = targetpath;

    if ((force || !fi.exists() || libCreatorSettings.useHashAsDatasheetFilename) && proceed) {
        ui->pushButton_3->setText("downloading..");

        QDir().mkpath(p);
        restRequester.startRequest(selectedOctopartMPN.urlDataSheet, params, &buf);

        if (libCreatorSettings.useHashAsDatasheetFilename) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            buf.open(QIODevice::ReadOnly);
            hash.addData(buf.readAll());
            buf.close();
            QByteArray hashresult = hash.result();
            QString hash_str = hashresult.toHex();
            datasheetSavePath = fi.absolutePath() + QDir::separator() + hash_str + "." + fi.suffix();
        }
        QFile file(datasheetSavePath);
        ui->pushButton_3->setText("download and show");

        if (file.open(QIODevice::WriteOnly)) {
            buf.open(QIODevice::ReadOnly);
            file.write(buf.readAll());
            if (libCreatorSettings.useHashAsDatasheetFilename) {
                QString linksuffix = "";
#ifdef Q_OS_WIN32
                linksuffix = ".lnk";
#endif
                if (!file.link(targetpath + linksuffix)) {
                    ui->statusBar->showMessage("couldnt  link " + targetpath + linksuffix, 5000);
                }
            }
            file.close();
            ui->statusBar->showMessage("saved datasheet to " + datasheetSavePath, 5000);

        } else {
            ui->statusBar->showMessage("couldnt open file " + datasheetSavePath, 5000);
            qDebug() << "couldnt open file " + datasheetSavePath;
            datasheetSavePath = "";
        }

    } else {
        if (proceed == false) {
            datasheetSavePath = "";
        } else {
        }
    }
    setDatasheetButton();
    return datasheetSavePath;
}

void MainWindow::on_pushButton_3_clicked() {
    QString filepath = downloadDatasheet(true);
    if (filepath.size()) {
        QFileInfo fi(filepath);
        bool isTextFile = false;
        if (fi.size() < 1000) {
            //test if textfile or not.
            QFile file(filepath);
            isTextFile = true;
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray buf = file.readAll();
                for (auto b : buf) {
                    if ((uint8_t)b > 127) {
                        isTextFile = false;
                    }
                }
            }
        }
        if (!isTextFile) {
            showDatasheet();
        } else {
            TextFile *textfile = new TextFile(this);
            textfile->setText(filepath);
            textfile->show();
        }
    }
}

void MainWindow::on_btnCreatePart_clicked() {
    QString realDatasheetPath = downloadDatasheet(false);

    KICADLibSchematicDeviceLibrary targetLib;
    KICADLibSchematicDevice deviceToBeCreated = createDevicePropertiesFromGui(currentSourceDevice, realDatasheetPath);

    QString targetLibName = ui->cmb_targetLibrary->currentText().trimmed();
    if (targetLibName.size() == 0) {
        QMessageBox::critical(this, "Empty library name", "Please specify a library name.");
        return;
    }

    QFileInfo fi(targetLibName);

    if (fi.suffix() == "kicad_sym") {
        //targetLibName = targetLibName;
    } else {
        targetLibName = targetLibName + ".kicad_sym";
    }
    targetLibName = cleanUpAndJoinFileName(libCreatorSettings.path_targetLibrary, targetLibName);

    if (deviceToBeCreated.isValid()) {
        targetLib.loadFile(targetLibName);
        bool proceed = true;
        if (targetLib.indexOf(deviceToBeCreated.get_symbol_name()) > -1) {
            QMessageBox msgBox;
            msgBox.setText("Part duplicate");
            msgBox.setInformativeText("A part with this name already exists. Overwrite old part?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Ok);
            proceed = QMessageBox::Ok == msgBox.exec();
        }
        if (proceed) {
            targetLib.insertDevice(deviceToBeCreated);
            if (targetLib.saveFile(targetLibName)) {
                ui->statusBar->showMessage("Inserted part " + deviceToBeCreated.get_symbol_name() + " to library: \"" + targetLibName + "\"", 10000);
            } else {
                ui->statusBar->showMessage("Could not save library \"" + targetLibName + "\"", 10000);
            }
        }
    } else {
        QMessageBox::warning(this, "current device invalid", "current device invalid");
    }
}

void MainWindow::on_lbl_targetFootprint_linkActivated(const QString &link) {
    setCurrentComboText(ui->cmb_targetFootprint, link);
}

void MainWindow::on_cmb_targetFootprint_currentTextChanged(const QString &arg1) {
    if (ui->cmb_targetFootprint->findText(arg1) == -1) {
        QPalette pal = ui->edt_targetDescription->palette();
        pal.setColor(ui->cmb_targetFootprint->backgroundRole(), Qt::red);
        pal.setColor(ui->cmb_targetFootprint->foregroundRole(), Qt::red);
        ui->cmb_targetFootprint->setPalette(pal);
    } else {
        QPalette pal = ui->edt_targetDescription->palette();
        ui->cmb_targetFootprint->setPalette(pal);
    }
}

void insertStandVariableValue(QMap<QString, QString> &variables, QString name, QString value, bool allowEmptyValue) {
    if (value.size() || allowEmptyValue) {
        if ((value != "example") || (!variables.contains(name))) {
            variables.insert(name, value);
        }
    }
}

void MainWindow::insertStandardVariablesToMap(QString supplier_prefix, QMap<QString, QString> &variables, QString footprint, QString reference,
                                              QString ruleName, QString mpn, QString manufacturer, QString description, QString OctoFootprint,
                                              QString OctoFootprintMetric_IPC) {
    insertStandVariableValue(variables, "%source.footprint%", footprint, false);
    insertStandVariableValue(variables, "%source.ref%", reference, false);
    insertStandVariableValue(variables, "%rule.name%", ruleName, false);
    insertStandVariableValue(variables, "%rule.name.saveFilename%", cleanUpFileNameNode(ruleName, false), false);

    insertStandVariableValue(variables, "%" + supplier_prefix + ".mpn%", mpn, true);
    insertStandVariableValue(variables, "%" + supplier_prefix + ".manufacturer%", manufacturer, true);
    insertStandVariableValue(variables, "%" + supplier_prefix + ".description%", description, true);
    insertStandVariableValue(variables, "%" + supplier_prefix + ".footprint%", OctoFootprint, true);
    insertStandVariableValue(variables, "%" + supplier_prefix + ".metric-ipc-footprint%", OctoFootprintMetric_IPC, true);
    insertStandVariableValue(variables, "%" + supplier_prefix + ".mpn.saveFilename%", cleanUpFileNameNode(mpn, false), true);
    insertStandVariableValue(variables, "%" + supplier_prefix + ".manufacturer.saveFilename%", cleanUpFileNameNode(manufacturer, false), true);

    insertStandVariableValue(variables, "%octo.mpn%", mpn, true);
    insertStandVariableValue(variables, "%octo.manufacturer%", manufacturer, true);
    insertStandVariableValue(variables, "%octo.description%", description, true);
    insertStandVariableValue(variables, "%octo.footprint%", OctoFootprint, true);
    insertStandVariableValue(variables, "%octo.metric-ipc-footprint%", OctoFootprintMetric_IPC, true);
    insertStandVariableValue(variables, "%octo.mpn.saveFilename%", cleanUpFileNameNode(mpn, false), true);                   //for backward compatibility
    insertStandVariableValue(variables, "%octo.manufacturer.saveFilename%", cleanUpFileNameNode(manufacturer, false), true); //for backward compatibility

    variables.insert("%target.id%", QString::number(QDateTime::currentMSecsSinceEpoch()));
}

QMap<QString, QString> MainWindow::createVariableMap() {
    QMap<QString, QString> variables = selectedOctopartMPN.getQueryResultMap();
    QString footprint = currentSourceDevice.get_property_value_str("Footprint");
    QString reference = currentSourceDevice.get_property_value_str("Reference");
    QString ruleName = ui->cmb_targetRuleName->currentText();
    QString mpn = selectedOctopartMPN.getMpn();
    QString manufacturer = selectedOctopartMPN.manufacturer;
    QString description = selectedOctopartMPN.description;
    QString OctoFootprint = selectedOctopartMPN.footprint;
    QString OctoFootprintMetric_IPC = selectedOctopartMPN.footprint;

    if (OctoFootprintMetric_IPC == "01005") {
        OctoFootprintMetric_IPC = "0402";
    } else if (OctoFootprintMetric_IPC == "0201") {
        OctoFootprintMetric_IPC = "0603";
    } else if (OctoFootprintMetric_IPC == "0402") {
        OctoFootprintMetric_IPC = "1005";
    } else if (OctoFootprintMetric_IPC == "0603") {
        OctoFootprintMetric_IPC = "1608";
    } else if (OctoFootprintMetric_IPC == "0805") {
        OctoFootprintMetric_IPC = "2012";
    } else if (OctoFootprintMetric_IPC == "1008") {
        OctoFootprintMetric_IPC = "2520";
    } else if (OctoFootprintMetric_IPC == "1206") {
        OctoFootprintMetric_IPC = "3216";
    } else if (OctoFootprintMetric_IPC == "1210") {
        OctoFootprintMetric_IPC = "3225";
    } else if (OctoFootprintMetric_IPC == "1806") {
        OctoFootprintMetric_IPC = "4516";
    } else if (OctoFootprintMetric_IPC == "1812") {
        OctoFootprintMetric_IPC = "4532";
    } else if (OctoFootprintMetric_IPC == "2010") {
        OctoFootprintMetric_IPC = "5025";
    } else if (OctoFootprintMetric_IPC == "2512") {
        OctoFootprintMetric_IPC = "6332";
    } else if (OctoFootprintMetric_IPC == "TSSOP") {
        OctoFootprintMetric_IPC = "SOP";
    } else if (OctoFootprintMetric_IPC == "TSOP") {
        OctoFootprintMetric_IPC = "SOP";
    } else if (OctoFootprintMetric_IPC == "PSOP") {
        OctoFootprintMetric_IPC = "SOP";
    } else if (OctoFootprintMetric_IPC == "SSOP") {
        OctoFootprintMetric_IPC = "SOP";
    } else if (OctoFootprintMetric_IPC == "QSOP") {
        OctoFootprintMetric_IPC = "SOP";
    } else if (OctoFootprintMetric_IPC == "VSOP") {
        OctoFootprintMetric_IPC = "SOP";
    } else if (OctoFootprintMetric_IPC == "SMB") {
        OctoFootprintMetric_IPC = "DIOM5336X240";
    } else if (OctoFootprintMetric_IPC == "SMC") {
        OctoFootprintMetric_IPC = "DIOM7959X241";
    } else if (OctoFootprintMetric_IPC == "SMA") {
        OctoFootprintMetric_IPC = "DIOM5025X231";
    }

    insertStandardVariablesToMap(selectedOctopartMPN.supplier_prefix, variables, footprint, reference, ruleName, mpn, manufacturer, description, OctoFootprint,
                                 OctoFootprintMetric_IPC);

    return variables;
}

void MainWindow::on_actionEdit_triggered() {
    RuleEditor ruleeditor(this);
    QStringList proposedCategories;
    QStringList proposedSourceDevices;
    ruleeditor.setRules(partCreationRuleList, proposedCategories, proposedSourceDevices);
    ruleeditor.exec();
    if (ruleeditor.result()) {
        partCreationRuleList = ruleeditor.getRules();
        partCreationRuleList.modified();
        loadRuleCombobox();
    }
}

void MainWindow::on_btn_editRule_clicked() {
    RuleEditor ruleeditor(this);
    QStringList proposedCategories;
    QStringList proposedSourceDevices;

    proposedSourceDevices << currentSourceLib.getName() + "/" + currentSourceDevice.get_symbol_name();
    for (auto cat : selectedOctopartMPN.categories) {
        QString name = cat.categorie_uid + "~" + cat.get_full_path();
        proposedCategories.append(name);
    }

    ruleeditor.setRules(partCreationRuleList, proposedCategories, proposedSourceDevices);
    ruleeditor.setCurrenRule(ui->cmb_targetRuleName->currentText());
    ruleeditor.setVariables(createVariableMap());
    ruleeditor.exec();
    if (ruleeditor.result()) {
        partCreationRuleList = ruleeditor.getRules();
        partCreationRuleList.modified();
        loadRuleCombobox();
    }
}

void MainWindow::on_btn_applyRule_clicked() {
    PartCreationRule currentRule = partCreationRuleList.getRuleByNameForAppliaction(ui->cmb_targetRuleName->currentText());

    //  if (currentRule.name != "")
    // even if name is empty a global rule may still be active.
    {
        QMap<QString, QString> variables = createVariableMap();
        PartCreationRuleResult creationRuleResult = currentRule.setKicadDeviceFieldsByRule(variables);
        ui->edt_targetDesignator->setText(creationRuleResult.designator);
        ui->edt_targetName->setText(creationRuleResult.name);
        setCurrentComboText(ui->cmb_targetFootprint, creationRuleResult.footprint);

        ui->lbl_targetFootprint->setText("<a href=" + creationRuleResult.footprint + ">" + creationRuleResult.footprint + "</a>");
        ui->edt_targetDatasheet->setText(creationRuleResult.datasheet);
        ui->edt_targetID->setText(creationRuleResult.id);
        ui->edt_targetMPN->setText(creationRuleResult.mpn);
        ui->edt_targetManufacturer->setText(creationRuleResult.manufacturer);
        ui->edt_targetDescription->setText(creationRuleResult.description);
        ui->edt_targetDisplayValue->setText(creationRuleResult.display_value);
        setCurrentComboText(ui->cmb_targetLibrary, creationRuleResult.lib_name);
    }
}

void MainWindow::oncmbOctoQueryEnterPressed() {
    on_pushButton_clicked();
}

void MainWindow::setDatasheetButton() {}

void MainWindow::on_edt_targetDatasheet_textChanged(const QString &arg1) {
    (void)arg1;
    setDatasheetButton();
}

void MainWindow::on_btn_show_datasheet_clicked() {}

void MainWindow::on_actionOptions_triggered() {
    OptionsDialog diag(libCreatorSettings);
    diag.exec();
    octopartInterface.setAPIKey(libCreatorSettings.octo_apikey);
    libCreatorSettings.complainAboutSettings(this);
}

void MainWindow::on_actionDesign_settings_triggered() {
    DesignSettings diag(libCreatorSettings, this);
    diag.exec();
}

void MainWindow::apply_appearence_settings() {
    ui->cmb_search_supplier->setCurrentIndex(appearance_settings.value("last_search_suppplier", "").toInt());
    ui->cbtn_exact_match->setChecked(appearance_settings.value("last_exact_matches_checked", false).toBool());
}

void MainWindow::save_appearence_settings() {
    appearance_settings.setValue("last_search_suppplier", ui->cmb_search_supplier->currentIndex());
    appearance_settings.setValue("last_exact_matches_checked", ui->cbtn_exact_match->isChecked());
}

void MainWindow::on_cmb_search_supplier_currentIndexChanged(int index) {
    ui->cbtn_exact_match->setVisible(index == 0);
}
