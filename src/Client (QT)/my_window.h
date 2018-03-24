//////////////////////////////////////////////////////////////
// my_window.h - Header file for the my_windows.cpp
//
// Author:	Rithvik Mitresh Ballal
//			Tejas chavan
// Date:	1-March-2017
//
// Description:
// ------------
//This file contains class attributes and the class definition for MyWindow class, signal and slots for the corresponding class.
//
/////////////////////////////////////////////////////////////////

#ifndef MY_WINDOW_H
#define MY_WINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMessageBox>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <iostream>
#include <string>
#include "thread.h"

using namespace std;
namespace Ui {
class MyWindow;
}

class MyWindow : public QMainWindow                                 //This is the class name which extends QMainWindow. My Windows class contains the GUI for the application.
{
    Q_OBJECT                                                        //convert header to cpp code. Tells GUI to use signal and slots

public:
    explicit MyWindow(QWidget *parent = 0);                         //Explicitly declaring the constructor of MyWindows class
    ~MyWindow();                                                    //The destructor for MyWindows class. This is used by the Heap to destory the MyWindows class when it is de-referenced.
    Thread *mThread;                                                //This holds the reference to Thread class.
    bool isdegreeC=false;                                           //This is variable which keeps track of if the cel button is pressed in the GUI.
    void isOnFan(bool condition);                                   //This is a function which is  used to on the fan if the temperature exceeds 30 degrees.

public slots:
    void connectedToServer();                                       //connect to the server
    void sendData();                                                //send the data.
    void displayError(QAbstractSocket::SocketError socketError);    //Catch connection errors
    void disconnectFromServer();                                    //disconnect from the server
    void showTime();                                                //It is used to get the time value from the system
    void disconnectedAbuptly();                                     //Used to disconnect the client from the server as soon as the connection from the server is lost(If we remove the ethernet cable)
    void readDataFromSocket();                                      //This is used to read data from the socket.
    void sendDataContinously();                                     //This is used to send the data to the server continously(Request for temperature and humidity continously).
    void generateRPM(int duty_cycle);                               //This is used to generate the RPM for a given duty cycle.
    void on_pushButton_clicked();                                   //This is used to display the version of server
    void isChecked(bool value);                                     //This is function for asking the server for temperature in degree c or degree f, depending on the check box.

private:
    Ui::MyWindow *ui;                                               //This is the reference to the GUI
    QTcpSocket *socket;                                             //This holds the reference of socket object.
    QTimer *timer;                                                  //This holds the reference to the timer object. This object will display the current system time on the GUI.

private:
};

#endif // MY_WINDOW_H
