#ifndef VIDEO_H
#define VIDEO_H

#include <QtWidgets/QMainWindow>
#include <qprocess.h>
#include "ui_video.h"

class video : public QMainWindow
{
	Q_OBJECT

public:
	video(QWidget *parent = 0);
	void init();
	void video::closeEvent(QCloseEvent *event);
	void load_file(QString filename);
	void load_file();
	~video();

public slots:
	void play();
	void menu_select(QAction*);

private:
	QProcess *player;
	Ui::MainWindow ui;
	QString filename;

};

#endif // VIDEO_H
