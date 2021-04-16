#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QProgressBar>

#include "digikeyinterface.h"
#include "httprequestworker.h"
#include "kicadfile_lib.h"
#include "libcreatorsettings.h"
#include "octopartinterface.h"
#include "partcreationrule.h"
#include "querymemory.h"
#include "remotedatasource.h"
#include "renderarea.h"
#include <QListWidgetItem>

class MainWindow;

class AppearenceSettings {
    public:
    AppearenceSettings(QString filename);

    void apply_appearence_settings(MainWindow &mainwindow);
    void save_appearence_settings(MainWindow &mainwindow);

    private:
    QSettings m_settings;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);
    static QString cleanUpFileNameNode(QString filename, bool allowSeparatorLikeChars);
    static void insertStandardVariablesToMap(QString supplier_prefix, QMap<QString, QString> &variables, QString footprint, QString reference, QString ruleName,
                                             QString mpn, QString manufacturer, QString description, QString OctoFootprint, QString OctoFootprintMetric_IPC);
    QString cleanUpAndJoinFileName(QString filename_root, QString filename_relative);
    void tableOctopartResult_linkclicked(int row, int column);
    void openHttpLink(QString url);

    public slots:
    void setProgressbar(int progress, int total);
    private slots:
    void on_pushButton_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_list_input_libraries_currentRowChanged(int currentRow);

    void on_tableOctopartResult_cellActivated(int row, int column);
    void onQuickLinkClicked(QString s);

    void on_btnCreatePart_clicked();

    void on_list_input_devices_currentRowChanged(int currentRow);

    void on_lbl_targetFootprint_linkActivated(const QString &link);

    void on_cmb_targetFootprint_currentTextChanged(const QString &arg1);

    void on_actionEdit_triggered();

    void on_btn_editRule_clicked();

    void on_btn_applyRule_clicked();
    void oncmbOctoQueryEnterPressed();

    void on_pushButton_3_clicked();

    void on_edt_targetDatasheet_textChanged(const QString &arg1);

    void on_btn_show_datasheet_clicked();

    void on_actionOptions_triggered();

    void on_tableOctopartResult_cellDoubleClicked(int row, int column);

    void on_comboBox_editTextChanged(const QString &arg1);

    void on_tableOctopartResult_cellClicked(int row, int column);
    void search_request_finished();
    void search_request_started();
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_tableOctopartResult_itemSelectionChanged();

    void on_list_input_devices_itemDoubleClicked(QListWidgetItem *item);

    void showDatasheet();

    void on_actionDesign_settings_triggered();

    void on_lbl_productlink_linkActivated(const QString &link);

    void apply_appearence_settings();
    void save_appearence_settings();

    void on_cmb_search_supplier_currentIndexChanged(int index);

    private:
    Ui::MainWindow *ui;
    OctopartInterface octopartInterface;
    OctopartCategorieCache octopartCategorieCache;
    DigikeyWrapper digikeyInterface;

    LibCreatorSettings libCreatorSettings;
    QStringList sourceLibraryPaths;

    QList<RemoteDataSource_Result_QueryMPN_Entry> queryResults;
    RemoteDataSource_Result_QueryMPN_Entry selectedOctopartMPN;

    KICADLibSchematicDevice currentSourceDevice;

    KICADLibSchematicDeviceLibrary currentSourceLib;
    KICADLibFootprintLibrary fpLib;

    PartCreationRuleList partCreationRuleList;
    void clearQuickLinks(QLayout *layout);

    QMap<QString, QString> createVariableMap();
    void setDatasheetButton();
    void loadRuleCombobox();
    void resetSearchQuery(bool resetAlsoTable);
    QSettings appearance_settings;
    QString downloadDatasheet(bool force);
    QueryMemory querymemory;

    QString getDataSheetFileName(bool relativePath);
    QProgressBar *progressbar;

    RenderArea *renderarea;

    KICADLibSchematicDevice createDevicePropertiesFromGui(KICADLibSchematicDevice &sourceDevice, QString realDatasheetPath);
};

#endif // MAINWINDOW_H
