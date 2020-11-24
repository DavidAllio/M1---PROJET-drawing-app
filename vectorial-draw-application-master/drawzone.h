#ifndef DRAWZONE_H
#define DRAWZONE_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QtGui>
#include <QWidget>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include "mainwindow.h"
#include <QUndoView>
#include <QUndoCommand>

class drawZone : public QGraphicsView
{
    Q_OBJECT
public:
    explicit drawZone(QWidget *parent = 0);
    ~drawZone() override;
    static int getWidth();
    static int getHeight();
    void setWidht(int w);
    void setHeight(int h);
    void setactualTool(Tool);
    void setactualColor(QColor);
    void setactualColor2(QColor);
    void setactualSize(int);
    void setactualtextSize(int);
    void setactualtextFont(QFont);
    void setactualtextContent(QString);
    void clearScene();
    void saveScene();
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    bool saveAsFile();
    void addpicture(QPixmap pixmap);
    QGraphicsItem* getActItem();
    QGraphicsScene *getScene();
    void selectNothing();
    QUndoStack* getactualStack();
    QUndoCommand* AddShapeCommand(QGraphicsItem *item);
    QUndoStack *undoStack;
    //QUndoView *undoView;
    QPointF initialPosition;
    void undo();
    void redo();

protected:
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    void keyPressEvent( QKeyEvent*) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void showcontextmenu();

private slots:
    void deleteItem();
    void DuplicateItem();

private :
    QGraphicsScene *scene;
    QPointF origPoint;
    Tool actualTool;
    QColor actualColor;
    QColor actualColor2;
    int actualSize;
    int actualtextSize;
    QFont actualtextFont;
    QString actualtextContent;
    int PointActuel=0;
    int count=0;
    bool first=true;
    QPointF point_init;
    QPainter *painter;
    QMenu *menu;
    QAction *qduplicate, *qdelete;


    //Pour le dessin à main levée
    QGraphicsPathItem *pathitem=nullptr;
    QVector<QPainterPath> paths;
    QPainterPath *path=new QPainterPath();
    QGraphicsEllipseItem *previewPoint=nullptr;

    //Pour le dessin polygonal
     QGraphicsPolygonItem *polygon=nullptr;
     QPolygonF *poly=new QPolygonF();
     QGraphicsEllipseItem *actualPoint=nullptr;
     QVector<QPolygonF> polygons;

    //Pour la selection
     QGraphicsItem *SelItem=nullptr;
     QGraphicsPathItem* pathItem ;
     QGraphicsLineItem* lineItem ;
     QGraphicsPolygonItem* polygonItem ;
     QGraphicsRectItem* rectItem;
     QGraphicsEllipseItem* elliItem ;
     QGraphicsTextItem* textItem;

     //pour le deplacement
     double initX,initY;

     //pour la rotation
     bool centerPointSet=false;
     bool doRotate=false;
     int angle=0;

     //Pour les previews des différentes formes
     QGraphicsEllipseItem *previewcircle=nullptr;
     QGraphicsLineItem* previewline=nullptr ;
     QGraphicsPolygonItem* previewtriangle=nullptr ;
     QGraphicsRectItem* previewrectangle=nullptr;
     QGraphicsTextItem* previewtext=nullptr;
     QPolygonF tripoly;
     QPointF trianglep1,trianglep2,trianglep3;
     bool pt1=false,pt2=false,pt3=false;

     QPointF PreviousPoint;
     QGraphicsEllipseItem *tmp_ellipse=nullptr;
     QList<QGraphicsItem*> RemovedItems;





signals:
    void actualToolShowProperty(Tool);
    void setStrokeColor(QColor);
    void setFillColor(QColor);
    void setStrokeSize(int);
    void setTextFont(QFont);
    void setTextContent(QString);


};

#endif // DRAWZONE_H
