#ifndef QRUNNABLEFORIMAGE_H
#define QRUNNABLEFORIMAGE_H

#include <QObject>
#include "QRunnable"
#include "QFile"
#include "QImage"
#include "QDebug"

class ImageLoadingTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ImageLoadingTask(QString fileName, QString fullpath, QObject *parent = nullptr);

    ~ImageLoadingTask() {
        qDebug() << "qrunnableforimage deleted";
    }

signals:
    void finished(QString filename, QString fullpath, QImage img, QByteArray bytes);
    void errorFound(QString what);

public slots:
    void run();

private:
    QString fileName;
    QString fullpath;
};

#endif // QRUNNABLEFORIMAGE_H
