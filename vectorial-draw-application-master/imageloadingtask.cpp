#include "imageloadingtask.h"
#include "QBuffer"

ImageLoadingTask::ImageLoadingTask(QString fileName, QString fullpath, QObject *parent) : QObject(parent)
{
    this->fileName = fileName;
    this->fullpath = fullpath;
}

void ImageLoadingTask::run()
{
    QFile fl(this->fullpath);
    if (fl.open(QIODevice::ReadOnly)) {
        QByteArray bytes = fl.readAll();

        QImage img;
        img= img.fromData(bytes);
        img= img.scaled(130, 130, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        emit finished(this->fileName,this->fullpath, img, bytes);
    } else {
        emit errorFound(fl.errorString());
    }
}
