#ifndef SWF_MP3_LOOPER_HPP
#define SWF_MP3_LOOPER_HPP

#include <QtGui/QMainWindow>
#include "ui_SwfMp3Looper.h"

class SwfMp3Looper : public QMainWindow
{
	Q_OBJECT

public:
	SwfMp3Looper(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SwfMp3Looper();

private:
	Ui::SwfMp3Looper ui;
};

#endif
