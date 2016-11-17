#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "httprequestworker.h"
#include "octopartinterface.h"
#include "libcreatorsettings.h"


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

private:
    Ui::MainWindow *ui;

    OctopartInterface octopartInterface;

    OctopartCategorieCache octopartCategorieCache;

    LibCreatorSettings libCreatorSettings;
    QStringList libraryPaths;

    QList<OctopartResult_QueryMPN_Entry> queryResults;
    OctopartResult_QueryMPN_Entry selectedOctopartMPN;

};

#endif // MAINWINDOW_H
