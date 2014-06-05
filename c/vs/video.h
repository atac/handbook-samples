#ifndef VIDEO_H
#define VIDEO_H

#include <QtWidgets/QMainWindow>
#include "ui_video.h"

class video : public QMainWindow
{
	Q_OBJECT

public:
	video(QWidget *parent = 0);
	void init();
	~video();

public slots:
	void play();
	void menu_select(QAction*);

private:
	Ui::MainWindow ui;

};

#endif // VIDEO_H
