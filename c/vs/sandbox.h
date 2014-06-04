#ifndef SANDBOX_H
#define SANDBOX_H

#include <QtWidgets/QMainWindow>
#include "ui_sandbox.h"

class Sandbox : public QMainWindow
{
	Q_OBJECT

public:
	Sandbox(QWidget *parent = 0);
	~Sandbox();

private:
	Ui::SandboxClass ui;
};

#endif // SANDBOX_H
