#include "options.h"
#include "ui_options.h"
#include "QShortcut"

#include <QColorDialog>

#include <QDebug>
#include <QString>
#include <QPalette>
#include <QSettings>

#define toleranceDefault 15
#define timeOutDayDefault 45
#define fixedDateDefault 1

//Sels de hachage
#define SEL_DEBUT "NOSPY"
#define SEL_FIN "NOFLIC"

//Mise en garde de perte du mdp dans deladminslot ?

// Construire et detruire l'onglet admin. Le camouflage et desactivation engendre des bug d'affichage sur les autres onglets et l'entete de l'onglet reste present


options::options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::options)
{
    ui->setupUi(this);

    //Recuperation de l'onglet adminTab puis suppression
    adminTab = ui->generalTab->widget(2);
    ui->generalTab->removeTab(2);

    //Raccourcis clavier pour creation/suppression dde l'onglet adminTab
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_G), this, SLOT(adminTabActivation()));

    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    //Récuperation des parametres dans le .ini
    tolerance = settings.value ("Tolerance",toleranceDefault).toInt();
    timeOutDay = settings.value ("TimeOutDay/Value",timeOutDayDefault).toInt();
    fixedDate = settings.value ("FixedDate",1).toInt();
    userPwd = settings.value("Admin").toString();

    //Recupere la methode de traitement et active/desactive les zones correspondantes
    if (settings.value("Method").toString()== "fixedDate") {
        ui->toleranceInt->setEnabled(false);
        ui->fixedDate->setChecked(true);
        method = fixedDateMethod;
    }
    else {
        ui->fixedDateInt->setEnabled(false);
        ui->afterPassage->setChecked(true);
        method = afterPassageMethod;
    }

    timeOutDayStatus = settings.value ("TimeOutDay/Enabled",true).toBool();
    if (timeOutDayStatus) {
        ui->dayNumberBox->setChecked(true);}
    else {
        ui->dayNumberBox->setChecked(false);}

    ui->toleranceInt->setValue(tolerance);
    ui->timeOutDayInt->setValue(timeOutDay);
    ui->fixedDateInt->setValue(fixedDate);

    if (!userPwd.isNull() && !userPwd.isEmpty())
        ui->mdpLigneEdit->setText(userPwd);

    colorProcessing.setRgb (settings.value("Traitement/Red",219).toInt(),settings.value("Traitement/Green",239).toInt(),settings.value("Traitement/Blue",255).toInt());
    colorBuletin.setRgb (settings.value("Bulletin/Red",250).toInt(),settings.value("Bulletin/Green",150).toInt(),settings.value("Bulletin/Blue",45).toInt());
    colorTimeOutDay.setRgb (settings.value("TimeOutDay/Red",200).toInt(),settings.value("TimeOutDay/Green",0).toInt(),settings.value("TimeOutDay/Blue",0).toInt());

    QBrush backgroundBuletin (colorBuletin);
    QBrush backgroundProcessing (colorProcessing);
    QBrush backgroundTimeOutDay (colorTimeOutDay);

    paletteBuletin.setBrush(QPalette::Window,backgroundBuletin);
    ui->buletinColorView->setPalette(paletteBuletin);
    paletteProcessing.setBrush(QPalette::Window,backgroundProcessing);
    ui->processingColorView->setPalette(paletteProcessing);
    paletteTimeOutDay.setBrush(QPalette::Window,backgroundTimeOutDay);
    ui->timeOutDayColorView->setPalette(paletteTimeOutDay);

    ui->processingColorView->setAutoFillBackground(true);
    ui->buletinColorView->setAutoFillBackground(true);
    ui->timeOutDayColorView->setAutoFillBackground(true);


    QObject::connect(ui->processingColor, SIGNAL(clicked()), this, SLOT(processingDialogColor()));
    QObject::connect(ui->bulletinColor, SIGNAL(clicked()), this, SLOT(bulletinDialogColor()));
    QObject::connect(ui->timeOutDayColor, SIGNAL(clicked()), this, SLOT(timeOutDayDialogColor()));
    QObject::connect(ui->defautProcessing, SIGNAL(clicked()), this, SLOT(defautProcessingColor()));
    QObject::connect(ui->defautBuletin, SIGNAL(clicked()), this, SLOT(defautBuletinColor()));
    QObject::connect(ui->defautTimeOutDay, SIGNAL(clicked()), this, SLOT(defautTimeOutDayColor()));
    QObject::connect(this, SIGNAL(accepted()), this, SLOT(acceptParam()));
    QObject::connect(ui->afterPassage, SIGNAL(toggled(bool)),this, SLOT (methodeAfterPassage (bool)));
    QObject::connect(ui->fixedDate, SIGNAL(toggled(bool)),this, SLOT (methodeFixedDate (bool)));
    QObject::connect(ui->showMdpCheckBox, SIGNAL(toggled(bool)),this, SLOT (echoChange (bool)));
    QObject::connect(ui->checkAdminButton, SIGNAL(clicked()),this, SLOT (checkAdminSlot()));
    QObject::connect(ui->delAdminButton, SIGNAL(clicked()),this, SLOT (delAdminSlot()));
}

//Fonctionalités graphiques
void options::processingDialogColor()
{
    QColor colorSelected = QColorDialog::getColor(colorProcessing, this);
    if (colorSelected.isValid()) {
        colorProcessing = colorSelected;
        QBrush backgroundBrush (colorProcessing);
        paletteProcessing.setBrush(QPalette::Window,backgroundBrush);
        ui->processingColorView->setPalette(paletteProcessing);
    }
}

void options::bulletinDialogColor()
{
    QColor colorSelected = QColorDialog::getColor(colorBuletin, this);
    if (colorSelected.isValid()) {
        colorBuletin = colorSelected;
        QBrush backgroundBrush (colorBuletin);
        paletteBuletin.setBrush(QPalette::Window,backgroundBrush);
        ui->buletinColorView->setPalette(paletteBuletin);
    }
}

void options::timeOutDayDialogColor()
{
    QColor colorSelected = QColorDialog::getColor(colorTimeOutDay, this);
    if (colorSelected.isValid()) {
        colorTimeOutDay = colorSelected;
        QBrush backgroundBrush (colorTimeOutDay);
        paletteTimeOutDay.setBrush(QPalette::Window,backgroundBrush);
        ui->timeOutDayColorView->setPalette(paletteTimeOutDay);
    }
}

void options::defautProcessingColor()
{
    colorProcessing = QColor(219,239,255);
    QBrush backgroundBrush (colorProcessing);
    paletteProcessing.setBrush(QPalette::Window,backgroundBrush);
    ui->processingColorView->setPalette(paletteProcessing);
}

void options::defautBuletinColor()
{
    colorBuletin = QColor(250,150,45);
    QBrush backgroundBrush (colorBuletin);
    paletteBuletin.setBrush(QPalette::Window,backgroundBrush);
    ui->buletinColorView->setPalette(paletteBuletin);
}

void options::defautTimeOutDayColor()
{
    colorTimeOutDay = QColor(200,0,0);
    QBrush backgroundBrush (colorTimeOutDay);
    paletteTimeOutDay.setBrush(QPalette::Window,backgroundBrush);
    ui->timeOutDayColorView->setPalette(paletteTimeOutDay);
}

//Fonctionalités générales
void options::methodeAfterPassage (bool choice)
{
    if (!choice)
        ui->toleranceInt->setEnabled(false);
    else
        ui->toleranceInt->setEnabled(true);
}

void options::methodeFixedDate (bool choice)
{
    if (!choice)
        ui->fixedDateInt->setEnabled(false);
    else
        ui->fixedDateInt->setEnabled(true);
}

//Fonctionalités administrateur
void options::echoChange (bool echoMode)
{
    if (echoMode) {
        ui->mdpLigneEdit->setEchoMode(QLineEdit::Normal);
    }
    else {
        ui->mdpLigneEdit->setEchoMode(QLineEdit::Password);
    }

}

void options::checkAdminSlot()
{
    ui->CheckLabel->clear();

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

    //Recuperation du mot de passe entré par l'utilisateur
    QString userPwdChoice = ui->mdpLigneEdit->text();


    QPalette CheckLabelPalette;
    //Comparaison du mot de passe et de l'entrée utilisateur
    if (userPwdChoice == pwd) {
        CheckLabelPalette.setColor(QPalette::WindowText, Qt::darkGreen);
        ui->CheckLabel->setPalette(CheckLabelPalette);
        ui->CheckLabel->setText("Mot de passe correct");
    }
    else {
        CheckLabelPalette.setColor(QPalette::WindowText, QColor(200,0,0));
        ui->CheckLabel->setPalette(CheckLabelPalette);
        ui->CheckLabel->setText("Mot de passe incorrect...");
    }

//image: url(myindicator.png);
//lineEdit->setStyleSheet("background: lightgreen");


}

void options::delAdminSlot()
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    ui->mdpLigneEdit->clear();
    settings.remove("Admin");
}

//Sauvegarde des parametres
void options::acceptParam()
{
    //Variable globale : fichier ini
    QSettings settings ("GaliHydro.ini", QSettings::IniFormat);

    if (colorProcessing.isValid()) {
        settings.setValue("Traitement/Red",colorProcessing.red ());
        settings.setValue("Traitement/Green",colorProcessing.green ());
        settings.setValue("Traitement/Blue",colorProcessing.blue ());
    }

    if (colorBuletin.isValid()) {
        settings.setValue("Bulletin/Red",colorBuletin.red ());
        settings.setValue("Bulletin/Green",colorBuletin.green ());
        settings.setValue("Bulletin/Blue",colorBuletin.blue ());
    }

    if (colorTimeOutDay.isValid()) {
        settings.setValue("TimeOutDay/Red",colorTimeOutDay.red ());
        settings.setValue("TimeOutDay/Green",colorTimeOutDay.green ());
        settings.setValue("TimeOutDay/Blue",colorTimeOutDay.blue ());
    }

    int toleranceChoice = ui->toleranceInt->value();
    int fixedDateChoice = ui->fixedDateInt->value();
    if (toleranceChoice != tolerance && ui->afterPassage->isChecked()) {
        settings.setValue("Tolerance",toleranceChoice);
        emit changeTolerance (toleranceChoice);
    }
    else if (fixedDateChoice != fixedDate && ui->fixedDate->isChecked()) {
        settings.setValue("FixedDate",fixedDateChoice);
        emit changeFixedDate (fixedDateChoice);
    }


    if (ui->afterPassage->isChecked() && method != afterPassageMethod) {
        method = afterPassageMethod;
        settings.setValue("Method", "afterPassage");
        emit methodChanged (method);
    }
    else if (ui->fixedDate->isChecked() &&  method != fixedDateMethod) {
        method = fixedDateMethod;
        settings.setValue("Method", "fixedDate");
        emit methodChanged (method);
    }

    int timeOutDayChoice = ui->timeOutDayInt->value();
    if (timeOutDayChoice != timeOutDayDefault && timeOutDayChoice != timeOutDay) {
        settings.setValue("TimeOutDay/Value",timeOutDayChoice);
        emit changeTimeOutDay (timeOutDayChoice);
    }

    bool timeOutDayStatusChoice = ui->dayNumberBox->isChecked();
    if (timeOutDayStatusChoice && timeOutDayStatusChoice != timeOutDayStatus) {
        settings.setValue("TimeOutDay/Enabled",true);
        emit ChangeTimeOutDayStatus (true);
    }
    else if (!timeOutDayStatusChoice  && timeOutDayStatusChoice != timeOutDayStatus){
        settings.setValue("TimeOutDay/Enabled",false);
        emit ChangeTimeOutDayStatus (false);
    }

    QString userPwdChoice = ui->mdpLigneEdit->text();
    if (!userPwdChoice.isEmpty() && !userPwdChoice.isNull() && userPwdChoice != userPwd) {
        settings.setValue("Admin",userPwdChoice);
    }

    emit refreshTable ();
}

//Insert ou supprime l'onglet adminTab
void options::adminTabActivation()
{
    if (ui->generalTab->count() == 2)
        ui->generalTab->insertTab(2, adminTab, "Gestionnaire");
    else
        ui->generalTab->removeTab(2);
}

//Destructeur
options::~options()
{
    emit refreshTable();
    delete ui;
}
