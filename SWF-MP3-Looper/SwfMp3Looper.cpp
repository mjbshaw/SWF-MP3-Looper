#include "SwfMp3Looper.hpp"
#include "Transcode.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"
#include "Swf.hpp"

#include <QStandardItemModel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMessageBox>

#include <exception>

enum UI { FILENAME, CLASSNAME, DEFAULT_SETTING, CODEC, SAMPLE_RATE, CHANNELS, ALGO_QUALITY, VBR_QUALITY };

SwfMp3Looper::SwfMp3Looper(QWidget* parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.addButton, SIGNAL(pressed()), this, SLOT(addFile()));
    connect(ui.removeButton, SIGNAL(pressed()), this, SLOT(removeFile()));
    connect(ui.saveAsButton, SIGNAL(pressed()), this, SLOT(saveAs()));
    connect(ui.cancelButton, SIGNAL(pressed()), this, SLOT(cancel()));

    connect(ui.classNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(classnameChanged(const QString&)));
    connect(ui.useDefaultAudioSettings, SIGNAL(toggled(bool)), this, SLOT(useDefaultChanged(bool)));
    connect(ui.codecComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(codecChanged(int)));
    connect(ui.sampleRateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sampleRateChanged(int)));
    connect(ui.channelsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(channelsChanged(int)));
    connect(ui.algorithmicQualitySlider, SIGNAL(valueChanged(int)), this, SLOT(algoQualityChanged(int)));
    connect(ui.vbrQualitySlider, SIGNAL(valueChanged(int)), this, SLOT(vbrQualityChanged(int)));

    itemModel = new QStandardItemModel;
    ui.itemList->setModel(itemModel);

    connect(ui.itemList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(selectionChanged(const QModelIndex&, const QModelIndex&)));
    connect(itemModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(itemChanged(QStandardItem*)));
}

SwfMp3Looper::~SwfMp3Looper()
{
    delete itemModel;
}

void SwfMp3Looper::setControlsEnabled(bool enabled)
{
    ui.labelSourceFile->setEnabled(enabled);
    ui.labelClassName->setEnabled(enabled);
    ui.labelCodec->setEnabled(enabled);

    ui.classNameLineEdit->setEnabled(enabled);
    ui.useDefaultAudioSettings->setEnabled(enabled);
    ui.codecComboBox->setEnabled(enabled);
    ui.sampleRateComboBox->setEnabled(enabled);
    ui.channelsComboBox->setEnabled(enabled);
    ui.audioSettingsGroupBox->setEnabled(!ui.useDefaultAudioSettings->isChecked());
    if (ui.codecComboBox->currentIndex() == 0) // MP3
    {
        ui.algorithmQualityGroup->setEnabled(enabled);
        ui.vbrQualityGroup->setEnabled(enabled);
    }
    else // WAV
    {
        ui.algorithmQualityGroup->setEnabled(false);
        ui.vbrQualityGroup->setEnabled(false);
    }
}

void SwfMp3Looper::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void SwfMp3Looper::dropEvent(QDropEvent* event)
{
    event->accept();
}

void SwfMp3Looper::addFile()
{
    //QString path = QFileDialog::getOpenFileName(this, "", "", "Audio files (*.*)");
    QStringList paths = QFileDialog::getOpenFileNames(this, "", "", "Audio files (*.*)");
    if (!paths.isEmpty())
    {
        for (QString path : paths)
        {
            QList<QStandardItem*> options;
            options.append(new QStandardItem(path)); // File name
            options.append(new QStandardItem("Symbol")); // Symbol class name
            options.append(new QStandardItem); options.last()->setData(true); // Use default?
            options.append(new QStandardItem); options.last()->setData(0); // Codec index
            options.append(new QStandardItem); options.last()->setData(2); // Sample rate index
            options.append(new QStandardItem); options.last()->setData(0); // Channel index
            options.append(new QStandardItem); options.last()->setData(5); // Algorithm quality setting
            options.append(new QStandardItem); options.last()->setData(4); // VBR quality setting

            itemModel->appendRow(options);
        }
        ui.itemList->setCurrentIndex(itemModel->index(itemModel->rowCount()-1,0));

        ui.removeButton->setEnabled(true);
        ui.saveAsButton->setEnabled(true);
        ui.progressBar->setValue(0);
    }
}

void SwfMp3Looper::removeFile()
{
    QModelIndex index = ui.itemList->selectionModel()->currentIndex();
    if (index.isValid())
    {
        itemModel->removeRows(index.row(), 1);
    }

    ui.saveAsButton->setEnabled(itemModel->rowCount() > 0);
    ui.progressBar->setValue(0);
}

void SwfMp3Looper::selectionChanged(const QModelIndex &current, const QModelIndex &/*prev*/)
{
    if (!current.isValid())
    {
        ui.removeButton->setEnabled(false);
        setControlsEnabled(false);
    }
    else
    {
        int row = current.row();

        ui.sourceFileLineEdit->setText(itemModel->item(row, UI::FILENAME)->text());
        ui.classNameLineEdit->setText(itemModel->item(row, UI::CLASSNAME)->text());
        ui.useDefaultAudioSettings->setChecked(itemModel->item(row, UI::DEFAULT_SETTING)->data().toBool());
        ui.codecComboBox->setCurrentIndex(itemModel->item(row, UI::CODEC)->data().toInt());
        ui.sampleRateComboBox->setCurrentIndex(itemModel->item(row, UI::SAMPLE_RATE)->data().toInt());
        ui.channelsComboBox->setCurrentIndex(itemModel->item(row, UI::CHANNELS)->data().toInt());
        ui.algorithmicQualitySlider->setValue(itemModel->item(row, UI::ALGO_QUALITY)->data().toInt());
        ui.vbrQualitySlider->setValue(itemModel->item(row, UI::VBR_QUALITY)->data().toInt());

        setControlsEnabled(true);
    }
}

void SwfMp3Looper::saveAs()
{
    std::string path = QFileDialog::getSaveFileName(this, "", "", "SWF file (*.swf)").toStdString();

    if (!path.empty())
    {
        ui.cancelButton->setEnabled(true);
        cancelEncode = false;

        try
        {
            Swf swf(path);

            int rows = itemModel->rowCount();
            float totalProgress = 0.f;
            for (int row=0; row<rows && !cancelEncode; ++row)
            {
                // Default values
                const int DEFAULT_ALGO_QUALITY = 5;
                const int DEFAULT_VBR_QUALITY = 4;
                const int DEFAULT_SAMPLE_RATE = 44100;
                const int DEFAULT_CHANNEL_COUNT = -1;
                const AVCodecID DEFAULT_CODEC = AV_CODEC_ID_MP3;

                std::string source = itemModel->item(row, UI::FILENAME)->text().toStdString();
                std::string className = itemModel->item(row, UI::CLASSNAME)->text().toStdString();

                bool useDefaults = itemModel->item(row, UI::DEFAULT_SETTING)->data().toBool();
                int vbrQuality = useDefaults ? DEFAULT_VBR_QUALITY :
                                               itemModel->item(row, UI::VBR_QUALITY)->data().toInt();
                int audioQuality = useDefaults ? DEFAULT_ALGO_QUALITY :
                                                 itemModel->item(row, UI::ALGO_QUALITY)->data().toInt();

                int sampleRateIndex = itemModel->item(row, UI::SAMPLE_RATE)->data().toInt();
                int sampleRate = useDefaults ? DEFAULT_SAMPLE_RATE :
                                 sampleRateIndex == 0 ? 11025 :
                                 sampleRateIndex == 1 ? 22050 :
                                                        44100;

                int channelCountIndex = itemModel->item(row, UI::CHANNELS)->data().toInt();
                int channelCount = useDefaults ? DEFAULT_CHANNEL_COUNT :
                                   channelCountIndex == 0 ? -1 : // Same as source
                                   channelCountIndex == 1 ?  2 : // Stereo
                                                             1;  // Mono
                AVCodecID codecId = useDefaults ? DEFAULT_CODEC :
                                    itemModel->item(row, UI::CODEC)->data().toInt() == 0 ? AV_CODEC_ID_MP3 :
                                                                                           AV_CODEC_ID_PCM_S16LE;

                AudioDecoder decoder(source);

                if (channelCount < 0) // Same as source
                {
                    channelCount = decoder.getChannelCount();
                }

                AudioEncoder encoder(codecId, channelCount, sampleRate, audioQuality, vbrQuality);

                std::function<bool(float)> callback = [this, &rows, &totalProgress](float t) -> bool {
                    totalProgress += t;
                    ui.progressBar->setValue((int)(totalProgress / rows * 99)); // 0-99% for transcode, 100% for file save
                    QApplication::processEvents();
                    return !cancelEncode;
                };

                swf.addSound(decoder, encoder, className, callback);
            }

            swf.close();
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

void SwfMp3Looper::itemChanged(QStandardItem *item)
{
    int col = itemModel->indexFromItem(item).column();
    if (col == UI::FILENAME)
    {
        ui.sourceFileLineEdit->setText(item->text());
    }
}

void SwfMp3Looper::classnameChanged(const QString& text)
{
    itemModel->item(ui.itemList->currentIndex().row(), UI::CLASSNAME)->setText(text);
}

void SwfMp3Looper::useDefaultChanged(bool checked)
{
    itemModel->item(ui.itemList->currentIndex().row(), UI::DEFAULT_SETTING)->setData(checked);
    ui.audioSettingsGroupBox->setEnabled(!checked);
}

void SwfMp3Looper::codecChanged(int index)
{
    itemModel->item(ui.itemList->currentIndex().row(), UI::CODEC)->setData(index);
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

void SwfMp3Looper::sampleRateChanged(int index)
{
    itemModel->item(ui.itemList->currentIndex().row(), UI::SAMPLE_RATE)->setData(index);
}

void SwfMp3Looper::channelsChanged(int index)
{
    itemModel->item(ui.itemList->currentIndex().row(), UI::CHANNELS)->setData(index);
}

void SwfMp3Looper::algoQualityChanged(int value)
{
    itemModel->item(ui.itemList->currentIndex().row(), UI::ALGO_QUALITY)->setData(value);
}

void SwfMp3Looper::vbrQualityChanged(int value)
{
    itemModel->item(ui.itemList->currentIndex().row(), UI::VBR_QUALITY)->setData(value);
}
