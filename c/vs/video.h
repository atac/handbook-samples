#ifndef VIDEO_H
#define VIDEO_H

#include <QtWidgets/QMainWindow>
#include "ui_video.h"

class video : public QMainWindow
{
	Q_OBJECT

public:
	video(QWidget *parent = 0);
	~video();

private:
	Ui::videoClass ui;
};

#endif // VIDEO_H
