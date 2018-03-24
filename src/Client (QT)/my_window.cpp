//////////////////////////////////////////////////////////////
// my_window.cpp - Blueprint for the GUI of the program
//
// Author:	Rithvik Mitresh Ballal
// 			Tejas Chavan
// Date:	1-March-2017
//
// Description:
// ------------
//This file contains all the function , signals and slot required for interaction between the GUI and the backend(C++).
//It also contains methods used to handle the emitted signal from the Thread class.
/////////////////////////////////////////////////////////////////

#include "my_window.h"
#include "ui_mywindow.h"
#include<QThread>
MyWindow::MyWindow(QWidget *parent) :
    QMainWindow(parent),                                                                                                 //This is the main window
    ui(new Ui::MyWindow)
{
   socket = new QTcpSocket(this);                                                                                        //A new socket object is instantiated. This socket is used to communicate with the server.
   mThread =new Thread(this);                                                                                            //A new Thread object is instantiated. This is used to continously poll for change in temperature and humidity to the server.
   ui->setupUi(this);                                                                                                    //Open the window
   QString text="Disconnected";                                                                                          //This is used to set the GUI connection_status label as disconnected.
   timer=new QTimer(this);                                                                                               //A new Timer object is instantiated. This is used to get the system time and display it in the GUI.
   ui->progressBar->setValue(0);
   ui->connection_status->setText(text);                                                                                 //This uses QString text to set the connection_status label in the GUI.
   connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(connectedToServer()));                                             //This is used to connect buttonBox(open) with the connectedServer() function.When the open button in GUI is pressed,connectedToServer() function is triggered.
   connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError))); //This is used to display the error of signal not being connected.
   connect(ui->dial,SIGNAL(valueChanged(int)),this,SLOT(sendData()));                                                    //This is used to connect dial button(RPM) with the ValueChanged(int)(Which is an in-built function) signal.When the dial button in GUI is rotated,sendData() function is triggered which will send the duty cycle value to the server through the socket.
   connect(ui->dial,SIGNAL(valueChanged(int)),ui->progressBar,SLOT(setValue(int)));                                      //This is used to connect dial button(RPM) with the ValueChanged(int)(Which is an in-built function) signal.When the dial button in GUI is rotated,setValue(int) function is triggered which will update the progress bar(duty cycle).
   connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(disconnectFromServer()));                                          //This is used to disconnect from the server.
   connect(timer,SIGNAL(timeout()),this,SLOT(showTime()));                                                               //This is used to connect the timer object with the timeout() function, which will be called every one minute. When the timeout() signal is called,showTime() function, is called. This will update the GUI, with the latest system time.
   connect(socket,SIGNAL(disconnected()),this,SLOT(disconnectedAbuptly()));                                              //This is used to disconnect the socket if the connection is been disconnected aburtly.
   connect (socket,SIGNAL(readyRead()),this,SLOT(readDataFromSocket()));                                                 //This is used to read the data from the socket.
   connect(ui->checkBox,SIGNAL(clicked(bool)),this,SLOT(isChecked(bool)));                                               //This is used to connect the checkBox with the clicked(bool)(which is an in-built function) signal.When the checkBox is pressed, the isCheckBox(bool) function is called which will change, how the temperature is requested from the server(in degree c or degree f).
   connect(mThread,SIGNAL(callWriteSocket()),this,SLOT(sendDataContinously()));                                          //This is used to request for temperature and humidity continuously from the server.The request is emitted from the thread using the callWriteSocket signal.
   timer->start();                                                                                                       //Will start the timer, which will the current system time.
}
MyWindow::~MyWindow()                                                                                                    //A destructor, which will destory the object from the heap when the object is dereferenced.
{
    delete ui;
}

void MyWindow::sendData()                                                                                               //This will send the data to the server.This is triggerred when the dial button is changed.
{

   int x=ui->dial->value();                                                                                             //Capture the change in dial value.
   QString b=QString::number(x);                                                                                        //Converts the number to string.
   QByteArray block=b.toLatin1();                                                                                       //Converts the string to a latin format(which includes only ASCII) and then again the result is converted to a ByteArray.The string is converted to ByteArray, because the data is transfered in sockets a bytes of data.
   socket->write(block);                                                                                                //Writes the ByteArray to the socket.
   generateRPM(x);                                                                                                      //Generates the RPM depending on the dial value.

}
void MyWindow::connectedToServer()                                                                                      //This function is used to connect to the server.
{

   socket->connectToHost(ui->ip_address->toPlainText(),7);                                                              //Pings for connection to the ip address mentioned in the ip_address dialog box.
   if(socket->waitForConnected(2000))                                                                                   //Waits for connection to be establised.
   {
       ui->connection_status->setText("Connected");                                                                     //The connection_status label is changed to "Connected" if the connection is established successfully.
       mThread->start();
    }
   else
   {
        QMessageBox::information(this, tr("Server Client Model"),
                                 tr("The following error occurred: %1.Or You have given an invalid address")           //This message is displayed if the connectioned failed.
                                 .arg(socket->errorString()));
   }
}

void MyWindow::displayError(QAbstractSocket::SocketError socketError)                                                  //This function displays message on other error pertaining to the signal connection.
{
    switch (socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Server Client Model"),
                                     tr("The host was not found. Please check the "
                                        "host name and make sure it is sharing."));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Server Client Model"),
                                     tr("The connection was refused by the peer. "
                                        "Make sure the other side is sharing, "
                                        "and check that the host name and port "
                                        "settings are correct."));
            break;
        default:
            QMessageBox::information(this, tr("Server Client Model"),
                                     tr("The following error occurred: %1.")
                                     .arg(socket->errorString()));
     }

    socket->close();                                                                                                   //This is used to close the socket connection.
}
void MyWindow::disconnectFromServer()                                                                                  //This is function is used to disconnect fromt the server.
{
    socket->write(QString::number(0).toLatin1());                                                                      //This sends a duty cycle of zero to off the fan, when we want to disconnect from the server.
    socket->waitForBytesWritten(3000);                                                                                 //We are waiting for all the bytes to be written from the socket on to the server.
    ui->lcdNumber->display(0);                                                                                         //Resetting the LCD screen in the GUI to its initial state.
    ui->lcdNumber_2->display(0);
    ui->lcdNumber_3->display(0);
    socket->disconnectFromHost();
    ui->connection_status->setText("Disconnected");                                                                    //Changing the connection_status label from "Connected" to "Disconnected"

}

void MyWindow::showTime()                                                                                              //This slot function which is called when a timeout signal is emitted after every one minute.
{
    QDateTime dateTime=QDateTime::currentDateTime();                                                                   //Takes the currenct time from the system.
    ui->system_clock->setDateTime(dateTime);                                                                           //Sets the system_clock dialog in GUI with the current system time.

}


void MyWindow::on_pushButton_clicked()                                                                                //This is a slot function when a version button is selected.This function gives the current version of the client as well as the server.
{
    QMessageBox::about(this, tr("Version"),
                            tr("The Application Version is 1.0 \n The Server Version is 1.0"));                       //This prompt a message box, which contains the current version of the client and the sever.
}
void MyWindow::disconnectedAbuptly()                                                                                  //This is a slot function which is emitted when the server is suddenly disconnect(eg. removing the ethernet cable).
{
    ui->connection_status->setText("Disconnected");                                                                   //This is setting the connection_status label of GUI to "Disconnected"
    ui->lcdNumber->display(0);                                                                                        //Resetting the value of the LCD to 0
    ui->lcdNumber_2->display(0);
    ui->lcdNumber_3->display(0);
}
void MyWindow::readDataFromSocket()                                                                                   //This is a slot which is called when a ReadReady() signal is emitted. This is place where we recieve the data from server.
{
   if(!socket->bytesAvailable())                                                                                      //Check whether there is any bytes available to be read.
   {
       return;
   }
   QByteArray m=socket->readAll();                                                                                    //Reading the data  sent by the server and assigning it back to the ByteArray.
   qDebug()<<m.size();
    long command_data=m.toLongLong();                                                                                 //Converting the ByteArray data to long.We are converting it to long because we are recieving data only in the form of numbers.
   quint8 temperature_humidity=command_data;                                                                          //Extracting the data part from the 16bit packet recieved from the server.The LSB 8 bits are the data and MSB 8 bits are the Command.
   QString value=QString::number(temperature_humidity);
    switch(command_data>>8)                                                                                           //Checking whether the command is for temperature(2) or for humidity (4).
    {
     case 2:
        ui->lcdNumber->display(value);                                                                               //Update the LCD with updated temperature value.
        if((isdegreeC==false && value.toInt()>=86)||(isdegreeC==true && value.toInt()>=30))                           //This condition is to see whether to oning the fan is necessary.
            isOnFan(true);
        else
            isOnFan(false);
        break;
     case 4:
        ui->lcdNumber_2->display(value);                                                                               //Update the humidity LCD with new value.
    }
}
void MyWindow::isChecked(bool value)
{
    if(value==true)
    {
        isdegreeC=true;
    }
    else
    {
        isdegreeC=false;
    }
}
void MyWindow::sendDataContinously()                                                                                  //This is a slot function which is used to send the data continuously. This slot function is called when callWriteSocket signal is emitted from the Thread.
{
    if(!(socket->state()==QAbstractSocket::ConnectedState))                                                           //This is condition to check whether the socket is in connected state.
    {
        return;
    }
    if(isdegreeC)                                                                                                     //condition to check whether the temperature must be requested in celcius.
    {
    socket->write("327680");                                                                                          //This is the packet sent to server to request for temperature in degrees celcius.
    socket->waitForBytesWritten(3000);                                                                                //Waiting for the bytes(packets) to be written to the server.

   }
    else
    {
      socket->waitForBytesWritten(3000);
      socket->write("65536");                                                                                        //This is the packet sent to server to request for temperature in degrees Fahrenheit.
      socket->waitForBytesWritten(3000);
    }
    socket->waitForBytesWritten(3000);
    socket->write("196608");                                                                                         //This is the packet sent to server to request for humidity.




}
void MyWindow::generateRPM(int duty_cycle)                                                                          //This is function used to generate the RPM for a given duty cycle.
{
   double rpm=(duty_cycle*2000)/100;
   ui->lcdNumber_3->display(rpm);                                                                                   //Updates the LCD display of the GUI with a number value.

}

void MyWindow::isOnFan(bool condition)                                                                             //This function is used send a onFan command to temperature, since the temperature has crossed 30.
{
    if(condition)                                                                                                  //Checks whether on an onFan command is true.
    {
        socket->write(QString::number(99).toLatin1());                                                             //Writes the duty cycle at which the fan must run. We have given a 99 percent duty cycle.
        ui->progressBar->setValue(99);                                                                             //Updates the progress bar with new duty cycle value.
        generateRPM(99);
        socket->waitForBytesWritten(3000);                                                                         //Waits for the Bytes to be written from socket to the server.
    }
    else
    {
        int temp=ui->dial->value();                                                                               //Takes the value from dial button.
        socket->write(QString::number(temp).toLatin1());                                                          //writes the value taken from dial button on to the socket.
        generateRPM(temp);                                                                                        //Calls the generateRPM function with duty cycle value taken from the dial button.
        ui->progressBar->setValue(temp);                                                                          //Updates the progress bar with new duty cycle value.
    }
}
