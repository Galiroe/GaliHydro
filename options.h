#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>
#include <QColor>
#include <QProcessEnvironment>
#include <QCryptographicHash>

enum processingMethod {afterPassageMethod,fixedDateMethod};

namespace Ui {
class options;
}

class options : public QDialog
{
    Q_OBJECT

public:
    explicit options(QWidget *parent = 0);
    ~options();

signals:
    void refreshTable ();
    void changeTolerance (int);
    void changeTimeOutDay (int);
    void changeFixedDate (int);
    void ChangeTimeOutDayStatus (bool);
    void methodChanged (processingMethod);

private slots:
    void processingDialogColor();
    void bulletinDialogColor ();
    void timeOutDayDialogColor ();
    void defautProcessingColor();
    void defautBuletinColor();
    void defautTimeOutDayColor();
    void acceptParam();
    void methodeAfterPassage (bool choice);
    void methodeFixedDate (bool choice);
    void echoChange (bool echoMode);
    void checkAdminSlot();
    void delAdminSlot();
    void adminTabActivation();

private:
    int tolerance;
    int timeOutDay;
    int fixedDate;
    bool timeOutDayStatus;
    QString userPwd;
    QPalette paletteProcessing;
    QPalette paletteBuletin;
    QPalette paletteTimeOutDay;
    QColor colorProcessing;
    QColor colorBuletin;
    QColor colorTimeOutDay;
    Ui::options *ui;
    processingMethod method;
    QWidget *adminTab;
};

#endif // OPTIONS_H
