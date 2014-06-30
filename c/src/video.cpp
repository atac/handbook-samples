#define NOMINMAX

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <Windows.h>
#include <io.h>

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


// The Ticker thread - emit Ticker.tick once per second //

void Ticker::run(){
	this->running = 1;
	while (this->running){
		emit this->tick();
		Sleep(1000);
	}
	this->quit();
}


// The loader thread //

Loader::Loader(QWidget * parent, QString filename){
	this->filename = filename;
	this->pos = 0;
	this->size = 0;
}

void Loader::run(){
	// Create output handles
	FILE * out[0x1000];
	for (int i = 0; i < 0x1000; i++){
		out[i] = NULL;
	}

	// Find the size of the input file.
	FILE * f = fopen(this->filename.toUtf8().constData(), "rb");
	_fseeki64(f, 0, SEEK_END);
	size = _ftelli64(f);
	fclose(f);

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

		// Skip non-video packets.
		if (0x3F > header.ubyDataType || header.ubyDataType > 0x43){
			continue;
		}

		// Create a temporary file for this channel.
		_mkdir("tmp");

		// Get the correct filename for this channel.
		char path[1000];
		strcpy(path, "./tmp");
		char channel[5];
		sprintf(channel, "/%d", header.uChID);
		strcat(path, channel);
		strcat(path, ".mpg");

		// Ensure an output file is open for this channel.
		if (out[header.uChID] == NULL){
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
		Irig106::enI106Ch10GetPos(input_handle, &pos);

		// Ignore first 4 bytes (CSDW)
		BYTE * data = (BYTE *)buffer + 4;
		
		// Byteswap and output data.
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

	// Set up UI
	
	ui.setupUi(this);

	QMenuBar * menubar = this->findChild<QMenuBar*>(QString("menubar"));
	connect(menubar, &QMenuBar::triggered, this, &video::menu_select);

	this->grid = this->findChild<QGridLayout*>(QString("grid"));

	this->volume = this->findChild<QSlider*>(QString("volume"));
	connect(this->volume, &QSlider::sliderMoved, this, &video::set_volume);

	const QAbstractButton * play_btn = this->findChild<QAbstractButton*>(QString("play_btn"));
	connect(play_btn, &QAbstractButton::clicked, this, &video::play);

	QProgressBar * load_meter = this->findChild<QProgressBar*>("load_meter");

	// Background events
	ticker = new Ticker;
	connect(ticker, &Ticker::tick, this, &video::tick);
	ticker->start();
}

void video::set_volume(int to){}

// Background updates (once per second).
void video::tick(){

	// Loader progress bar and label.
	QLabel * load_label = this->findChild<QLabel*>("load_label");
	QProgressBar * load_meter = this->findChild<QProgressBar*>("load_meter");
	int percent = 100;
	if (this->loader != NULL && !this->loader->isFinished()){
		char pos[0x1000];
		itoa(this->loader->pos / 1024 / 1024, pos, 10);
		char size[0x1000];
		itoa(this->loader->size / 1024 / 1024, size, 10);
		load_label->setText("Read " + QString(pos) + " / " + QString(size) + " mb");
		percent = ((float) this->loader->pos / (float) this->loader->size) * 100;
	}
	else {
		load_label->setText("Done");
	}
	load_meter->setValue(percent);
}

// Navigation menu events
void video::menu_select(QAction * action){
	if (action->text() == "Exit"){
		QApplication::quit();
	}
	else if (action->text() == "Open"){
		this->load_file();
	}
}

// Create and add a widget for a single video.
void video::add_video(QString path){
	// Create widget and player.
	QFrame * container = new QFrame();
	QMPwidget * vid = new QMPwidget(container);
	vid->setMPlayerPath(QString("..\\..\\mplayer.exe"));
	vid->start(QStringList("-wid") << QString::number((int)(container->winId())));
	vid->writeCommand(QString("pausing_keep_force loadfile ") + path);
	vid->writeCommand(QString("pausing_keep_force volume 0 1"));

	// Find place in grid and add widget to window.
	int x = 0, y = this->grid->rowCount() - 1;
	if (y < 0){
		y = 0;
	}
	while (this->grid->itemAtPosition(y, x) != 0){
		if (x == 2){
			x = 0;
			y++;
			continue;
		}
		x++;
	}

	this->grid->addWidget(container, y, x);
}

// Start C10 video export and initialize screens for each video channel.
void video::load_file(QString filename){
	this->loader = new Loader(this, filename);
	this->loader->start();

	// Clear grid.
	for (int i = 0; i < this->grid->count(); i++){
		QWidget * frame = this->grid->itemAt(i)->widget();
		QMPwidget * vid = (QMPwidget *)frame->children()[0];
		this->grid->removeWidget(frame);
		vid->writeCommand(QString("quit"));
		delete frame;
	}

	Sleep(250);

	// Add new videos from ./tmp
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile((LPCWSTR) L"tmp\\*.mpg", &fd);
	do {
		QString name = QString::fromWCharArray(fd.cFileName);
		this->add_video(QString("tmp/") + name);
		//@todo: add to audio options
	} while (FindNextFile(h, &fd) != 0);
}

// Use a QFileDialog to select and load a file.
void video::load_file(){
	QString filename = QFileDialog::getOpenFileName(this, "Load Chapter 10 File", ".", "Chapter 10 Files (*.c10 *.ch10);;All Files (*.*)");
	if (filename != ""){
		this->load_file(filename);
	}
}

// Play / Pause
void video::play(){
	for (int i = 0; i < this->grid->count(); i++){
		QWidget * frame = this->grid->itemAt(i)->widget();
		QMPwidget * vid = (QMPwidget *) frame->children()[0];
		vid->writeCommand("pause");
	}
}

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
