#ifndef GROUPEDIT_H
#define GROUPEDIT_H

#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVector>
#include <QString>
#include <QRegExp>


namespace Ui {
class groupedit;
}

class groupedit : public QDialog
{
    Q_OBJECT

public:
    explicit groupedit(QWidget *parent = 0);
    ~groupedit();

private slots:
    void addGroup();
    void dellGroup ();
    void saveGroup ();
    void cancelGroup ();
    void closeGroup ();
    void loadGroup (int currentRow);

private:
    Ui::groupedit *ui;
//    int currentGroup;
};

#endif // GROUPEDIT_H
