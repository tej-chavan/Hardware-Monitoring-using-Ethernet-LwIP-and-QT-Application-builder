//////////////////////////////////////////////////////////////
// main.cpp - Blueprint for the entry function(main())
//
// Author:	Rithvik Mitresh Ballal
//			Tejas Chavan
// Date:	1-March-2017
//
// Description:
// ------------
//This file is the entry point of the program. It has instance variable of  MyWindow.
//
/////////////////////////////////////////////////////////////////

#include "my_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyWindow w;                                 //MyWindow instance
    w.show();                                   //Shows the GUI, when the application starts

    return a.exec();                            //Goings to MyWindows class after execution.
}
