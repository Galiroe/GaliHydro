#include "corth.h"
#include "ui_corth.h"

#include <QSettings>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QDateTime>

//Optimiser creation du tableau de station (creation puis supression de colonne -> inutile)
//Ignoré station de la liste "ignorer"
//Afficher uniquement un paquet/stations perso
//Couleur sur corth non a zero ?


corth::corth(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::corth)
{
    //Chargement de l'interface graphique
    ui->setupUi(this);

    //Configuration de la BDD
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);
    QString baremeDir = settings.value("BDD/bareme","C://").toString();
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    //RegExp dateTime
    QRegExp regExDateTime("^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):[0-9]{2}$");

    //Connexion a la base BAREME et recuperation des informations
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+baremeDir);
    if(db.open()) {

        //Query pour recuperation du numero station et de la date maximum
        QSqlQuery queryDate;
        queryDate.exec("SELECT courbecorrection.nosta, MAX(courbecorrection.ladate) FROM courbecorrection GROUP BY nosta");

        //Creation d'un tableau dynamique 2 dimensions pour stocker le resultat de la requete
        QVector< QVector<QString> > vectorDate = QVector< QVector<QString> >();
        while (queryDate.next()) {
            QVector< QString > lineDate (0);
            QString nosta = queryDate.value(0).toString();
            QString ladate = queryDate.value(1).toString();
            lineDate.push_back(nosta);
            lineDate.push_back(ladate);
            vectorDate.push_back(lineDate);
        }



        //Query pour recuperation des informations station dela table station et jointure avec la table courbecorrectionde pour recuperation de la date
        QSqlQuery queryValeur;
        queryValeur.exec("SELECT S.nosta, C.ladate, S.nom, S.codehydro, S.courdo, C.valeur FROM station AS S INNER JOIN courbecorrection AS C ON S.nosta = C.nosta ORDER BY C.nosta ASC");

        //Creation d'un tableau dynamique 2 dimensions pour stocker le resultat de la requete
        QVector< QVector<QString> > vectorInfos = QVector< QVector<QString> >();
        while (queryValeur.next()) {
            QVector< QString > ligneInfos (0);
            QString nosta2 = queryValeur.value(0).toString();
            QString ladate2 = queryValeur.value(1).toString();
            QString nom = queryValeur.value(2).toString();
            QString courdo = queryValeur.value(3).toString();
            QString codehydro = queryValeur.value(4).toString();
            QString valeur = queryValeur.value(5).toString();

            ligneInfos.push_back(nosta2);
            ligneInfos.push_back(nom);
            ligneInfos.push_back(courdo);
            ligneInfos.push_back(codehydro);
            ligneInfos.push_back(ladate2);
            ligneInfos.push_back(valeur);

            vectorInfos.push_back(ligneInfos);
        }

        //Assemblage des tableaux des deux tableaux pour un numero station (nosta) et une date (ladate) identique
        ui->corthTable->setColumnCount(8);
        //Boucle de creation et de remplissage des lignes
        for (int i=0; i<vectorDate.size();i++) {
            ui->corthTable->insertRow(i);
            //Boucle de remplissage des colonnes a partir du premier tableau puis du second
            for (int x=0; x<vectorDate[i].size();x++) {
                QString dataString = vectorDate[i][x];
                QTableWidgetItem *itemDate = new QTableWidgetItem;
                itemDate->setText(dataString);
                ui->corthTable->setItem(i, x, itemDate);
                //Boucle de comparaison puis de remplissage des colonnes depuis le second tableau
                for (int match=0; match<vectorInfos.size();match++) {
                    //Comparaison avec le premier tableau quand le numero station (nosta) et la date max (ladate) sont identique
                    if (vectorDate[i][0]==vectorInfos[match][0] && vectorDate[i][1]==vectorInfos[match][4]) {
                        //Boucle de remplissage avec les informations du second tableau
                        for (int z=0; z<vectorInfos[match].size();z++) {
                            QString dataStation = vectorInfos[match][z];
                            QTableWidgetItem *itemInfos = new QTableWidgetItem;
                            //Converstion du format de la date
                            if (z==4) {
                                if (dataStation.contains(regExDateTime))
                                {
                                    QDateTime dateTime;
                                    dateTime.setDate(QDate(regExDateTime.cap(1).toInt(),regExDateTime.cap(2).toInt(),regExDateTime.cap(3).toInt()));
                                    dateTime.setTime(QTime(regExDateTime.cap(4).toInt(),regExDateTime.cap(5).toInt()));
                                    itemInfos->setData (0 ,dateTime);
                                }
                            }
                            else
                                itemInfos->setText(dataStation.toUpper().replace("_"," "));

                            ui->corthTable->setItem(i, z+2, itemInfos);
                        }
                    }
                }
            }
        }
        ui->corthTable->removeColumn(0);
        ui->corthTable->removeColumn(0);
        ui->corthTable->removeColumn(0);

        QStringList headerLabelList = (QStringList() << "Station" << "Code Hydro" << "Cours d'eau" << "Date" << "Valeur");
        ui->corthTable->setHorizontalHeaderLabels(headerLabelList);

        ui->corthTable->sortItems(2);


        //Parametrage de la taille des lignes (fixé) et colonnes (suivant contenus)
        ui->corthTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->corthTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

        //Affichage du tableau final
        ui->corthTable->show();
    }
}

corth::~corth()
{
    delete ui;
}
