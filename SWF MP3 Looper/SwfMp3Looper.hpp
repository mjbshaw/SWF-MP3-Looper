#ifndef SWF_MP3_LOOPER_HPP
#define SWF_MP3_LOOPER_HPP

#include <QtGui/QDialog>
#include "ui_SwfMp3Looper.h"

class SwfMp3Looper : public QDialog
{
	Q_OBJECT

public:
	SwfMp3Looper(QWidget *parent = nullptr);
	~SwfMp3Looper();

private:
	Ui::SwfMp3Looper ui;
	bool cancelEncode;

private slots:
	void sourceChanged(const QString& source);
	void selectFile();
	void saveAs();
	void cancel();
};

#endif
