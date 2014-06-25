#define NOMINMAX

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <Windows.h>

#include <gl\GLU.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtMultimedia>
#include <QMediaPlayer>
#include <qvideowidget.h>
#include <qthread.h>

#include "stdint.h"
#include "irig106ch10.h"

#include "video.h"

#include "qmpwidget.h"
#include "qmpwidget.cpp"


// Byteswap a buffer.
void swap(char *p, int len) {
	char tmp;
	for (int i = 0; i < ((len / 2)); i++) {
		tmp = p[i * 2];
		p[i * 2] = p[(i * 2) + 1];
		p[(i * 2) + 1] = tmp;
	}
}


// The loader thread //

Loader::Loader(QWidget * parent, QString filename){
	this->filename = filename;
}

void Loader::run(){

	// Create output containers
	FILE * out[0x1000];
	for (int i = 0; i < 0x1000; i++){
		out[i] = NULL;
	}

	// Open input file.
	int input_handle;
	Irig106::EnI106Status status = Irig106::enI106Ch10Open(&input_handle, this->filename.toUtf8().constData(), Irig106::I106_READ);
	if (status != Irig106::I106_OK){
		qCritical() << "Error opening source file: ";
		return;
	}

	// Parse loop.
	Irig106::SuI106Ch10Header header;
	void * buffer = malloc(24);
	void * ts = malloc(188);
	while (1){

		// Read next header or exit.
		status = Irig106::enI106Ch10ReadNextHeader(input_handle, &header);
		if (status != Irig106::I106_OK){
			qDebug() << "Finished";
			return;
		}

		// Ensure that we're interested in this particular packet.
		if (0x3F > header.ubyDataType || header.ubyDataType > 0x43){
			continue;
		}

		// Create a temporary file for this channel.
		_mkdir("tmp");

		char path[1000];
		strcpy(path, "./tmp");
		char channel[5];
		sprintf(channel, "/%d", header.uChID);
		strcat(path, channel);
		strcat(path, ".mpg");

		if (out[header.uChID] == NULL){
			qDebug() << "Opening file: ";
			qDebug() << path;
			out[header.uChID] = fopen(path, "wb");

			if (out[header.uChID] == NULL){
				qCritical() << ("Error opening output file: %s", path);
				continue;
			}
		}

		// Read packet data.
		buffer = realloc(buffer, header.ulPacketLen);
		status = Irig106::enI106Ch10ReadDataFile(input_handle, header.ulPacketLen, buffer);
		if (status != Irig106::I106_OK){
			qCritical() << "Error reading packet.";
			continue;
		}

		// Ignore first 4 bytes (CSDW)
		BYTE * data = (BYTE *)buffer + 4;
		int datalen = header.ulDataLen - 4;
		for (int i = 0; i < (datalen / 188); i++){
			memcpy(ts, data + (i * 188), 188);
			swap((char *)ts, 188);
			fwrite(ts, 1, 188, out[header.uChID]);
		}
	}

}


// The Video Player //

// Constructor
video::video(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	const QAbstractButton* play_btn = this->findChild<QAbstractButton*>(QString("play_btn"));
	QObject::connect(play_btn, &QAbstractButton::click, this, &video::play);

	QMenuBar * menubar = this->findChild<QMenuBar*>(QString("menubar"));
	QObject::connect(menubar, &QMenuBar::triggered, this, &video::menu_select);

	QGridLayout * grid = this->findChild<QGridLayout*>(QString("grid"));

	QFrame * container = new QFrame();
	QMPwidget * vid = new QMPwidget(container);
	vid->setMPlayerPath(QString("..\\..\\mplayer.exe"));

	//vid->setVideoOutput(QString("directx:noaccel"));
	QString winid = QString::number((int)(container->winId()));
	vid->start(QStringList("-wid") << winid);
	//vid->load("14.mpg");
	grid->addWidget(container, 0, 0);

	player = vid->process();

}

void video::resizeEvent(QResizeEvent * event){
	/*
	QRect geo = this->geometry();
	QRect grid_size = QRect(QPoint(0, 0), QPoint(geo.right, geo.bottom - 200));
	grid->setGeometry(grid_size);
	//*/
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
void video::load_file(QString filename){
	this->loader = new Loader(this, filename);
	this->loader->start();
}

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

	if (argc > 1){
		w.load_file(argv[1]);
	}

	return a.exec();
}
