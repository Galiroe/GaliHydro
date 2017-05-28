#include "groupedit.h"
#include "ui_groupedit.h"

#include<QDebug>

//Ne pas afficher les stations dans la liste si aucun groupe créé
//Faire avertissement quand cancel et pour fermer
//Attention nom de paquet => regex ne tolere pas les caractere speciaux
//Comprendre pourquoi lors de suppression d'un paquet il reste toujours le dernier groupe vide malgres remove ! idem, reste groupe numbers pour un remove total !
//Gerer les boutons monter et descendre
//Gerer l'enregistrement des paquets multiples : plusieurs paquets modifier, deselection d'un paquet sans etre enregistré (dans signal change row avec variable enregistrer type bool)...
//Pour l'affichage : verifier que le nombre de groupe dans ini soit correct (exemple groupNumbers=2 et seulement 1 groupe reellement trouvé ou inversement)
//Amelioration possible : Charger les groupe dans tableau pour eviter les acces a la base pour chaque changement !

groupedit::groupedit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::groupedit)
{
    ui->setupUi(this);

    currentGroup = 0;
    groupCount = 0;
    //Variable globale : fichier ini
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    QString baremeDir = settings.value("BDD/bareme","C://").toString();
    QString localeDir = settings.value("BDD/locale","C://").toString();
    QString terrainDir = settings.value("BDD/terrain","C://").toString();

    //Recuperation des stations bulletin/traitement
    QRegExp regExCodeHydro("[A-Z][0-9]{7}");
    QRegExp regExGroup("^([a-zA-Z0-9]+)\\[(([A-Z][0-9]{7};)+)\\]$");
//    QRegExp regExGroup("((.+)\\[(([A-Z][0-9]{7};)+)\\]");
//        regExGroup.setMinimal(false);  


    QString stations;
    QString group;
    QString groupName;
    QVector<QString> groupStations;

    //Recuperation du nombre de groupe et du groupe courrant
    QRegExp regExGroupNumbers("^([0-9]+);([0-9]+)$");
    QString groupNumbers = settings.value("Group/numbers").toString();

    if (groupNumbers.contains(regExGroupNumbers))
    {
        groupCount = regExGroupNumbers.cap(1).toInt();
        currentGroup = regExGroupNumbers.cap(2).toInt();
    }
    else
        qDebug ("echec regex group numbers");

    //Boucle de creation/affichage de l'ensemble des groupes
    for (int g=0;g<groupCount;g++)
    {
        stations.clear();
        group.clear();
        groupName.clear();
        groupStations.clear();

        group = settings.value("Group/G"+QString::number(g)).toString();
        if (group.contains(regExGroup))
        {
            groupName = regExGroup.cap(1);
            qDebug ()<<groupName;
            stations = regExGroup.cap(2);
            qDebug ()<<stations;
            while (stations.contains(regExCodeHydro)) {
                groupStations.append(regExCodeHydro.cap(0));
                stations.remove(0,9);
            }
            QListWidgetItem *newGroup = new QListWidgetItem (groupName);
            ui->groupList->addItem(newGroup);
            ui->groupList->setCurrentItem (newGroup);
        }
        else
            qDebug ("echec regex group");
    }

    //Conservé l'affichage du groupe courant ???
    loadGroup(currentGroup);
    ui->groupList->setCurrentRow(currentGroup);


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

    QString baremeDir = settings.value("BDD/bareme","C://").toString();
    QString localeDir = settings.value("BDD/locale","C://").toString();
    QString terrainDir = settings.value("BDD/terrain","C://").toString();

    //Recuperation des stations bulletin/traitement
    QRegExp regExCodeHydro("[A-Z][0-9]{7}");
    QRegExp regExGroup("^([a-zA-Z0-9]+)\\[(([A-Z][0-9]{7};)+)\\]$");

    QVector<QString> VcodeHydro (0);
    QVector<int> Vnosta (0);
    QString stations;
    QString group;
    QString groupName;
    QVector<QString> groupStations;
    QVector<QString> ignoredStations;

    group = settings.value("Group/G"+QString::number(currentRow)).toString();
    if (group.contains(regExGroup))
    {
        groupName = regExGroup.cap(1);
        qDebug ()<<groupName;
        stations = regExGroup.cap(2);
        qDebug ()<<stations;
        while (stations.contains(regExCodeHydro)) {
            groupStations.append(regExCodeHydro.cap(0));
            stations.remove(0,9);
        }
    }
    else
        qDebug ("echec regex group");

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
    ui->groupList->takeItem(currentRow);
    settings.remove("Group/G"+QString::number(currentRow));

    //Cas ou le dernier groupe est supprimé
    if (groupCount==1){
        settings.remove("Group");
    }
    else {
        for (int i=currentRow;i<groupCount;i++){
            QString group = settings.value("Group/G"+QString::number(i+1)).toString();
            settings.setValue("Group/G"+QString::number(i),group);
            settings.remove("Group/G"+QString::number(i+1));
        }
        //Mise a jour du nombre de paquet
        groupCount--;
        //Si paquet courrant est supprimé, attribution du paquet precedent si ce n'est pas deja le premier sinon on le conserve
        if (currentRow==currentGroup && currentGroup!=0){
            currentGroup--;
        }
    }
    settings.setValue("Group/numbers",QString::number(groupCount)+";"+QString::number(currentGroup));
}

void groupedit::saveGroup ()
{
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    groupCount = ui->groupList->count();
    settings.setValue("Group/numbers",QString::number(groupCount)+";"+QString::number(currentGroup));

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
        QString group ("");
        group = groupName+"["+stationCheckedP+"]";

        settings.setValue("Group/G"+QString::number(currentRow),group);
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
