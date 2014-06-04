#include "video.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	video w;
	w.show();
	return a.exec();
}
