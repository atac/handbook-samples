#include <QtWidgets/QApplication>
#include "video.h"

video::video(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

video::~video()
{

}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	video w;
	w.show();
	return a.exec();
}
