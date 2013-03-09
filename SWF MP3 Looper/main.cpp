#include "swfmp3looper.h"
#include <QtGui/QApplication>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main(int argc, char *argv[])
{
	avcodec_register_all();
	av_register_all();

	QApplication a(argc, argv);
	SWFMP3Looper w;
	w.show();
	return a.exec();
}
