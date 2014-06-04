#include "sandbox.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Sandbox w;
	w.show();
	return a.exec();
}
