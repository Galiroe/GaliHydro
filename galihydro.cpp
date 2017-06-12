#include "galihydro.h"
#include "ui_galihydro.h"
#include "mqaction.h"
#include "bdd_singleton.h"
#include "apropos.h"

#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QTableWidget>
#include <QFile>
#include <QMessageBox>
#include <QHeaderView>
#include <QTableView>
#include <QSettings>

#include <QList>
#include <QMenu>
#include <QDebug>

//Definition des colonnes et de leurs index
#define c_nomStation 0
#define c_codeHydro 1
#define c_courDo 2
#define c_dateTraitement 3
#define c_heureTraitement 4
#define c_datePassage 5
#define c_heurePassage 6
#define c_coteEchelle 7
#define c_coteStation 8
#define c_recalage 9
#define c_NetSeuil 10
#define c_comment 11

//Sels de hachage
#define SEL_DEBUT "NOSPY"
#define SEL_FIN "NOFLIC"

//Regler crash si tentative d'afficher le lastGroup au demmarrage alors qu'il n'y a aucun lastGroup
//Finir ajout fonction de selection et d'affichage des paquets
//Afficher dernier paquet utilisé a l'ouverture comme "mes stations, stations dreal..."
//Actualiser le mode admin ou non apres modif mdp + apres modif paquets (dont affichage de l'option paquet)
//relier signal changement de method
//creer classe de haschage
//Trouver meilleurs methode pour suppression menu "mes paquetes" si non gestonnaire

//void QHeaderView::setSectionResizeMode(ResizeMode mode)
//QHeaderView::ResizeToContents

//Alerté de l'echec de recuperation du nom utilisateur dans param environnement ? (getconfigmode)
//Mode administrateur ou gestionnaire ?

galihydro::galihydro(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::galihydro)
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    //Recuperation de la tolerance entre le traitement et le passage
    tolerance = settings.value ("Tolerance",15).toInt();

    //Initialisation de la date actuelle
    currentDate = QDate::currentDate();

    //Recuperation du delai entre deux traitements de du status de l'option
    timeOutDay = settings.value ("TimeOutDay/Value",45).toInt();
    timeOutDayStatus = settings.value ("TimeOutDay/Enabled",true).toBool();

    //Recupere la methode de traitement
    if (settings.value("Method").toString()== "fixedDate") {
        method = fixedDateMethod;
    }
    else {
        method = afterPassageMethod;
    }

    //Recuperation de l'interface graphique
    ui->setupUi(this);

    //Recuperation du mode d'utilisation
    adminMode = getConfigMode();

    //Reglage du mode (admin ou non)
    if (!adminMode){
        ui->actionGerer_les_paquets->setVisible(false);
        ui->actionGerer_les_paquets->setEnabled(false);

        //Suppresion du menu Mes paquets
        delete ui->menuMes_paquets;
//        ui->menuMes_paquets->setVisible(false);
//        ui->menuMes_paquets->setEnabled(false);
    }
    else{
        getGroupName ();
    }

    //Recuperation de la taille de la fenetre
    setGeometry (settings.value("Geometry/x",25).toInt(),settings.value("Geometry/y",25).toInt(),settings.value("Geometry/Width",625).toInt(),settings.value("Geometry/Height",625).toInt());

    //Initialisation du message de la statut bar
    statusMessage = new QLabel;
    ui->statusBar->addWidget(statusMessage);

    //Parametrage de la taille des lignes (fixé) et colonnes (suivant contenus)
    ui->StationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->StationTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

//test
//    ui->StationTable->resizeColumnsToContents(); //marche APRES remplissage du tableau
//    ui->StationTable->horizontalHeader()->setStretchLastSection(true); // Ne rend pas l'ensemble du texte visible
//fin test
//    if (ui->StationTable->columnWidth(c_comment) <= )
//    ui->StationTable->horizontalHeader()->;
//    setColumnWidth(int column, int width)

//    QHeaderView *headers = ui->StationTable->horizontalHeader();
//    headers->setResizeMode(QHeaderView::ResizeToContents);
//QHeaderView::ResizeToContents

    //Initialisation de la variable de visibilité d'une colonne
    bool columnState = true;

    //Creation des QAction personalisées pour affichage/camouflage des colonnes
    MQAction *Station = new MQAction ("Station" ,this);
        Station->setCheckable(true);
        Station->setColumn(c_nomStation);
        columnState = settings.value ("Column/"+QString::number(c_nomStation),true).toBool();
        Station->setChecked(columnState);
        hideColumn (columnState, c_nomStation);
    MQAction *CodeHydro = new MQAction ("Code Hydro", this);
        CodeHydro->setCheckable(true);
        CodeHydro->setColumn(c_codeHydro);
        columnState = settings.value ("Column/"+QString::number(c_codeHydro),true).toBool();
        CodeHydro->setChecked(columnState);
        hideColumn (columnState, c_codeHydro);
    MQAction *CourDo = new MQAction ("Cours d'eau", this);
        CourDo->setCheckable(true);
        CourDo->setColumn(c_courDo);
        columnState = settings.value ("Column/"+QString::number(c_courDo),true).toBool();
        CourDo->setChecked(columnState);
        hideColumn (columnState, c_courDo);
    MQAction *DateTraitement = new MQAction ("Date Traitement", this);
        DateTraitement->setCheckable(true);
        DateTraitement->setColumn(c_dateTraitement);
        columnState = settings.value ("Column/"+QString::number(c_dateTraitement),true).toBool();
        DateTraitement->setChecked(columnState);
        hideColumn (columnState, c_dateTraitement);
    MQAction *HeureTraitement = new MQAction ("Heure Traitement", this);
        HeureTraitement->setCheckable(true);
        HeureTraitement->setColumn(c_heureTraitement);
        columnState = settings.value ("Column/"+QString::number(c_heureTraitement),true).toBool();
        HeureTraitement->setChecked(columnState);
        hideColumn (columnState, c_heureTraitement);
    MQAction *DatePassage = new MQAction ("Date Passage", this);
        DatePassage->setCheckable(true);
        DatePassage->setColumn(c_datePassage);
        columnState = settings.value ("Column/"+QString::number(c_datePassage),true).toBool();
        DatePassage->setChecked(columnState);
        hideColumn (columnState, c_datePassage);
    MQAction *HeurePassage = new MQAction ("Heure Passage", this);
        HeurePassage->setCheckable(true);
        HeurePassage->setColumn(c_heurePassage);
        columnState = settings.value ("Column/"+QString::number(c_heurePassage),true).toBool();
        HeurePassage->setChecked(columnState);
        hideColumn (columnState, c_heurePassage);
    MQAction *CoteEchelle = new MQAction ("Côte échelle", this);
        CoteEchelle->setCheckable(true);
        CoteEchelle->setColumn(c_coteEchelle);
        columnState = settings.value ("Column/"+QString::number(c_coteEchelle),true).toBool();
        CoteEchelle->setChecked(columnState);
        hideColumn (columnState, c_coteEchelle);
    MQAction *CoteStation = new MQAction ("Côte station", this);
        CoteStation->setCheckable(true);
        CoteStation->setColumn(c_coteStation);
        columnState = settings.value ("Column/"+QString::number(c_coteStation),true).toBool();
        CoteStation->setChecked(columnState);
        hideColumn (columnState, c_coteStation);
    MQAction *Recalage = new MQAction ("Recalage", this);
        Recalage->setCheckable(true);
        Recalage->setColumn(c_recalage);
        columnState = settings.value ("Column/"+QString::number(c_recalage),true).toBool();
        Recalage->setChecked(columnState);
        hideColumn (columnState, c_recalage);
    MQAction *NettoyageSeuil = new MQAction ("Nettoyage Seuil", this);
        NettoyageSeuil->setCheckable(true);
        NettoyageSeuil->setColumn(c_NetSeuil);
        columnState = settings.value ("Column/"+QString::number(c_NetSeuil),true).toBool();
        NettoyageSeuil->setChecked(columnState);
        hideColumn (columnState, c_NetSeuil);
    MQAction *Commentaire = new MQAction ("Commentaire", this);
        Commentaire->setCheckable(true);
        Commentaire->setColumn(c_comment);
        columnState = settings.value ("Column/"+QString::number(c_comment),true).toBool();
        Commentaire->setChecked(columnState);
        hideColumn (columnState, c_comment);

    //Ajout des QAction affichage/camouflage a la barre de menu
    ui->menuColonnes->addAction(Station);
    ui->menuColonnes->addAction(CodeHydro);
    ui->menuColonnes->addAction(CourDo);
    ui->menuColonnes->addAction(DateTraitement);
    ui->menuColonnes->addAction(HeureTraitement);
    ui->menuColonnes->addAction(DatePassage);
    ui->menuColonnes->addAction(HeurePassage);
    ui->menuColonnes->addAction(CoteEchelle);
    ui->menuColonnes->addAction(CoteStation);
    ui->menuColonnes->addAction(Recalage);
    ui->menuColonnes->addAction(NettoyageSeuil);
    ui->menuColonnes->addAction(Commentaire);

    //Connection QAction signaux/slots pour affichage/camouflage des colonnes
    QObject::connect(Station, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(CodeHydro, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(CourDo, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(DateTraitement, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(HeureTraitement, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(DatePassage, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(HeurePassage, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(CoteEchelle, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(CoteStation, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(Recalage, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(NettoyageSeuil, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));
    QObject::connect(Commentaire, SIGNAL(mTriggered (bool, int)), this, SLOT(hideColumn (bool, int)));

    //Connection signaux/slots
    QObject::connect(ui->actionGerer_les_BDD, SIGNAL(triggered()), this, SLOT(ouvrirBddDir()));
    QObject::connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(ouvrirOptions()));
    QObject::connect(ui->actionGerer_les_stations, SIGNAL(triggered()), this, SLOT(gestionStations()));
    QObject::connect(ui->actionGerer_les_paquets, SIGNAL(triggered()), this, SLOT(gestionGroups()));
    QObject::connect(ui->actionIntegralite, SIGNAL(triggered()), this, SLOT(stationsMy()));
    QObject::connect(ui->actionToutes_les_stations, SIGNAL(triggered()), this, SLOT(stationsAll()));
    QObject::connect(ui->actionStations_bulettin, SIGNAL(triggered()), this, SLOT(stationsBulletin()));
    QObject::connect(ui->actionNon_traitees, SIGNAL(triggered()), this, SLOT(stationsProcessing()));
    QObject::connect(ui->actionStations_Hors_Bulettin, SIGNAL(triggered()), this, SLOT(stationsMyProcessing()));
    QObject::connect(ui->actionStations_myBulletin, SIGNAL(triggered()), this, SLOT(stationsMyBulletin ()));
    QObject::connect(ui->actionActualiser, SIGNAL(triggered()), this, SLOT(refresh()));
    QObject::connect(ui->actionA_propos_de_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(ui->actionSupprimer_les_parametres, SIGNAL(triggered()), this, SLOT(delParam()));
    QObject::connect(ui->actionQuitter, SIGNAL(triggered()), qApp, SLOT(quit()));
    QObject::connect(ui->actionA_propos, SIGNAL(triggered()), this, SLOT(ouvrirAPrpops()));
    QObject::connect(ui->StationTable->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sortColumn (int)));

    //Creation et affichage du tableau de station
    refresh();
}

//Enregistre la taille de la fenetre si modification
void galihydro::resizeEvent(QResizeEvent *event)
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);
    settings.setValue("Geometry/Width",event->size().width());
    settings.setValue("Geometry/Height",event->size().height());
}

//Enregistre la posistion de la fenetre si deplacement
void galihydro::moveEvent(QMoveEvent * event)
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);
    settings.setValue("Geometry/x",event->pos().x());
    settings.setValue("Geometry/y",event->pos().y());
}

//Creation et affichage de la fenetre de parametrage des chemins de base de données
void galihydro::ouvrirBddDir()
{
    bddDir = new BddDir (this);
    QObject::connect(bddDir, SIGNAL(refreshTable()), this, SLOT(refresh()));
    bddDir->show();
}

//Creation et affichage de la fenetre A propos
void galihydro::ouvrirAPrpops ()
{
    aPropos *fenAPropos = new aPropos (this);
    fenAPropos->show();
}

//Creation et affichage de la fenetre des options
void galihydro::ouvrirOptions()
{
    optionsWindow = new options (this);
    QObject::connect(optionsWindow, SIGNAL(changeTolerance(int)), this, SLOT(setTolerance(int)));
    QObject::connect(optionsWindow, SIGNAL(changeTimeOutDay(int)), this, SLOT(setTimeOutDay(int)));
    QObject::connect(optionsWindow, SIGNAL(ChangeTimeOutDayStatus(bool)), this, SLOT(setTimeOutDayStatus(bool)));
    QObject::connect(optionsWindow, SIGNAL(methodChanged (processingMethod)), this, SLOT(setMethod (processingMethod)));
    QObject::connect(optionsWindow, SIGNAL(refreshTable()), this, SLOT(refresh()));
    optionsWindow->show();
}

//Creation et affichage de la fenetre de selection des stations a traiter
void galihydro::gestionStations ()
{
    //void galihydro::gestionStations (const QVector<QString> &VcodeHydro, const QVector<QString> &VStation)
    //stationSelect *staS = new stationSelect (VcodeHydro,VStation);
    staS = new stationSelect (this);
    QObject::connect(staS, SIGNAL(refreshTable()), this, SLOT(refresh()));
    staS->show();
}

//Creation et affichage de la fenetre de gestion des paquets de station
void galihydro:: gestionGroups ()
{
    groupE = new groupedit (this);
    groupE->show();
}

//SLOT : creation et affichage du tableau de toutes les stations
void galihydro::stationsAll ()
{
    showStations(all);
}

//SLOT : creation et affichage du tableau de toutes les stations à traiter hors bulletin
void galihydro::stationsProcessing ()
{
    showStations(processing);
}

//SLOT : creation et affichage du tableau des stations du bulletin
void galihydro::stationsBulletin ()
{
    showStations(bulletin);
}

//SLOT : creation et affichage du tableau des stations du paquet à traiter hors bulletin
void galihydro::stationsMyProcessing ()
{
    showStations(myProcessing);
}

//SLOT : creation et affichage du tableau des stations bulletin du paquet à traiter
void galihydro::stationsMyBulletin ()
{
    showStations(myBulletin);
}

//SLOT : creation et affichage du tableau de toutes les stations du paquet à traiter
void galihydro::stationsMy ()
{
    showStations(myAll);
}

//Creation et affichage du tableau des stations suivant le besoin
void galihydro::showStations (enum stationType stationType)
{
    cpteBulletin = 0;
    cpteProcessing = 0;
    cpteTimeOutDay = 0;

    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    //Récuperation de la date de traitement fixe si besoin. Adapte le jour pour obtenir une date valie
    fixedDate = settings.value ("FixedDate",1).toInt();
    if (method == fixedDateMethod) {
        processingDate = QDate (currentDate.year(),currentDate.month(),fixedDate);
        bool valide = processingDate.isValid();
        while (!valide) {
            fixedDate --;
            processingDate = QDate (currentDate.year(),currentDate.month(),fixedDate);
            if (processingDate.isValid())
                valide = true;
        }
    }

    QColor processingColor (settings.value("Traitement/Red",219).toInt(),settings.value("Traitement/Green",239).toInt(),settings.value("Traitement/Blue",255).toInt());
    QColor bulletinColor (settings.value("Bulletin/Red",250).toInt(),settings.value("Bulletin/Green",150).toInt(),settings.value("Bulletin/Blue",45).toInt());
    QColor colorTimeOutDay (settings.value("TimeOutDay/Red",200).toInt(),settings.value("TimeOutDay/Green",0).toInt(),settings.value("TimeOutDay/Blue",0).toInt());

    QVector<int> Vnosta (0);
    QVector<QString> VcodeHydro (0);
    int nbStation (0);
    QRegExp regExCodeHydro("[A-Z][0-9]{7}");
    QRegExp regDateTime ("^[0-9]{2}([0-9]{2})-([0-9]{2})-([0-9]{2})T([0-9]{2}:[0-9]{2}):[0-9]{2}$");
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
//    BDD_Singleton::instance(); BDD_Singleton::m_Instance;
    QString terrainDir = settings.value("BDD/terrain","C://").toString();
    QString baremeDir = settings.value("BDD/bareme","C://").toString();
    QString localeDir = settings.value("BDD/locale","C://").toString();

    //Recuperation des stations bulletin/traitement
    QString stations;
    QVector<QString> processingStations;
    QVector<QString> bulletinStations;
    QVector<QString> ignoredStations;

    stations = settings.value("Traitement/CodeHydro").toString();
    while (stations.contains(regExCodeHydro)) {
        processingStations.append(regExCodeHydro.cap(0));
        stations.remove(0,9);
    }

    stations = settings.value("Bulletin/CodeHydro").toString();
    while (stations.contains(regExCodeHydro)) {
        bulletinStations.append(regExCodeHydro.cap(0));
        stations.remove(0,9);
    }

    stations = settings.value("Ignored/CodeHydro").toString();
    while (stations.contains(regExCodeHydro)) {
        ignoredStations.append(regExCodeHydro.cap(0));
        stations.remove(0,9);
    }


    //Connexion a la base BAREME et recuperation des elements
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+baremeDir);
    if(db.open()) {
        QSqlQuery queryBareme;
        if (stationType == myAll || stationType == bulletin || stationType == myBulletin || stationType == myProcessing  || stationType == group) {
            ui->StationTable->clearContents();
            ui->StationTable->setRowCount(0);
            QString myStations;
            if (stationType == myAll) {
                myStations = settings.value("Traitement/CodeHydro").toString();
                settings.setValue("ShowOption/Last","myAll");
            }
            else if (stationType == myProcessing){
                QString processingString = settings.value("Traitement/CodeHydro").toString();
                QString bulletinString = settings.value("Bulletin/CodeHydro").toString();
                while (processingString.contains(regExCodeHydro)) {
                    QString currentStation = regExCodeHydro.cap(0);
                    if (!bulletinString.contains(currentStation)){
                        myStations.append(currentStation+";");
                    }
                    processingString.remove(0,9);
                }
                settings.setValue("ShowOption/Last","myProcessing");
            }
            else if (stationType == myBulletin) {
                QString processingString = settings.value("Traitement/CodeHydro").toString();
                QString bulletinString = settings.value("Bulletin/CodeHydro").toString();
                while (bulletinString.contains(regExCodeHydro)) {
                    QString currentStation = regExCodeHydro.cap(0);
                    if (processingString.contains(currentStation)){
                        myStations.append(currentStation+";");
                    }
                    bulletinString.remove(0,9);
                }
                settings.setValue("ShowOption/Last","myBulletin");
            }
            else if (stationType == group){
                    for (int i=0;i<groupStations.count();i++) {
                        myStations.append(groupStations[i]+";");
                    }
                    qDebug () << "Paquet a afficher :" << myStations;
                    settings.setValue("ShowOption/Last","group");
            }
            else {
                myStations = settings.value("Bulletin/CodeHydro").toString();
                settings.setValue("ShowOption/Last","bulletin");
            }

            //Récuperation des données de la base Bareme
            int i = 0;
            while (myStations.contains(regExCodeHydro)) {
                QString currentStation = regExCodeHydro.cap(0);
                VcodeHydro.append(currentStation);
                myStations.remove(0,9);
                queryBareme.exec("SELECT nosta, nom, courdo FROM station WHERE codehydro='"+currentStation+"'");
                if (queryBareme.next()) {
                    Vnosta.append(queryBareme.value(0).toInt());

                    createRow (i,currentStation,queryBareme.value(1).toString(),queryBareme.value(2).toString());
                    i++;
                }
            }
        }
        else {
            ui->StationTable->clearContents();
            ui->StationTable->setRowCount(0);
            settings.setValue("ShowOption/Last","all");
            queryBareme.exec("SELECT nosta, codehydro, nom, courdo FROM station ORDER BY codehydro;");
            int i = 0;
            while (queryBareme.next()) {

                int nosta = queryBareme.value(0).toInt();
                QString codeHydro = queryBareme.value(1).toString();
                QString nomStation = queryBareme.value(2).toString();
                QString courEau = queryBareme.value(3).toString();

                VcodeHydro.append(codeHydro);
                Vnosta.append(nosta);

                createRow (i,codeHydro,nomStation,courEau);
                i++;
            }
        }
        queryBareme.clear();
        db.close();
    }

    //Connexion a la base TERRAIN et recuperation des elements stations
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+terrainDir);
    if (db.open()) {
        QSqlQuery queryTerrain;
        int compteur (0);
        for (int i=0;i<Vnosta.size();i++) {
            QString requete ="SELECT TOP 1 `dat_pas`,`he_sta`,`cot_ech`,`cot_sta`,`recal`,`nett_seuil`,`obs` FROM `Passages(Tp)` WHERE `nosta`="+QString::number(Vnosta[i])+" ORDER BY `dat_pas` DESC;";
            queryTerrain.exec(requete);

            if (queryTerrain.next()) {
                //Date de passage station
                QString dateP = queryTerrain.value(0).toString();
                QDate datePa;
                if (dateP.contains(regDateTime)){
                    datePa.setDate(regDateTime.cap(1).toInt()+2000,regDateTime.cap(2).toInt(),regDateTime.cap(3).toInt());
//                    QTableWidgetItem *item_datePassage = new QTableWidgetItem(datePa.toString("dd/MM/yy"));
                    QTableWidgetItem *item_datePassage = new QTableWidgetItem;
                    item_datePassage->setData (0 ,datePa);
                    ui->StationTable->setItem(compteur,c_datePassage,item_datePassage);
                }
                //Heure station
                QString timePa = queryTerrain.value(1).toString();
                if (timePa.contains(regDateTime)){
                    timePa = regDateTime.cap(4);
                    QTableWidgetItem *item_timePassage = new QTableWidgetItem(timePa);
                    ui->StationTable->setItem(compteur,c_heurePassage,item_timePassage);
                }
                //Cote echelle
                float coteEchelle = queryTerrain.value(2).toFloat();
                QTableWidgetItem *item_coteEchelle = new QTableWidgetItem(QString::number(coteEchelle));
                ui->StationTable->setItem(compteur,c_coteEchelle,item_coteEchelle);
                //Cote station
                float coteStation= queryTerrain.value(3).toFloat()/10;
                QTableWidgetItem *item_coteStation = new QTableWidgetItem(QString::number(coteStation));
                ui->StationTable->setItem(compteur,c_coteStation,item_coteStation);

                //Recalage station
                bool recalage = queryTerrain.value(4).toBool();
                QTableWidgetItem *item_CheckRecalage = new QTableWidgetItem("");
                item_CheckRecalage->setFlags(item_CheckRecalage->flags()|Qt::ItemIsUserCheckable);
                if (recalage)
                     {item_CheckRecalage->setCheckState(Qt::Checked);}
                else
                     {item_CheckRecalage->setCheckState(Qt::Unchecked);}
                ui->StationTable->setItem(compteur,c_recalage,item_CheckRecalage);

                //Nettoyage seuil
                bool netSeuil = queryTerrain.value(5).toBool();
                QTableWidgetItem *item_CheckNetSeuil = new QTableWidgetItem("");
                item_CheckNetSeuil->setFlags(item_CheckNetSeuil->flags()|Qt::ItemIsUserCheckable);
                if (netSeuil)
                     {item_CheckNetSeuil->setCheckState(Qt::Checked);}
                else
                     {item_CheckNetSeuil->setCheckState(Qt::Unchecked);}
                ui->StationTable->setItem(compteur,c_NetSeuil,item_CheckNetSeuil);

                //Commentaire
                QString comment = queryTerrain.value(6).toString();
                QTableWidgetItem *item_comment = new QTableWidgetItem(comment);
                ui->StationTable->setItem(compteur,c_comment,item_comment);
                compteur++;
            }
            else {
                QTableWidgetItem *item_datePassage = new QTableWidgetItem("NULL");
                ui->StationTable->setItem(compteur,c_datePassage,item_datePassage);
                ui->StationTable->removeRow(compteur);
                VcodeHydro.remove(compteur);
            }
        }
        queryTerrain.clear();
        db.close();
    }

    //Connexion a la base LOCALE et recuperation des elements
    nbStation = VcodeHydro.size();
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+localeDir);
    if (db.open()) {
        QSqlQuery queryBDLocale;
        int compteur = 0;
        for (int i=0;i<nbStation;i++) {

            queryBDLocale.exec("SELECT MAX(date) FROM "+VcodeHydro[i]);
            QDate datePas = (ui->StationTable->item(compteur,c_datePassage))->data(0).toDate();

            if (ignoredStations.contains(VcodeHydro[i])) {
                ui->StationTable->removeRow(compteur);
            }
            else if (queryBDLocale.next() && !datePas.isNull() ) {
                QString dateTraitementString = queryBDLocale.value(0).toString();
                QString heureTraitement;
                QDate dateTraitement;
                QDate dateComp;
                if (dateTraitementString.contains(regDateTime)) {
                    heureTraitement = regDateTime.cap(4);
                    dateTraitement.setDate(regDateTime.cap(1).toInt()+2000,regDateTime.cap(2).toInt(),regDateTime.cap(3).toInt());
                    dateComp = dateTraitement.addDays(tolerance);
                }
                QTableWidgetItem *item_dateTraitement = new QTableWidgetItem;
                    item_dateTraitement->setData(0,dateTraitement);
                QTableWidgetItem *item_heureTraitement = new QTableWidgetItem(heureTraitement);
                ui->StationTable->setItem(compteur,c_dateTraitement,item_dateTraitement);
                ui->StationTable->setItem(compteur,c_heureTraitement,item_heureTraitement);

                //Comptage et coloration des stations du bulletin
                if (bulletinStations.contains(VcodeHydro[i])) {
                    QDate debutMoisDate = QDate (currentDate.year(),currentDate.month(),2);
                    if (dateTraitement<debutMoisDate) {
                        for (int i=0;i<(ui->StationTable->columnCount());i++) {
                            ui->StationTable->item(compteur,i)->setBackgroundColor(bulletinColor);
                        }
                        cpteBulletin++;
                    }
                    else if (stationType == processing) {
                        ui->StationTable->removeRow(compteur);
                        compteur--;
                    }
                }
                //Comptage et coloration et trie des stations hors bulletin avec traitement par date fixe
                else if (method == fixedDateMethod) {
                    if (dateTraitement<processingDate) {
                        for (int i=0;i<(ui->StationTable->columnCount());i++) {
                            ui->StationTable->item(compteur,i)->setBackgroundColor(processingColor);
                        }
                        //Comptage et coloration des station dont la date depuis dernier traitement est trop long
                        if (dateTraitement.addDays(timeOutDay)<=currentDate && timeOutDayStatus) {
                            for (int i=0;i<(ui->StationTable->columnCount());i++) {
                                ui->StationTable->item(compteur,i)->setTextColor(colorTimeOutDay);
                                QFont itemFont = ui->StationTable->item(compteur,i)->font();
                                itemFont.setBold(true);
                                ui->StationTable->item(compteur,i)->setFont(itemFont);
                            }
                            cpteTimeOutDay++;
                        }
                        cpteProcessing++;
                    }
                }
                //Comptage et coloration et trie des stations hors bulletin avec traitement apres passage
                else if (datePas>dateComp) {
                    for (int i=0;i<(ui->StationTable->columnCount());i++) {
                        ui->StationTable->item(compteur,i)->setBackgroundColor(processingColor);
                    }
                    //Comptage et coloration des station dont la date depuis dernier traitement est trop long
                    if (dateTraitement.addDays(timeOutDay)<=currentDate && timeOutDayStatus) {
                        for (int i=0;i<(ui->StationTable->columnCount());i++) {
                            ui->StationTable->item(compteur,i)->setTextColor(colorTimeOutDay);
                            QFont itemFont = ui->StationTable->item(compteur,i)->font();
                            itemFont.setBold(true);
                            ui->StationTable->item(compteur,i)->setFont(itemFont);
                        }
                        cpteTimeOutDay++;
                    }
                    cpteProcessing++;
                }
                //Comptage et coloration des station dont la date depuis dernier traitement est trop long
                else if (dateTraitement.addDays(timeOutDay)<=currentDate && timeOutDayStatus) {
                    for (int i=0;i<(ui->StationTable->columnCount());i++) {
                        ui->StationTable->item(compteur,i)->setTextColor(colorTimeOutDay);
                        QFont itemFont = ui->StationTable->item(compteur,i)->font();
                        itemFont.setBold(true);
                        ui->StationTable->item(compteur,i)->setFont(itemFont);
                    }
                    cpteTimeOutDay++;
                    cpteProcessing++;
                }
                //Suppression des station non à traiter
                else if (stationType == processing) {
                    ui->StationTable->removeRow(compteur);
                    compteur--;
                }
                compteur++;
            }
            else {
                ui->StationTable->removeRow(compteur);
            }
        }
        queryBDLocale.clear();
        db.close();
    }
    statusMessage->setText("A traiter : "+QString::number(cpteBulletin)+" Bulletins et "+QString::number(cpteProcessing)+" classiques dont "+QString::number(cpteTimeOutDay)+" urgents");

    ui->StationTable->sortItems (settings.value("Sort/Index",1).toInt(),static_cast<Qt::SortOrder>(settings.value("Sort/Order",0).toInt()));
}

//Lance le rafraichissiement de la vue en cours ou de la derniere vue utilisée
void galihydro::refresh ()
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    if (settings.value("ShowOption/Last").toString()== "processing")
        showStations (processing);
    else if (settings.value("ShowOption/Last").toString()== "bulletin")
        showStations (bulletin);
    else if (settings.value("ShowOption/Last").toString()== "myAll")
        showStations (myAll);
    else if (settings.value("ShowOption/Last").toString()== "myProcessing")
        showStations (myProcessing);
    else if (settings.value("ShowOption/Last").toString()== "myBulletin")
        showStations (myBulletin);
    else if (settings.value("ShowOption/Last").toString()== "group") {
        loadGroup (settings.value("ShowOption/LastGroup").toString());
    }
    else
        showStations (all);
}

//Creation d'une ligne suivant les parametres de la stations (BDD Bareme)
void galihydro::createRow (int row,QString codeHydro,QString nomStation,QString courEau)
{
    QTableWidgetItem *item_codeHydro = new QTableWidgetItem(codeHydro);
    QTableWidgetItem *item_nomStation = new QTableWidgetItem(nomStation.toUpper().replace("_"," "));
    QTableWidgetItem *item_courEau = new QTableWidgetItem(courEau.toUpper().replace("_"," "));

    ui->StationTable->insertRow(row);
    ui->StationTable->setItem(row,c_nomStation,item_nomStation);
    ui->StationTable->setItem(row,c_codeHydro,item_codeHydro);
    ui->StationTable->setItem(row,c_courDo,item_courEau);

//Solution de debugage !
    QTableWidgetItem *dateTraitement = new QTableWidgetItem("NULL");
    QTableWidgetItem *heureTraitement = new QTableWidgetItem("NULL");
    QTableWidgetItem *datePassage = new QTableWidgetItem("NULL");
    ui->StationTable->setItem(row,c_dateTraitement,dateTraitement);
    ui->StationTable->setItem(row,c_heureTraitement,heureTraitement);
    ui->StationTable->setItem(row,c_datePassage,datePassage);

    QTableWidgetItem *item_coteEchelle = new QTableWidgetItem("NULL");
    QTableWidgetItem *item_coteStation = new QTableWidgetItem("NULL");
    QTableWidgetItem *item_recalage = new QTableWidgetItem("NULL");
    QTableWidgetItem *item_heurePassage = new QTableWidgetItem("");
    QTableWidgetItem *item_NetSeuil = new QTableWidgetItem("NULL");
    QTableWidgetItem *item_comment = new QTableWidgetItem("");
    ui->StationTable->setItem(row,c_coteEchelle,item_coteEchelle);
    ui->StationTable->setItem(row,c_coteStation,item_coteStation);
    ui->StationTable->setItem(row,c_recalage,item_recalage);
    ui->StationTable->setItem(row,c_heurePassage,item_heurePassage);
    ui->StationTable->setItem(row,c_NetSeuil,item_NetSeuil);
    ui->StationTable->setItem(row,c_comment,item_comment);
}

//Suppression des parametres utilisateur (fichier ini)
void galihydro::delParam()
{
    QFile file("GaliHydro.ini");
    bool valid;
    if (file.exists()) {
        valid = file.remove();

        if (!valid) {
            QMessageBox::critical(this, "Supression des parametres", "Echec lors de la suppression des parametres");
        }
        else {
            QMessageBox::information(this, "Supression des parametres", "Suppression des parametres réussie");
        }
    }
    else {
        QMessageBox::information(this, "Supression des parametres", "Aucun parametres a supprimer");
    }
}

//SLOT : Affiche ou camoufle une colonne
void galihydro::hideColumn (bool checked, int column)
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    if (!checked){
        ui->StationTable->hideColumn(column);
        settings.setValue("Column/"+QString::number(column),false);
    }
    else {
        ui->StationTable->showColumn(column);
        settings.setValue("Column/"+QString::number(column),true);
    }
}

//SLOT : Trie la colonne d'index 'index' et affecte un caractere materialisant le sens montant/descendant
void galihydro::sortColumn (int index)
{
    QString ascendingChar = QString("\u25B3")+" ";
    QString descendingChar = QString("\u25BD")+" ";

    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    if (settings.value("Sort/Index").toInt()== index){
        if (settings.value("Sort/Order").toInt() == Qt::AscendingOrder){
            ui->StationTable->sortItems(index, Qt::DescendingOrder);
            settings.setValue("Sort/Order",Qt::DescendingOrder);

            QString textheader = ui->StationTable->horizontalHeaderItem(index)->text();
            textheader.replace(ascendingChar,"");
            ui->StationTable->horizontalHeaderItem(index)->setText(descendingChar+textheader);
        }
        else if (settings.value("Sort/Order").toInt() == Qt::DescendingOrder){
            ui->StationTable->sortItems(index, Qt::AscendingOrder);
            settings.setValue("Sort/Order",Qt::AscendingOrder);

            QString textheader = ui->StationTable->horizontalHeaderItem(index)->text();
            textheader.replace(descendingChar,"");
            ui->StationTable->horizontalHeaderItem(index)->setText(ascendingChar+textheader);
        }
    }
    else {
        QString textOtherHeader = ui->StationTable->horizontalHeaderItem(settings.value("Sort/Index").toInt())->text();
        textOtherHeader.replace(ascendingChar,"");
        textOtherHeader.replace(descendingChar,"");
        ui->StationTable->horizontalHeaderItem(settings.value("Sort/Index").toInt())->setText(textOtherHeader);

        ui->StationTable->sortItems(index, Qt::AscendingOrder);
        settings.setValue("Sort/Order",Qt::AscendingOrder);

        QString textheader = ui->StationTable->horizontalHeaderItem(index)->text();
        ui->StationTable->horizontalHeaderItem(index)->setText(ascendingChar+textheader);
    }
    settings.setValue("Sort/Index",index);
}


//Attribue la tolerance de jour entre la date de traitement et la date de passage
void galihydro::setTolerance (int newTolerance)
{
    tolerance = newTolerance;
}

//Attribue le nombre de jour entre la date de traitement et le nombre maxi avant traitement
void galihydro::setTimeOutDay (int newTimeOutDay)
{
    timeOutDay = newTimeOutDay;
}

//Attribue le status d'affichage pour depassement du delais entre la date de traitement et le nombre maxi avant traitement
void galihydro::setTimeOutDayStatus(bool status)
{
    timeOutDayStatus = status;
}

void galihydro::setMethod (processingMethod newMethod)
{
    method = newMethod;
    qDebug () << "Changement methode : (setMethod)";
}

bool galihydro::getConfigMode ()
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    //Initialisation de la variable admin
    bool adminMode = false;

    //Récuperation du mdp administateur dans le .ini
    QString userPwd = settings.value("Admin").toString();

    //Recuperation du nom d'utilisateur dans les parametres environnement
    const QProcessEnvironment pcEnvirenment = QProcessEnvironment::systemEnvironment();

    QString userName = "";
    if (!pcEnvirenment.isEmpty()){
        userName = pcEnvirenment.value("USERNAME");
    }

    //Création du mot de passe comparatif : Hachage + salage
    QByteArray hash = QCryptographicHash::hash(QByteArray(QCryptographicHash::hash(QString(SEL_DEBUT).toUtf8(), QCryptographicHash::Md5) +
                                                          QCryptographicHash::hash(userName.toUtf8(), QCryptographicHash::Sha1) +
                                                          QCryptographicHash::hash(QString(SEL_FIN).toUtf8(), QCryptographicHash::Md5)),QCryptographicHash::Sha1);
    QString pwd = hash.toHex();
    pwd.truncate (25);

    //Verification du mdp
    if (pwd == userPwd){
        adminMode = true;
    }
    return adminMode;
}

//Creation du menu d'acces aux paquets
void galihydro::getGroupName ()
{
    //Variable globale : fichier ini
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    //Recuperation des stations
    QString stations;
    QStringList groupsNames;

    //Recuperation du nombre de groupe et du groupe courrant
    settings.beginGroup("Group");
    groupsNames = settings.childKeys();
    settings.endGroup();

    //Boucle de creation/affichage de l'ensemble des groupes
    for (int g=0;g<groupsNames.count();g++) {
        stations.clear();

        stations = settings.value("Group/"+groupsNames.at(g)).toString();
        ui->menuMes_paquets->addAction(groupsNames.at(g));
    }

    //Attribution des caracteristiques et connection !
    QList<QAction *> QActionGroupList = ui->menuMes_paquets->findChildren<QAction *>();
    int groupNumber = QActionGroupList.size();
    qDebug () << "taille : "<<QString::number(groupNumber);
    if (groupNumber>=1){
        QActionGroupList.removeAt(0);
        groupNumber--;
    }
    for (int h=0;h<groupNumber;h++)
    {
        qDebug () << "connect this action : " << QActionGroupList[h] << " : "<< QActionGroupList[h]->text();
        QObject::connect(QActionGroupList[h], SIGNAL(triggered ()), this, SLOT(loadGroup ()));
    }
}


//SLOT :Renvoie la liste des codes hydros du paquet suivant le QAction emmeteur du signal. Si erreur renvoie une liste vide
void galihydro::loadGroup (QString groupName)
{
    groupStations.clear();
    QString stations;
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);
    QRegExp regExCodeHydro("[A-Z][0-9]{7}");


    //Recuperation du QAction emmeteur
    if (!groupName.isNull() && !groupName.isEmpty()){
        stations = settings.value("Group/"+groupName).toString();
    }
    else {
        QAction* action = qobject_cast<QAction* >(sender());
        qDebug () <<"Action groupe recut de "<< action->text();
        //Recuperation des données du paquet par son nom
        stations = settings.value("Group/"+action->text()).toString();
        settings.setValue("ShowOption/LastGroup",action->text());
    }
        //On cree le tableau de code hydro
        while (stations.contains(regExCodeHydro)) {
            groupStations.append(regExCodeHydro.cap(0));
            stations.remove(0,9);
        }
    showStations(group);
}

galihydro::~galihydro()
{
    delete ui;
}
