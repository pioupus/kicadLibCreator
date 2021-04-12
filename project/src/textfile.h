#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <QDialog>

namespace Ui {
    class TextFile;
}

class TextFile : public QDialog {
    Q_OBJECT

    public:
    explicit TextFile(QWidget *parent = 0);
    void setText(QString filepath);
    ~TextFile();

    private:
    Ui::TextFile *ui;
};

#endif // TEXTFILE_H
