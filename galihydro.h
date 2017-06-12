#ifndef GALIHYDRO_H
#define GALIHYDRO_H

#include <QMainWindow>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QLabel>
#include <QTableWidgetItem>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QCryptographicHash>

#include "bdddir.h"
#include "options.h"
#include "stationSelect.h"
#include "groupedit.h"

enum stationType {all,processing,bulletin,myAll,myProcessing,myBulletin,group};

namespace Ui {
class galihydro;
}

class galihydro : public QMainWindow
{
    Q_OBJECT

public:
    explicit galihydro(QWidget *parent = 0);
    ~galihydro();

signals:

private slots:
    void ouvrirBddDir();
//    void gestionStations (const QVector<QString> &VcodeHydro, const QVector<QString> &VStation);
    void gestionStations ();
    void gestionGroups ();
    void ouvrirAPrpops ();
    void stationsProcessing ();
    void stationsBulletin ();
    void stationsMyProcessing ();
    void stationsMyBulletin ();
    void stationsMy ();
    void stationsAll ();
    void refresh ();
    void createRow (int row,QString codeHydro,QString nomStation,QString courEau);
    void ouvrirOptions ();
    void delParam();
    void hideColumn (bool checked, int column);
    void setTolerance (int newTolerance);
    void setTimeOutDay (int newTimeOutDay);
    void setTimeOutDayStatus(bool status);
    void setMethod (processingMethod newMethod);
    void sortColumn (int index);
    void loadGroup (QString groupName = NULL);

private:
    //Fonctions privées
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent * event);
    void showStations (enum stationType stationType = all);
    bool getConfigMode ();
    void getGroupName ();

    //Attribus privés
    BddDir *bddDir;
    options *optionsWindow;
    stationSelect *staS;
    groupedit *groupE;
    int tolerance;
    int timeOutDay;
    int fixedDate;
    Ui::galihydro *ui;
    int cpteProcessing;
    int cpteBulletin;
    int cpteTimeOutDay;
    QLabel *statusMessage;
    bool timeOutDayStatus;
    QDate currentDate;
    QDate processingDate;
    processingMethod method;
    bool adminMode;
    QVector<QString> groupStations;
};

#endif // GALIHYDRO_H
