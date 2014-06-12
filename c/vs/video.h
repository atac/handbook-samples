#ifndef VIDEO_H
#define VIDEO_H

#include <QtWidgets/QMainWindow>
#include <qprocess.h>
#include "ui_video.h"

class Player : public QWidget {
public:
	Player(QWidget * parent);
	Player();

private:
	QProcess * process;
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

};

#endif // VIDEO_H
