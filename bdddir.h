#ifndef BDDDIR_H
#define BDDDIR_H

#include <QDialog>

namespace Ui {
class BddDir;
}

class BddDir : public QDialog
{
    Q_OBJECT

public:
    explicit BddDir(QWidget *parent = 0);
    ~BddDir();

signals:
    void refreshTable ();

private slots:
    void openBdBarem ();
    void openBdLocale ();
    void openBdTerrain ();
    void acceptParam();

private:
    QString DirNoSlash (QString Dir);
    Ui::BddDir *ui;
};

#endif // BDDDIR_H
