#include "galihydro.h"
#include <QApplication>

#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

//Test bug caracteres speciaux
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
//Fin test bug caracteres speciaux

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Traduction en francais
    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&translator);

    galihydro w;
    w.show();

//Test bug caracteres speciaux
//    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
//    QSqlQuery query;
//    db.setDatabaseName("Driver={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=C:\\Users\\Clement\\Desktop\\projetC\\GaliHydro\\GaliHydro\\BAREME_2005.MDB");
//    if(db.open()) {
//        query.exec("SELECT nom, courdo FROM station");
//        while (query.next()) {
//            qDebug () << query.value(0).toString();
//            qDebug () << query.value(1).toString();
//        }
//    }
//Fin test bug caracteres speciaux

    return a.exec();
}
