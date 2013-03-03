#ifndef SWFMP3LOOPER_H
#define SWFMP3LOOPER_H

#include <QtGui/QMainWindow>
#include "ui_swfmp3looper.h"

class SWFMP3Looper : public QMainWindow
{
	Q_OBJECT

public:
	SWFMP3Looper(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SWFMP3Looper();

private:
	Ui::SWFMP3LooperClass ui;
};

#endif // SWFMP3LOOPER_H
