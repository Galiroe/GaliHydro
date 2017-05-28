#ifndef BDD_SINGLETON_H
#define BDD_SINGLETON_H

#include <QMutex>
#include <QSqlDatabase>

class BDD_Singleton : public QSqlDatabase
{
public:
    static BDD_Singleton* instance()
    {
        static QMutex mutex;
        if (!m_Instance)
        {
            mutex.lock();

            if (!m_Instance)
                m_Instance = new BDD_Singleton;

            mutex.unlock();
        }

        return m_Instance;
    }

    static void drop()
    {
        static QMutex mutex;
        mutex.lock();
        delete m_Instance;
        m_Instance = 0;
        mutex.unlock();
    }

private:
    BDD_Singleton() {}

    BDD_Singleton(const BDD_Singleton &); // hide copy constructor
    BDD_Singleton& operator=(const BDD_Singleton &); // hide assign op
                                 // we leave just the declarations, so the compiler will warn us
                                 // if we try to use those two functions by accident

    static BDD_Singleton* m_Instance;
};



/*
#include <QObject>

class BDD_Singleton : public QObject
{
    Q_OBJECT
public:
    explicit BDD_Singleton(QObject *parent = 0);
    ~BDD_Singleton();

signals:

public slots:
};
*/
#endif // BDD_SINGLETON_H
