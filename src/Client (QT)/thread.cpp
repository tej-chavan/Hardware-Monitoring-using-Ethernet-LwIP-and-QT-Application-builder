//////////////////////////////////////////////////////////////
// thread.cpp - Blueprint for thread model used for our program
//
// Author:	Rithvik Mitresh Ballal
//			Tejas chavan
// Date:	1-March-2017
//
// Description:
// ------------
//This file contains the method body for run method and a signal which is emitted for MyWindow every one second.
//
/////////////////////////////////////////////////////////////////

#include "thread.h"

Thread::Thread(QObject *parent) : QThread(parent)   //Constructor of the Thread class with QObject class as its parent and QThread as its parent class.
{

}
void Thread::run()                                  //This is abstract method inheritted from the QThread class.
{
  while(1)
  {

      Thread::msleep(1000);                         // A delay of 1 sec.
      emit callWriteSocket();                       //emits the callWriteSocket. This signal is captured in MyWindow class.

  }
}
