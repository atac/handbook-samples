#define NOMINMAX
#include <Windows.h>
#include <gl\GLU.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtMultimedia>
#include <QMediaPlayer>
#include <qvideowidget.h>
#include <stdio.h>
#include "video.h"

#include "qmpwidget.h"
#include "qmpwidget.cpp"

// Constructor
video::video(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	const QAbstractButton* play_btn = this->findChild<QAbstractButton*>(QString("play_btn"));
	QObject::connect(play_btn, &QAbstractButton::click, this, &video::play);

	QMenuBar * menubar = this->findChild<QMenuBar*>(QString("menubar"));
	QObject::connect(menubar, &QMenuBar::triggered, this, &video::menu_select);

	QMPwidget * vid = new QMPwidget;
	vid->setMPlayerPath(QString("..\\..\\mplayer.exe"));
	vid->start(QStringList("13.mpg"));
	player = vid->process();

}

void video::closeEvent(QCloseEvent *event){
	player->close();
}

// Destructor
video::~video(){}

// Navigation menu events
void video::menu_select(QAction * action){
	if (action->text() == "Exit"){
		QApplication::quit();
	}
	else if (action->text() == "Open"){
		this->load_file();
	}
}

// Start C10>video export and initialize screens for each video channel.
void video::load_file(QString filename){}

// Use a QFileDialog to select and load a file.
void video::load_file(){
	QString filename = QFileDialog::getOpenFileName(this, "Load Chapter 10 File", ".", "Chapter 10 Files (*.c10 *.ch10);;All Files (*.*)");
	this->load_file(filename);
}

// Play / Pause
void video::play(){}

// Initialize and run.
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	video w;
	w.show();

	return a.exec();
}
