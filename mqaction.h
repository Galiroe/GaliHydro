#ifndef MQACTION_H
#define MQACTION_H

#include <QAction>
#include <QObject>


class MQAction : public QAction
{
    Q_OBJECT

public:
    MQAction(const QString &text, QObject * parent = 0);
    ~MQAction();
    void setColumn (int column);
//    MQAction();
//    explicit MQAction(QObject * parent = 0);
//    explicit MQAction(const QString &text, QObject * parent = 0);
//    virtual bool event(QEvent * e);

signals:
    void mTriggered (bool state, int col);
    void mTriggered (int row);

private slots:
    void mTrigger(bool state);

private:
    //Fonctions privées
    int getColumn ();

    //Attribus privés
    int m_column;
};

#endif // MQACTION_H
