#ifndef CORTH_H
#define CORTH_H

#include <QMainWindow>

namespace Ui {
class corth;
}

class corth : public QMainWindow
{
    Q_OBJECT

public:
    explicit corth(QWidget *parent = 0);
    ~corth();

private slots:

private:
    Ui::corth *ui;
};

#endif // CORTH_H
