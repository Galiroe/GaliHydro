#include "groupedit.h"
#include "ui_groupedit.h"

#include<QDebug>

//Suite suppresion nom du groupe apres le "=" et ainsi les regex associé => A faire egalement dans galihydro.cpp
//Empecher le tri autmoatique des paquet par ordre alhpabetique
//Ne pas afficher les stations dans la liste si aucun groupe créé
//Faire avertissement quand cancel et pour fermer
//Attention nom de paquet => regex ne tolere pas les caractere speciaux
//Comprendre pourquoi lors de suppression d'un paquet il reste toujours le dernier groupe vide malgres remove ! idem, reste groupe numbers pour un remove total !
//Gerer les boutons monter et descendre
//Gerer l'enregistrement des paquets multiples : plusieurs paquets modifier, deselection d'un paquet sans etre enregistré (dans signal change row avec variable enregistrer type bool)...
//Amelioration possible : Charger les groupe dans tableau pour eviter les acces a la base pour chaque changement !

groupedit::groupedit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::groupedit)
{
    ui->setupUi(this);

    //Variable globale : fichier ini
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    //Recuperation des stations bulletin/traitement
    QRegExp regExCodeHydro("[A-Z][0-9]{7}");

    QString stations;
    QString groupName;
    QVector<QString> groupStations;

    //Recuperation du nombre de groupe et de leur nom
    QStringList groupsNames;
    settings.beginGroup("Group");
    groupsNames = settings.childKeys();
    qDebug () << "childKeys" <<settings.childKeys();
    settings.endGroup();

    //Boucle de creation/affichage de l'ensemble des groupes
    for (int g=0;g<groupsNames.count();g++) {
        groupName =groupsNames.at(g);
        stations = settings.value("Group/"+groupName).toString();
            while (stations.contains(regExCodeHydro)) {
                groupStations.append(regExCodeHydro.cap(0));
                stations.remove(0,9);
        }
            QListWidgetItem *newGroup = new QListWidgetItem (groupName);
            ui->groupList->addItem(newGroup);
            ui->groupList->setCurrentItem (newGroup);
    }


    QObject::connect(ui->addGroupButton,SIGNAL(clicked()),this,SLOT(addGroup()));
    QObject::connect(ui->dellGroupButton,SIGNAL(clicked()),this,SLOT(dellGroup()));
    QObject::connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(saveGroup()));
    QObject::connect(ui->cancelButton,SIGNAL(clicked()),this, SLOT(cancelGroup()));
    QObject::connect(ui->closeButton,SIGNAL(clicked()),this, SLOT(closeGroup()));
    QObject::connect(ui->groupList,SIGNAL(currentRowChanged(int)),this, SLOT(loadGroup(int)));

    setAttribute(Qt::WA_DeleteOnClose);
}

void groupedit::loadGroup (int currentRow)
{
    //Variable globale : fichier ini
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    //Adresses des differentes BDD
    QString baremeDir = settings.value("BDD/bareme","C://").toString();
    QString localeDir = settings.value("BDD/locale","C://").toString();
    QString terrainDir = settings.value("BDD/terrain","C://").toString();

    //Recuperation des stations bulletin/traitement
    QRegExp regExCodeHydro("[A-Z][0-9]{7}");

    QVector<QString> VcodeHydro (0);
    QVector<int> Vnosta (0);
    QString stations;
    QVector<QString> groupStations;
    QVector<QString> ignoredStations;

    stations = settings.value("Group/"+ui->groupList->item(currentRow)->text()).toString();
        while (stations.contains(regExCodeHydro)) {
            groupStations.append(regExCodeHydro.cap(0));
            stations.remove(0,9);
        }

    stations = settings.value("Ignored/CodeHydro").toString();
    while (stations.contains(regExCodeHydro)) {
        ignoredStations.append(regExCodeHydro.cap(0));
        stations.remove(0,9);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QSqlQuery querySelect;

    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+baremeDir);
    if(db.open()) {
        querySelect.exec("SELECT nosta, codehydro, nom FROM station ORDER BY codehydro;");
        int i = 0;
        while (querySelect.next()) {
            int nosta = querySelect.value(0).toInt();
            QString codeHydro = querySelect.value(1).toString();
            QString nomStation = querySelect.value(2).toString().replace("_"," ").toUpper();
            Vnosta.append(nosta);
            VcodeHydro.append(codeHydro);

            QTableWidgetItem *itemCodeHydro;
            itemCodeHydro = new QTableWidgetItem(codeHydro);
            QTableWidgetItem *itemNomStation;
            itemNomStation = new QTableWidgetItem(nomStation);

            ui->selectTable->setRowCount(i+1);
            ui->selectTable->setItem(i,0,itemNomStation);
            ui->selectTable->setItem(i,1,itemCodeHydro);
            QTableWidgetItem *itemCheckGroup = new QTableWidgetItem("");
            itemCheckGroup->setFlags(itemCheckGroup->flags()|Qt::ItemIsUserCheckable);
            if (groupStations.contains(codeHydro))
            {itemCheckGroup->setCheckState(Qt::Checked);}
            else
            {itemCheckGroup->setCheckState(Qt::Unchecked);}
            ui->selectTable->setItem(i,2,itemCheckGroup);

            QTableWidgetItem *itemCheckIgnored = new QTableWidgetItem("");
            itemCheckIgnored->setFlags(itemCheckIgnored->flags()|Qt::ItemIsUserCheckable);
            if (ignoredStations.contains(codeHydro))
            {itemCheckIgnored->setCheckState(Qt::Checked);}
            else
            {itemCheckIgnored->setCheckState(Qt::Unchecked);}
            ui->selectTable->setItem(i,3,itemCheckIgnored);

            i++;
        }
    }
    querySelect.clear ();
    db.close();

    //Connexion a la base LOCALE pour elimination des stations inutiles
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+localeDir);
    if (db.open()) {
        int compteur = 0;
        for (int i=0;i<VcodeHydro.size();i++) {
            querySelect.exec("SELECT * FROM "+VcodeHydro[i]);
            if (!querySelect.next()) {
                ui->selectTable->removeRow(compteur);
                Vnosta.remove(compteur);
                compteur--;
            }
            compteur++;
        }
        querySelect.clear();
        db.close();
    }

    //Connexion a la base TERRAIN  pour elimination des stations inutiles
    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ="+terrainDir);
    if (db.open()) {
        int compteur = 0;
        for (int i=0;i<Vnosta.size();i++) {
            querySelect.exec("SELECT * FROM `Passages(Tp)` WHERE `nosta`="+QString::number(Vnosta[i])+";");
            if (!querySelect.next()) {
                ui->selectTable->removeRow(compteur);
                VcodeHydro.remove(compteur);
                compteur--;
            }
            compteur++;
        }
        querySelect.clear();
        db.close();
    }
}

void groupedit::addGroup ()
{
    bool ok = false;
    QString addGroup = QInputDialog::getText(this,"Nom du paquet","Veuillez saisir le nom du nouveau paquet :", QLineEdit::Normal, QString(), &ok);
    if (ok && !addGroup.isEmpty())
    {
        QListWidgetItem *newGroup = new QListWidgetItem (addGroup);
        ui->groupList->addItem(newGroup);
        ui->groupList->setCurrentItem (newGroup);
    }
}

void groupedit::dellGroup ()
{
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    int currentRow = ui->groupList->currentRow ();
    QString itemName = ui->groupList->takeItem(currentRow)->text();
    settings.remove("Group/"+itemName);
}

void groupedit::saveGroup ()
{
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    QString stationCheckedP ("");
    QString stationCheckedI ("");
    for (int i=0;i<ui->selectTable->rowCount();i++) {
        if(ui->selectTable->item(i,2)->checkState()) {
            stationCheckedP = stationCheckedP + ui->selectTable->item(i,1)->text()+";";
        }
        if(ui->selectTable->item(i,3)->checkState()) {
            stationCheckedI = stationCheckedI + ui->selectTable->item(i,1)->text()+";";
        }

        int currentRow = ui->groupList->currentRow ();
        QString groupName = ui->groupList->item(currentRow)->text();

        settings.setValue("Group/"+groupName,stationCheckedP);
        settings.setValue("Ignored/CodeHydro",stationCheckedI);
    }

//        emit refreshTable();
}

void groupedit::cancelGroup ()
{
    close();
}

void groupedit::closeGroup ()
{
    close();
}

groupedit::~groupedit()
{
    delete ui;
}
