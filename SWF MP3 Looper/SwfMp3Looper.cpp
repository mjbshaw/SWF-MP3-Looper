#include "SwfMp3Looper.hpp"
#include "Transcode.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"

#include <QtGui/QFileDialog>
#include <QMessageBox>

#include <exception>

SwfMp3Looper::SwfMp3Looper(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.selectFileButton, SIGNAL(pressed()), this, SLOT(selectFile()));
	connect(ui.saveAsButton, SIGNAL(pressed()), this, SLOT(saveAs()));
	connect(ui.cancelButton, SIGNAL(pressed()), this, SLOT(cancel()));
	connect(ui.sourceFileLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(sourceChanged(const QString&)));
}

SwfMp3Looper::~SwfMp3Looper()
{
}

void SwfMp3Looper::sourceChanged(const QString& source)
{
	ui.saveAsButton->setEnabled(!source.isEmpty());
}

void SwfMp3Looper::selectFile()
{
	QString path = QFileDialog::getOpenFileName(this, "", "", "Lossless audio file (*.wav *.flac)");
	if (!path.isEmpty())
	{
		ui.sourceFileLineEdit->setText(path);
	}
}

void SwfMp3Looper::saveAs()
{
	std::string path = QFileDialog::getSaveFileName(this, "", "", "SWF file (*.swf)").toStdString();
	std::string source = ui.sourceFileLineEdit->text().toStdString();

	if (!path.empty())
	{
		ui.cancelButton->setEnabled(true);
		cancelEncode = false;

		std::string className = ui.classNameLineEdit->text().toStdString();
		int vbrQuality = ui.vbrQualitySlider->value();
		int audioQuality = ui.algorithmicQualitySlider->value();
		int sampleRate = ui.sampleRateComboBox->currentIndex() == 0 ? 11025 :
						 ui.sampleRateComboBox->currentIndex() == 1 ? 22050 :
																	  44100;
		
		try
		{
			AudioDecoder decoder(source);
			AudioEncoder encoder(sampleRate, audioQuality, vbrQuality);

			std::function<bool(float)> callback = [this](float t) -> bool {
				ui.progressBar->setValue((int)(t * 100));
				QApplication::processEvents();
				return !cancelEncode;
			};

			transcode(decoder, encoder, callback);
		}
		catch (std::exception& e)
		{
			QMessageBox::critical(this, "Error!", e.what());
		}

		ui.cancelButton->setEnabled(false);
	}
}

void SwfMp3Looper::cancel()
{
	cancelEncode = true;
}