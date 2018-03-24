//////////////////////////////////////////////////////////////
// thread.h - Header file for the thread.cpp
//
// Author:	Rithvik Mitresh Ballal
//			Tejas chavan
// Date:	1-March-2017
//
// Description:
// ------------
//This file contains class attributes and the class definition for Thread class, signal for the corresponding class.
//
/////////////////////////////////////////////////////////////////

#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include<QThread>
#include<QDebug>

class Thread : public QThread                       //This is the class definition of the Thread class.
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = nullptr);    //Explicitly declaring the constructor of MyWindows class
    void run();                                    //This is an abstract method from QThread class

signals:
    void callWriteSocket();                       //This is the signal emitted from the Thread class every one second.
public slots:
};

#endif // THREAD_H
