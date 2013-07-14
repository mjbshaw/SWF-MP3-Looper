#include "SwfMp3Looper.hpp"
#include "Transcode.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"
#include "SwfMp3Sound.hpp"

#include <QFileDialog>
#include <QMessageBox>

#include <exception>

SwfMp3Looper::SwfMp3Looper(QWidget* parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.selectFileButton, SIGNAL(pressed()), this, SLOT(selectFile()));
    connect(ui.saveAsButton, SIGNAL(pressed()), this, SLOT(saveAs()));
    connect(ui.cancelButton, SIGNAL(pressed()), this, SLOT(cancel()));
    connect(ui.sourceFileLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged()));
    connect(ui.classNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged()));

    connect(ui.codecComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(codecChanged(int)));
}

SwfMp3Looper::~SwfMp3Looper()
{
}

void SwfMp3Looper::textChanged()
{
    ui.saveAsButton->setEnabled(!ui.sourceFileLineEdit->text().isEmpty() && !ui.classNameLineEdit->text().isEmpty());
    ui.progressBar->setValue(0);
}

void SwfMp3Looper::selectFile()
{
    QString path = QFileDialog::getOpenFileName(this, "", "", "Audio files (*.*)");
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
        int channelCount = ui.channelsComboBox->currentIndex() == 0 ? -1 : // Same as source
                           ui.channelsComboBox->currentIndex() == 1 ?  2 : // Stereo
                                                                       1;  // Mono
        AVCodecID codecId = ui.codecComboBox->currentIndex() == 0 ? AV_CODEC_ID_MP3 : AV_CODEC_ID_PCM_S16LE;

        try
        {
            AudioDecoder decoder(source);

            if (channelCount < 0) // Same as source
            {
                channelCount = decoder.getChannelCount();
            }

            AudioEncoder encoder(codecId, channelCount, sampleRate, audioQuality, vbrQuality);

            std::function<bool(float)> callback = [this](float t) -> bool {
                ui.progressBar->setValue((int)(t * 99)); // 0-99% for transcode, 100% for file save
                QApplication::processEvents();
                return !cancelEncode;
            };

            SwfMp3Sound swf;
            swf.data = transcode(decoder, encoder, callback);

            swf.channelCount = encoder.getChannelCount();
            swf.className = className;
            swf.sampleCount = encoder.getEncodedSampleCount();
            swf.sampleRate = encoder.getSampleRate();
            swf.sampleSize = 8 * av_get_bytes_per_sample(encoder.getSampleFormat());
            swf.mp3 = codecId == AV_CODEC_ID_MP3;
            swf.seekSamples = decoder.getDelay() + encoder.getDelay();

            swf.saveSwf(path);

            ui.progressBar->setValue(100);
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
void SwfMp3Looper::codecChanged(int index)
{
    if (index == 0) // MP3
    {
        ui.algorithmQualityGroup->setEnabled(true);
        ui.vbrQualityGroup->setEnabled(true);
    }
    else // WAV
    {
        ui.algorithmQualityGroup->setEnabled(false);
        ui.vbrQualityGroup->setEnabled(false);
    }
}
