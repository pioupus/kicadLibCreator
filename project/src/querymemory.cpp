#include "querymemory.h"
#include <QFile>
#include <QLineEdit>
#include <QTextStream>

QueryMemory::QueryMemory() {
  QFile file("oldqueries.conf");
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream in(&file);
    while (!in.atEnd()) {
      oldQueries.append(in.readLine());
    }
    file.close();
  }
  oldQueries.sort();
}

void QueryMemory::addQuery(QString s) {
  if (oldQueries.indexOf(s) == -1) {
    oldQueries.append(s);
    oldQueries.sort();
  }
}

void QueryMemory::save() {
  QFile file("oldqueries.conf");
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream out(&file);
    for (auto s : oldQueries) {
      out << s << Qt::endl;
    }
    file.close();
  }
}

void QueryMemory::loadQueryList(QComboBox *cmb) {
  QString tmp = cmb->currentText();
  cmb->setProperty("ignoreChanges", true);
  cmb->clear();
  int found_index = -1;
  for (auto s : oldQueries) {
    cmb->addItem(s);
    if (s == tmp) {
      found_index = cmb->count();
    }
  }
  if (found_index > -1) {
    cmb->setCurrentIndex(found_index);
  } else {
    cmb->setEditText(tmp);
  }
  cmb->setProperty("ignoreChanges", false);
}
