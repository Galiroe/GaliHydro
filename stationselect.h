#ifndef STATIONSELECT_H
#define STATIONSELECT_H

#include <QDialog>

namespace Ui {
class stationSelect;
}

class stationSelect : public QDialog
{
    Q_OBJECT

public:
    explicit stationSelect(QWidget *parent = 0);
    ~stationSelect();
//    explicit stationSelect(const QVector<QString> &VcodeHydro, const QVector<QString> &VStation,QWidget *parent = 0);

signals:
    void refreshTable ();

private slots :
    void acceptSelect();

private:
    Ui::stationSelect *ui;
};

#endif // STATIONSELECT_H
