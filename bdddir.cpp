#include "bdddir.h"
#include "ui_bdddir.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

BddDir::BddDir(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BddDir)
{
    ui->setupUi(this);

    QSettings settings("GaliHydro.ini", QSettings::IniFormat);

    QString baremeDir = settings.value("BDD/bareme","").toString();
        ui->dirBareme->setText(baremeDir);
    QString localeDir = settings.value("BDD/locale","").toString();
        ui->dirLocale->setText(localeDir);
    QString terrainDir = settings.value("BDD/terrain","").toString();
        ui->dirTerrain->setText(terrainDir);

    QObject::connect(ui->setBdBarem, SIGNAL(clicked()), this, SLOT(openBdBarem()));
    QObject::connect(ui->setBdLocale, SIGNAL(clicked()), this, SLOT(openBdLocale()));
    QObject::connect(ui->setBdTerrain, SIGNAL(clicked()), this, SLOT(openBdTerrain()));
    QObject::connect(this, SIGNAL(accepted()), this, SLOT(acceptParam()));
}

void BddDir::openBdBarem ()
{
    QString BdBareme = QFileDialog::getOpenFileName(this, "Selectionner une base Access", QString(), "Bases Access (*.mdb)");
    ui->dirBareme->setText(BdBareme);
}

void BddDir::openBdLocale ()
{
    QString BdLocale = QFileDialog::getOpenFileName(this, "Selectionner une base Access", QString(), "Bases Access (*.mdb)");
    ui->dirLocale->setText(BdLocale);
}

void BddDir::openBdTerrain ()
{
    QString BdTerrain = QFileDialog::getOpenFileName(this, "Selectionner une base Access", QString(), "Bases Access (*.mdb)");
    ui->dirTerrain->setText(BdTerrain);
}

void BddDir::acceptParam()
{
    QSettings settings("GaliHydro.ini", QSettings::IniFormat);
    QString dirBarem = DirNoSlash(ui->dirBareme->text());
    QString dirLocale = DirNoSlash(ui->dirLocale->text());
    QString dirTerrain = DirNoSlash(ui->dirTerrain->text());
    settings.setValue("BDD/bareme",dirBarem);
    settings.setValue("BDD/locale",dirLocale);
    settings.setValue("BDD/terrain",dirTerrain);

    emit refreshTable ();
}

QString BddDir::DirNoSlash (QString Dir)
{
    QString DirChanged = Dir.replace("/","\\");

    qDebug () << Dir;
    qDebug () << DirChanged;

    return DirChanged;
}

BddDir::~BddDir()
{
    delete ui;
}
