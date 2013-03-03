#include "swfmp3looper.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SWFMP3Looper w;
	w.show();
	return a.exec();
}
