#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QGraphicsItem>
#include <QFileSystemModel>
#include <QListWidgetItem>
#include <QUndoView>
#include <QUndoCommand>
#include "propiete.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Point
{
   int x, y;
};

enum Tool { CURSOR, LINE, RECTANGLE, CIRCLE, TRIANGLE, TEXT, IMAGE, POLYGON, FREE };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    static void setCursorLabelCoord(QMouseEvent*);
    static void leaveDrawZone();
    void showStatusMessage(const QString &msg);
    void clearFile(bool);
    QUndoStack *undoStack;

private slots:
    void newFile();
    void openFile();
    bool save();
    bool saveAs();
    void actualToolChangeProperty(Tool);
    void changeStrokeColor(QColor);
    void changeFillColor(QColor);
    void changeStrokeSize(int);
    void changeTextFont(QFont);
    void changeTextContent(QString);
    void ImageWidgetOpened(bool);

    void dockWidgetInit();
    void propertyWidgetInit();
    void drawZoneSignalInit();
    void menuInit();
    void sharedPropertyInit();

    void shapeToolSelected();
    void lineToolSelected();
    void textToolSelected();
    void noPropertyToolSelected();
    void imageToolSelected(bool);

    void helpButtonClicked();
    void toolButtonClicked();
    void propertyButtonClicked();
    void undoButtonClicked();
    void redoButtonClicked();
    void ActionListClicked();

    void on_fillColorButton_clicked();
    void on_strokeColorButton2_clicked();
    void on_squareButton_clicked();
    void on_circleButton_clicked();
    void on_triangleButton_clicked();
    void on_pictureButton_clicked();
    void on_actionZoomPlus_triggered();
    void on_actionZoomMoins_triggered();
    void on_actionExporter_triggered();
    //void on_actionImporter_triggered();



    void on_cursorButton_clicked();
    void on_freeDrawButton_clicked();
    void on_pointButton_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void on_lineButton_clicked();
    void lineChecked(bool checked);

    void on_spinBox_valueChanged(int arg1);

    void on_fontComboBox_currentFontChanged(const QFont &f);

    void on_textEdit_textChanged();

    void setDrawzoneSize(int width, int height);

    void on_treeFolder_clicked(const QModelIndex &index);
    void imageLoaded(QString , QString , QImage , QByteArray );

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;

    void initStatusBar();
    bool saveFile(const QString &filename);
    void setCurrentFile(const QString &fileName);
    bool loadImportedFile(const QString &filename);
    bool saveExportFile(const QString &fileName);
    bool loadFile(const QString &fileName);

    bool isSaved=false;

    QString currentFile;

    int width;
    int height;

    //Pour la sauvegarde
    QGraphicsPathItem* pathItem ;
    QGraphicsLineItem* lineItem ;
    QGraphicsPolygonItem* polygonItem ;
    QGraphicsRectItem* rectItem;
    QGraphicsEllipseItem* elliItem ;
    QGraphicsTextItem* textItem;
    QGraphicsPixmapItem* imgItem;

    //Pour Undo/Redo
    void createUndoView();
    QUndoView *undoView;

    //Pour l'insertion d'images
    QFileSystemModel *folderModel;
    QFileSystemModel *fileModel;
    QMap<QString, QListWidgetItem*> m_mapFileNameListWidgetItem;
    QString mPath;
};
#endif // MAINWINDOW_H
