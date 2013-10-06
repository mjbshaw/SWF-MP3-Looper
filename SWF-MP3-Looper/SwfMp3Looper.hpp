#ifndef SWF_MP3_LOOPER_HPP
#define SWF_MP3_LOOPER_HPP

#include <QDialog>
#include "ui_SwfMp3Looper.h"

class QStandardItem;
class QStandardItemModel;

class SwfMp3Looper : public QDialog
{
    Q_OBJECT

public:
    SwfMp3Looper(QWidget *parent = nullptr);
    ~SwfMp3Looper();

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    Ui::SwfMp3Looper ui;
    bool cancelEncode;
    QStandardItemModel* itemModel;

    void setControlsEnabled(bool enabled=true);

private slots:
    void addFile();
    void removeFile();
    void selectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void saveAs();
    void cancel();

    void itemChanged(QStandardItem *item);
    void classnameChanged(const QString &text);
    void useDefaultChanged(bool checked);
    void codecChanged(int index);
    void sampleRateChanged(int index);
    void channelsChanged(int index);
    void algoQualityChanged(int value);
    void vbrQualityChanged(int value);
};

#endif
