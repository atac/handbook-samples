#ifndef VIDEO_H
#define VIDEO_H

#include <QtWidgets/QMainWindow>
#include <qprocess.h>
#include <qthread.h>
#include "ui_video.h"

class Loader : public QThread {
	Q_OBJECT

	public:
		Loader(QWidget * parent, QString filename);
		QString filename, tmp;
		int finished = 1, size = 0, pos = 0;
		void run();
		void quit();

	signals:
		void done(const QString &result);
};

class video : public QMainWindow
{
	Q_OBJECT

public:
	video(QWidget *parent = 0);
	void resizeEvent(QResizeEvent * event);
	void init();
	void video::closeEvent(QCloseEvent *event);
	void load_file(QString filename);
	void load_file();
	~video();

public slots:
	void play();
	void menu_select(QAction*);

private:
	Ui::MainWindow ui;
	QProcess * player;
	QGridLayout * grid;
	QString filename;
	Loader * loader;
};

#endif // VIDEO_H
