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
			this->finished = 1;
			emit this->done();
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

	// Set up UI and connect events.
	
	ui.setupUi(this);

	QMenuBar * menubar = this->findChild<QMenuBar*>(QString("menubar"));
	connect(menubar, &QMenuBar::triggered, this, &video::menu_select);

	this->grid = this->findChild<QGridLayout*>(QString("grid"));

	this->volume = this->findChild<QSlider*>(QString("volume"));
	this->volume->setValue(40);
	connect(this->volume, &QSlider::sliderMoved, this, &video::set_volume);

	this->audio = this->findChild<QComboBox*>(QString("audio"));
	connect(this->audio, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &video::audio_source);

	const QAbstractButton * play_btn = this->findChild<QAbstractButton*>(QString("play_btn"));
	connect(play_btn, &QAbstractButton::clicked, this, &video::play);

	this->slider = this->findChild<QSlider*>(QString("slider"));
	connect(slider, &QSlider::sliderMoved, this, &video::seek);

	this->load_meter = this->findChild<QProgressBar*>("load_meter");

	// Background events
	ticker = new Ticker;
	connect(ticker, &Ticker::tick, this, &video::tick);
	ticker->start();
}

void video::seek(int to){
	//@todo: seek still isn't completely accurate
	qDebug() << QString::number(to);

	int multiplier = (this->length - this->start_offset);
	if (multiplier < 1){
		multiplier = 1;
	}
	to = (to / 100.0) * multiplier;

	QWidget * frame = this->grid->itemAt(0)->widget();
	QMPwidget * vid = (QMPwidget *)frame->children()[0];
	int pos = (int) vid->tell();
	if (pos < 1){
		pos = 0;
	}
	pos -= this->start_offset;

	int offset;
	if (pos > to){
		offset = -(pos - to);
	}
	else {
		offset = to - pos;
	}
	qDebug() << QString("Seek to ") + QString::number(offset) + " / " + QString::number(this->length) + " offset: " + QString::number(this->start_offset);
	for (int i = 0; i < this->grid->count(); i++){
		QWidget * frame = this->grid->itemAt(i)->widget();
		QMPwidget * vid = (QMPwidget *)frame->children()[0];
		vid->seek(offset);
	}
}

// Switch source audio channel.
void video::audio_source(int index){

	// Only works if we have video (duh!)
	if (this->grid->count() < 1){
		return;
	}

	// Mute current source if applicable.
	if (this->audio_from > -1){
		QWidget * frame = this->grid->itemAt(this->audio_from)->widget();
		QMPwidget * vid = (QMPwidget *)frame->children()[0];
		vid->writeCommand(QString("pausing_keep_force volume 0 1"));
	}

	// Enable new source.
	QWidget * frame = this->grid->itemAt(index)->widget();
	QMPwidget * vid = (QMPwidget *)frame->children()[0];
	vid->writeCommand(QString("pausing_keep_force volume ") + QString::number(this->volume->value()) + " 1");
	this->audio_from = index;

}

// Adjust volume on the selected channel.
void video::set_volume(int to){
	if (this->grid->count() < 1){
		return;
	}
	QWidget * frame = this->grid->itemAt(this->audio_from)->widget();
	QMPwidget * vid = (QMPwidget *)frame->children()[0];
	vid->writeCommand(QString("pausing_keep_force volume ") + QString::number(to) + " 1");
}

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

void video::finished_loading(){
	QMPwidget * vid = new QMPwidget;
	QMPwidget::connect(vid, &QMPwidget::readStandardOutput, this, &video::info);
	QMPwidget::connect(vid, &QMPwidget::readStandardError, this, &video::error);
	vid->setMPlayerPath(QString("..\\..\\mplayer.exe"));

	vid->start();
	//@todo: need to do this without mplayer complaining
	//vid->start(QStringList(QString("-vo=null") + " -ao=null"));
	vid->writeCommand(QString("pausing_keep_force loadfile ./tmp/" + this->last_video));
	vid->writeCommand(QString("pausing_keep_force get_property length"));
	vid->writeCommand(QString("pausing_keep_force get_property time_pos"));
	qDebug() << "Finished loading.";
}

void video::error(const QString &line){
	qDebug() << "Error: " << line;
}
	
void video::info(const QString &line){
	qDebug() << "Info: " << line;
	if (line.contains(QString("ANS_length="), Qt::CaseSensitive)){
		this->length = line.split(QString("="))[1].toFloat();
	}
	if (line.contains(QString("ANS_time_pos="), Qt::CaseSensitive)){
		this->start_offset = line.split(QString("="))[1].toFloat();
	}
}

// Start C10 video export and initialize screens for each video channel.
void video::load_file(QString filename){
	this->loader = new Loader(this, filename);
	connect(this->loader, &Loader::done, this, &video::finished_loading);
	this->loader->start();

	// Clear and audio selection.
	for (int i = 0; i < this->grid->count(); i++){
		QWidget * frame = this->grid->itemAt(i)->widget();
		QMPwidget * vid = (QMPwidget *)frame->children()[0];
		this->grid->removeWidget(frame);
		vid->writeCommand(QString("quit"));
		delete frame;
	}
	this->audio->clear();

	// Add new videos from ./tmp
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile((LPCWSTR) L"tmp\\*.mpg", &fd);
	QString name;
	do {
		name = QString::fromWCharArray(fd.cFileName);
		this->add_video(QString("tmp/") + name);
		this->audio->addItem(name);
	} while (FindNextFile(h, &fd) != 0);

	Sleep(250);

	QWidget * frame = this->grid->itemAt(0)->widget();
	QMPwidget * vid = (QMPwidget *)frame->children()[0];
	this->last_video = name;
	this->start_offset = vid->tell();
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
