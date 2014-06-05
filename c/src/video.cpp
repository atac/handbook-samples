#include <QtWidgets/QApplication>
#include <stdio.h>
#include "video.h"

video::video(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	const QAbstractButton* play_btn = this->findChild<QAbstractButton*>(QString("play_btn"));
	QObject::connect(play_btn, &QAbstractButton::click, this, &video::play);

	QMenuBar * menubar = this->findChild<QMenuBar*>(QString("menubar"));
	QObject::connect(menubar, &QMenuBar::triggered, this, &video::menu_select);
}

void video::menu_select(QAction * action){
	if (action->text() == "Exit"){
		QApplication::quit();
	}
}

void video::play(){
	
}

video::~video()
{

}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	video w;
	w.show();
	return a.exec();
}
