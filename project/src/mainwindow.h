#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "httprequestworker.h"
#include "octopartinterface.h"
#include "libcreatorsettings.h"
#include "kicadfile_lib.h"
#include "partcreationrule.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



    void closeEvent(QCloseEvent *event);
private slots:
    void on_pushButton_clicked();
    void octopart_request_finished();
    void on_pushButton_2_clicked();
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

private:
    Ui::MainWindow *ui;

    OctopartInterface octopartInterface;

    OctopartCategorieCache octopartCategorieCache;

    LibCreatorSettings libCreatorSettings;
    QStringList sourceLibraryPaths;

    QList<OctopartResult_QueryMPN_Entry> queryResults;
    OctopartResult_QueryMPN_Entry selectedOctopartMPN;
    KICADLibSchematicDevice currentSourceDevice;

    KICADLibSchematicDevice targetDevice;

    KICADLibSchematicDeviceLibrary currentSourceLib;
    KICADLibFootprintLibrary fpLib;

    void setCurrentDevicePropertiesFromGui();

    PartCreationRuleList partCreationRuleList;
    void clearQuickLinks(QLayout* layout);

};

#endif // MAINWINDOW_H
