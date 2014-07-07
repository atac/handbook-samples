#ifndef VIDEO_H
#define VIDEO_H

#include <QtWidgets/QMainWindow>
#include <qprocess.h>
#include <qthread.h>
#include "ui_video.h"

class Ticker : public QThread {
	Q_OBJECT

	public:
		int running;
		void run();

	signals:
		void tick();
};

class Loader : public QThread {
	Q_OBJECT

	public:
		Loader(QWidget * parent, QString filename);
		QString filename, tmp;
		int finished = 1;
		__int64 size, pos;
		void run();
		void quit();

	signals:
		void done();
};

class video : public QMainWindow
{
	Q_OBJECT

	public:
		video(QWidget *parent = 0);
		void load_file(QString filename);
		void load_file();

	public slots:
		void play();
		void tick();
		void seek(int to);
		void set_volume(int to);
		void audio_source(int index);
		void menu_select(QAction*);
		void video::finished_loading();
		void video::error(const QString &line);
		void video::info(const QString &line);

	private:
		Ui::MainWindow ui;
		int length = 0;
		int start_offset = 0;
		int audio_from = 0;
		QString last_video;
		void add_video(QString path);
		QSlider * slider;
		QSlider * volume;
		QProgressBar * load_meter;
		QComboBox * audio;
		QGridLayout * grid;
		QString filename;
		Loader * loader;
		Ticker * ticker;
};

#endif // VIDEO_H
