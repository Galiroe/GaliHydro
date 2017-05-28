#include "mqaction.h"

MQAction::MQAction(const QString &text, QObject * parent): QAction(text, parent)
{
    QObject::connect(this, SIGNAL(triggered(bool)), this, SLOT(mTrigger(bool)));
}

MQAction::~MQAction()
{

}

void MQAction::setColumn (int column)
{
    m_column = column;
}

int MQAction::getColumn ()
{
    return m_column;
}

void MQAction::mTrigger(bool state)
{
    emit mTriggered (state,m_column);
}


