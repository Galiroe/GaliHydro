#include "stationselect.h"
#include "ui_stationselect.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QString>
#include <QSettings>

#include <qDebug>


stationSelect::stationSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::stationSelect)
{
    ui->setupUi(this);

    //Variable globale : fichier ini
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);
    QVector<QString> VcodeHydro (0);
    QVector<int> Vnosta (0);

    QString baremeDir = settings.value("BDD/bareme","C://").toString();
    QString localeDir = settings.value("BDD/locale","C://").toString();
    QString terrainDir = settings.value("BDD/terrain","C://").toString();

    //Recuperation des stations bulletin/traitement
    QRegExp regExCodeHydro("[A-Z][0-9]{7}");
    QString stations;
    QVector<QString> processingStations;
    QVector<QString> bulletinStations;
    QVector<QString> ignoredStations;

    stations = settings.value("Traitement/CodeHydro").toString();
    while (stations.contains(regExCodeHydro)) {
        processingStations.append(regExCodeHydro.cap(0));
        stations.remove(0,9);
        //        stations.remove(regExCodeHydro.cap(0));
    }

    stations = settings.value("Bulletin/CodeHydro").toString();
    while (stations.contains(regExCodeHydro)) {
        bulletinStations.append(regExCodeHydro.cap(0));
        stations.remove(0,9);
        //        stations.remove(regExCodeHydro.cap(0));
    }

    stations = settings.value("Ignored/CodeHydro").toString();
    while (stations.contains(regExCodeHydro)) {
        ignoredStations.append(regExCodeHydro.cap(0));
        stations.remove(0,9);
        //        stations.remove(regExCodeHydro.cap(0));
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    //Connexion a la base TERRAIN et recuperation des elements stations
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+baremeDir);
    if(db.open()) {
        QSqlQuery queryBareme;
        queryBareme.exec("SELECT nosta, codehydro, nom FROM station ORDER BY codehydro;");
        int i = 0;
        while (queryBareme.next()) {
            int nosta = queryBareme.value(0).toInt();
            QString codeHydro = queryBareme.value(1).toString();
            QString nomStation = queryBareme.value(2).toString().replace("_"," ").toUpper();
            Vnosta.append(nosta);
            VcodeHydro.append(codeHydro);

            QTableWidgetItem *item;
            item = new QTableWidgetItem(codeHydro);
            QTableWidgetItem *item2;
            item2 = new QTableWidgetItem(nomStation);

            ui->selectTable->setRowCount(i+1);
            ui->selectTable->setItem(i,0,item2);
            ui->selectTable->setItem(i,1,item);

            QTableWidgetItem *itemCheckTraitement = new QTableWidgetItem("");
            itemCheckTraitement->setFlags(itemCheckTraitement->flags()|Qt::ItemIsUserCheckable);
            if (processingStations.contains(codeHydro))
            {itemCheckTraitement->setCheckState(Qt::Checked);}
            else
            {itemCheckTraitement->setCheckState(Qt::Unchecked);}
            ui->selectTable->setItem(i,2,itemCheckTraitement);

            QTableWidgetItem *itemCheckBulletin = new QTableWidgetItem("");
            itemCheckBulletin->setFlags(itemCheckBulletin->flags()|Qt::ItemIsUserCheckable);
            if (bulletinStations.contains(codeHydro))
            {itemCheckBulletin->setCheckState(Qt::Checked);}
            else
            {itemCheckBulletin->setCheckState(Qt::Unchecked);}
            ui->selectTable->setItem(i,3,itemCheckBulletin);

            QTableWidgetItem *itemCheckIgnored = new QTableWidgetItem("");
            itemCheckIgnored->setFlags(itemCheckIgnored->flags()|Qt::ItemIsUserCheckable);
            if (ignoredStations.contains(codeHydro))
            {itemCheckIgnored->setCheckState(Qt::Checked);}
            else
            {itemCheckIgnored->setCheckState(Qt::Unchecked);}
            ui->selectTable->setItem(i,4,itemCheckIgnored);

            i++;
        }
        queryBareme.clear ();
        db.close();
    }


    //Connexion a la base LOCALE pour elimination des stations inutiles
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+localeDir);
    if (db.open()) {
        QSqlQuery queryBDLocale;
        int compteur = 0;
        for (int i=0;i<VcodeHydro.size();i++) {
            queryBDLocale.exec("SELECT * FROM "+VcodeHydro[i]);
            if (!queryBDLocale.next()) {
                ui->selectTable->removeRow(compteur);
                Vnosta.remove(compteur);
                compteur--;
            }
            compteur++;
        }
        queryBDLocale.clear();
        db.close();
    }

    //Connexion a la base TERRAIN  pour elimination des stations inutiles
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+terrainDir);
    if (db.open()) {
        QSqlQuery queryTerrain;
        int compteur = 0;
        for (int i=0;i<Vnosta.size();i++) {
            queryTerrain.exec("SELECT * FROM `Passages(Tp)` WHERE `nosta`="+QString::number(Vnosta[i])+";");
            if (!queryTerrain.next()) {
                ui->selectTable->removeRow(compteur);
                qDebug ()<< QString::number(Vnosta.size());
                qDebug ()<< QString::number(VcodeHydro.size());
                qDebug ()<< QString::number(compteur);
                VcodeHydro.remove(compteur);
                compteur--;
            }
            compteur++;
        }
        queryTerrain.clear();
        db.close();
    }

    QObject::connect(this, SIGNAL(accepted()), this, SLOT(acceptSelect()));
}

//stationSelect::stationSelect(const QVector<QString> &VcodeHydro, const QVector<QString> &VStation, QWidget *parent) :
//    QDialog(parent),
//    ui(new Ui::stationSelect)
//{
//    ui->setupUi(this);
//}

void stationSelect::acceptSelect()
{
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);
    QString stationCheckedP ("");
    QString stationCheckedB ("");
    QString stationCheckedI ("");
    for (int i=0;i<ui->selectTable->rowCount();i++) {
        if(ui->selectTable->item(i,2)->checkState()) {
            stationCheckedP = stationCheckedP + ui->selectTable->item(i,1)->text()+";";
        }
        if(ui->selectTable->item(i,3)->checkState()) {
            stationCheckedB = stationCheckedB + ui->selectTable->item(i,1)->text()+";";
        }
        if(ui->selectTable->item(i,4)->checkState()) {
            stationCheckedI = stationCheckedI + ui->selectTable->item(i,1)->text()+";";
        }
        settings.setValue("Traitement/CodeHydro",stationCheckedP);
        settings.setValue("Bulletin/CodeHydro",stationCheckedB);
        settings.setValue("Ignored/CodeHydro",stationCheckedI);
    }

    emit refreshTable();
}

stationSelect::~stationSelect()
{
    delete ui;
}
