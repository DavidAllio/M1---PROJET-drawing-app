#include <math.h>
#include <QDebug>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QApplication>
#include <QColorDialog>
#include <QGraphicsDropShadowEffect>
#include "mainwindow.h"
#include "drawzone.h"
#include <QUndoView>
#include <QUndoCommand>
static int largeur;
static int hauteur;

drawZone::drawZone(QWidget *parent) :
    QGraphicsView(parent)
{
    qDebug() << "init drawZone" ;
    setMouseTracking(true);
    setAcceptDrops(true);
    scene = new QGraphicsScene(this);
    setScene(scene);

    focusPolicy();
    undoStack = new QUndoStack(this);
    QList<QGraphicsItem*> graphicsItemList(scene->items());
    qduplicate=new QAction("Dupliquer", this);
    qdelete=new QAction("Supprimer", this);
    menu=new QMenu(this);
    menu->addAction(qduplicate);
    menu->addAction(qdelete);
    connect(qduplicate, SIGNAL(triggered()),this,SLOT(DuplicateItem()));
    connect(qdelete, SIGNAL(triggered()),this,SLOT(deleteItem()));

/*
   QBrush greenBrush(Qt::green);
   QBrush blueBrush(Qt::blue);
   QPen outlinePen(Qt::black);
   outlinePen.setWidth(2);

   rectangle = scene->addRect(100, 0, 80, 100, outlinePen, blueBrush);
   rectangle->setAcceptHoverEvents( true );
   rectangle->setFlag(QGraphicsItem::ItemIsMovable);
   rectangle->setFlag(QGraphicsItem::ItemIsSelectable);
   //(x,y,w,h,pen,brush)
   ellipse = scene->addEllipse(0, -100, 300, 60, outlinePen, greenBrush);

   text = scene->addText("Test", QFont("Arial", 20) );
   */
}

QGraphicsScene* drawZone::getScene(){
    return scene;
}

QUndoStack* drawZone::getactualStack(){
    return undoStack;
}

QString createCommandString(const QPointF &pos)
{
    return QObject::tr("%1 at (%2, %3)")
        .arg(pos.x()).arg(pos.y());
}

QUndoCommand* drawZone::AddShapeCommand(QGraphicsItem *item)
{
    QUndoCommand* Command = new QUndoCommand(); // an empty command
    QString text;

    RemovedItems.clear();
    //Pour détecter le type de l'item sélectionné et afficher les caractéristiques correspondantes
    pathItem = dynamic_cast<QGraphicsPathItem*>(item);
    lineItem = dynamic_cast<QGraphicsLineItem*>(item);
    polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item);
    rectItem = dynamic_cast<QGraphicsRectItem*>(item);
    elliItem = dynamic_cast<QGraphicsEllipseItem*>(item);
    textItem = dynamic_cast<QGraphicsTextItem*>(item);

    if(pathItem){text.append("Path ");}
    if(lineItem){text.append("Ligne ");}
    if(polygonItem){text.append("Polygone ");}
    if(rectItem){text.append("Rectangle ");}
    if(elliItem){text.append("Ellipse ");}
    if(textItem){text.append("Texte ");}

    text.append("ajouté en ");
    text.append(QString::number(item->sceneBoundingRect().x()));
    text.append(" , ");
    text.append(QString::number(item->sceneBoundingRect().y()));
    Command->setText(text);

    undoStack->push(Command);
}

void drawZone::undo()
{
    QList<QGraphicsItem*> graphicsItemList(scene->items());
    if(!graphicsItemList.isEmpty()){
        RemovedItems.prepend(graphicsItemList.first());
        scene->removeItem(graphicsItemList.first());
    }
    undoStack->undo();
    scene->update();
}

void drawZone::redo()
{
    if(!RemovedItems.isEmpty()){
        scene->addItem(RemovedItems.takeFirst());
    }
    undoStack->redo();
    scene->update();
}

void drawZone::setactualTool(Tool tool){
    setCursor(Qt::ArrowCursor);
    //Test sur l'outil précedemment sélectionné
    if(actualTool==POLYGON){
        polygons.append(*poly);
        polygon=nullptr;
        if(actualPoint)
            scene->removeItem(actualPoint);
        actualPoint=nullptr;
        poly= new QPolygonF();

    }
    if(actualTool==FREE){
        if(previewPoint)
            scene->removeItem(previewPoint);
        previewPoint=nullptr;
        if(SelItem!=nullptr)
            SelItem->setSelected(false);
        SelItem=nullptr;
    }

    actualTool=tool;

    //Test sur l'outil actuellement sélectionné

    if(tool==FREE){
        setCursor(Qt::BlankCursor);
    }
}

void drawZone::setWidht(int w){

    largeur=w;
}

void drawZone::setHeight(int h){

    hauteur=h;
}

int drawZone::getWidth(){
    return largeur;

}

int drawZone::getHeight(){
    return hauteur;

}

void drawZone::setactualSize(int value){
    actualSize=value;
    if(actualTool==CURSOR&&SelItem!=nullptr){
        if(pathItem)
            pathItem->setPen(QPen(pathItem->pen().color(),actualSize));
        if(lineItem)
            lineItem->setPen(QPen(lineItem->pen().color(),actualSize));
        if(polygonItem)
            polygonItem->setPen(QPen(polygonItem->pen().color(),actualSize));
        if(rectItem)
            rectItem->setPen(QPen(rectItem->pen().color(),actualSize));
        if(elliItem)
            elliItem->setPen(QPen(elliItem->pen().color(),actualSize));
    }
}

void drawZone::setactualtextSize(int value){
    actualtextSize=value;
    if(actualTool==CURSOR&&textItem){
        QFont tfont = textItem->font();
        tfont.setPointSize(value);
        textItem->setFont(tfont);
    }

}

void drawZone::setactualtextFont(QFont font){
    actualtextFont=font;
    if(actualTool==CURSOR&&textItem){
        font.setPointSize(actualtextSize);
        textItem->setFont(font);
    }
}

void drawZone::setactualtextContent(QString text){
    actualtextContent=text;
    if(actualTool==CURSOR&&textItem){
        textItem->setPlainText(text);
    }
}

void drawZone::setactualColor(QColor color){
    actualColor=color;
    //Si l'outil sélection est activé et qu'un item est sélectionné
    if(actualTool==CURSOR&&SelItem!=nullptr){
        if(pathItem)
            pathItem->setPen(QPen(actualColor,pathItem->pen().width()));
        if(lineItem)
            lineItem->setPen(QPen(actualColor,lineItem->pen().width()));
        if(polygonItem)
            polygonItem->setPen(QPen(actualColor,polygonItem->pen().width()));
        if(rectItem)
            rectItem->setPen(QPen(actualColor,rectItem->pen().width()));
        if(elliItem)
            elliItem->setPen(QPen(actualColor,elliItem->pen().width()));
    }
}

void drawZone::setactualColor2(QColor color){
    actualColor2=color;
    if(actualTool==CURSOR&&SelItem!=nullptr){
        if(polygonItem)
            polygonItem->setBrush(QBrush(actualColor2));
        if(rectItem)
            rectItem->setBrush(QBrush(actualColor2));
        if(elliItem)
            elliItem->setBrush(QBrush(actualColor2));
    }
}

void drawZone::mouseMoveEvent(QMouseEvent *ev)
{
    QPointF point = mapToScene(ev->pos());
    MainWindow::setCursorLabelCoord(ev);

    if(actualTool==FREE){
        if(ev->buttons().testFlag(Qt::LeftButton)){
            if(count%4==0){
                path->lineTo(point);
                /*
                On efface le précédent tracé puisqu'un nouveau point a été rajouté
                */
                if(pathitem!=nullptr){
                    scene->removeItem(pathitem);
                }
                pathitem=scene->addPath(*path,QPen(actualColor, actualSize, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                pathitem->setFlag(QGraphicsItem::ItemIsSelectable);
            }
            count++;
        }
        if(previewPoint!=nullptr)
            scene->removeItem(previewPoint);
        previewPoint=scene->addEllipse(point.x()-actualSize/2,point.y()-actualSize/2,actualSize,actualSize, QPen(QColor(Qt::black),2), QColor(Qt::transparent));
    }
    if(ev->buttons().testFlag(Qt::LeftButton)){
        if(actualTool==CIRCLE){

                if(previewcircle!=nullptr)
                    scene->removeItem(previewcircle);
                previewcircle = scene->addEllipse(PreviousPoint.x(),PreviousPoint.y(),abs(point.x()-PreviousPoint.x()),abs(point.y()-PreviousPoint.y()),actualColor, actualColor2);

        }

        if(actualTool==RECTANGLE){
                if(previewrectangle!=nullptr)
                    scene->removeItem(previewrectangle);

                previewrectangle = scene->addRect(PreviousPoint.x(),PreviousPoint.y(),abs(point.x()-PreviousPoint.x()),abs(point.y()-PreviousPoint.y()),actualColor, actualColor2);

        }
    }

    if(actualTool==LINE){
        if(previewcircle!=nullptr)
            scene->removeItem(previewcircle);
        previewcircle = scene->addEllipse(point.x()-(actualSize/2),point.y()-(actualSize/2),actualSize,actualSize,actualColor, actualColor);

        if(PointActuel==1){
            if(previewline!=nullptr)
                scene->removeItem(previewline);
            QPen pen;  // creates a default pen
            pen.setStyle(Qt::SolidLine);
            pen.setWidth(actualSize);
            pen.setBrush(actualColor);
            pen.setCapStyle(Qt::RoundCap);
            previewline = scene->addLine(PreviousPoint.x(),PreviousPoint.y(),point.x(),point.y(),pen);
        }
    }

    if(actualTool==TRIANGLE){
        if(previewtriangle!=nullptr)
            scene->removeItem(previewtriangle);

        tripoly.clear();
        if(!pt1){
            trianglep1=point;
            tripoly.append(trianglep1);
            tripoly.append(trianglep1);
            tripoly.append(trianglep1);
        }else{
            if(!pt2){
                trianglep2=point;
                tripoly.append(trianglep1);
                tripoly.append(trianglep2);
                tripoly.append(trianglep2);
            }else{
                trianglep3=point;
                tripoly.append(trianglep1);
                tripoly.append(trianglep2);
                tripoly.append(trianglep3);
            }
        }

        previewtriangle = scene->addPolygon(tripoly,actualColor, actualColor2);
    }

    if(actualTool==TEXT){
        if(previewtext!=nullptr)
            scene->removeItem(previewtext);
        actualtextFont.setPointSize(actualtextSize);
        previewtext = scene->addText(actualtextContent, actualtextFont);
        previewtext->setPos(point);

    }

    if(actualTool==CURSOR){

        if(SelItem){
            if(ev->buttons().testFlag(Qt::LeftButton)){
                SelItem->setPos(point.x()-initX,point.y()-initY);

            }
            if(doRotate){
                double a1=0,a2=0;
                if(textItem){

                    a1 = textItem->scenePos().x()-point.x();
                    a2 = textItem->scenePos().y()-point.y();
                    /*
                    if(qr)
                        scene->removeItem(qr);
                    qr=scene->addEllipse(a1,a2,100,100,QPen(Qt::red),QBrush());
                    */
                }else{
                    a1 = SelItem->boundingRect().center().x()-point.x();
                    a2 = SelItem->boundingRect().center().y()-point.y();
                }

                SelItem-> setRotation(atan2(a2,a1) * 180/M_PI);
            }
        }
    }

}

void drawZone::keyPressEvent( QKeyEvent *ev){
    if(SelItem){
        if(ev->key() == Qt::Key_R ){
            if(!centerPointSet){
                SelItem->setTransformOriginPoint(SelItem->boundingRect().center());
                centerPointSet=true;
            }
            doRotate=!doRotate;
            qDebug()<<"rotation activée";
        }

        if(ev->key() == Qt::Key_Delete ){
            deleteItem();
        }

        if(ev->key() == Qt::Key_D)
            DuplicateItem();

    }

}

void drawZone::addpicture(QPixmap pixmap)
{
        scene->addPixmap(pixmap);

}

bool drawZone::saveFile(const QString &fileName){
    qDebug() << __FUNCTION__ <<"sauvegarde";

    QPixmap *tosave = new QPixmap(scene->sceneRect().width(),scene->sceneRect().height());
    QPainter painter(tosave);
    scene->setBackgroundBrush(Qt::white);
    scene->render(&painter);

    painter.end();


    if (!fileName.isEmpty())
    {
      tosave->save(fileName);
      delete tosave;
      return true;
    }
    delete tosave;
    return false;
}

void drawZone::mouseReleaseEvent(QMouseEvent *event){
    if(actualTool==FREE){
        //pathitem->shape().swap(*path);
        pathitem=nullptr;
        paths.append(*path);
        path=new QPainterPath();
        scene->removeItem(previewPoint);
        previewPoint=nullptr;
    }

    if(actualTool==RECTANGLE){
        if(previewrectangle){
            previewrectangle->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable|QGraphicsTextItem::ItemIsFocusable);
            AddShapeCommand(previewrectangle);
            previewrectangle=nullptr;
        }
    }

    if(actualTool==CIRCLE){
        if(previewcircle){
            previewcircle->setFlag(QGraphicsEllipseItem::ItemIsSelectable);
            previewcircle->setFlag(QGraphicsEllipseItem::ItemIsMovable);
            AddShapeCommand(previewcircle);
            previewcircle=nullptr;
        }
    }
}

void drawZone::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void drawZone::dropEvent(QDropEvent *event){
    QList<QUrl> url = event->mimeData()->urls();
    QGraphicsPixmapItem *qpi = scene->addPixmap(url.first().toString());
    qpi->setFlag(QGraphicsPixmapItem::ItemIsSelectable);
    qpi->setFlag(QGraphicsPixmapItem::ItemIsMovable);
    qpi->setFlag(QGraphicsPixmapItem::ItemIsFocusable);
    qpi->setPos(event->pos());
    event->acceptProposedAction();

}

void drawZone::leaveEvent(QEvent * e)
{
    if(previewPoint)
        scene->removeItem(previewPoint);
    previewPoint=nullptr;

    if(previewcircle)
        scene->removeItem(previewcircle);
    previewcircle=nullptr;

    if(previewline)
        scene->removeItem(previewline);
    previewline=nullptr;

    if(previewtriangle)
        scene->removeItem(previewtriangle);
    previewtriangle=nullptr;

    if(previewtext)
        scene->removeItem(previewtext);
    previewtext=nullptr;

    if(previewrectangle)
        scene->removeItem(previewrectangle);
    previewrectangle=nullptr;

    MainWindow::leaveDrawZone();
}

void drawZone::clearScene()
{

    scene->clear();
}


void drawZone::mousePressEvent(QMouseEvent *ev)
{
    switch(ev->button()) {
    case Qt::RightButton :
        showcontextmenu();
        break;

    case Qt::LeftButton :
    {
        int x,y;
        x = ev->x();
        y = ev->y();
        qDebug() << x << ", " << y;
        QPointF point = mapToScene(x, y);

        switch(actualTool){
            case(CIRCLE):
            {
                PreviousPoint =point;

                break;
            }
            case(LINE):
            {
                if(PointActuel==0){
                    //QGraphicsEllipseItem *ellipse;
                    tmp_ellipse = scene->addEllipse(point.x()-(actualSize/2),point.y()-(actualSize/2),actualSize,actualSize,actualColor, actualColor);
                    tmp_ellipse->setFlag(QGraphicsEllipseItem::ItemIsMovable);
                    PreviousPoint=point;
                    PointActuel=1;
                    return;
                }

                if(PointActuel==1){
                    scene->removeItem(tmp_ellipse);
                    QGraphicsLineItem *line;
                    QPen pen;  // creates a default pen

                    pen.setStyle(Qt::SolidLine);
                    pen.setWidth(actualSize);
                    pen.setBrush(actualColor);
                    pen.setCapStyle(Qt::RoundCap);

                    line = scene->addLine(PreviousPoint.x(),PreviousPoint.y(),point.x(),point.y(),pen);

                    line->setFlag(QGraphicsLineItem::ItemIsSelectable);
                    line->setFlag(QGraphicsLineItem::ItemIsMovable);
                    PointActuel=0;

                    AddShapeCommand(line);
                    return;
                }
                break;
            }
            case(FREE):
                //points.append(point);
                path->moveTo(point);
                break;
            case(POLYGON):
            {
                if(polygon!=nullptr){
                    scene->removeItem(polygon);
                    scene->removeItem(actualPoint);
                }
                poly->append(point);
                //polygon->setPolygon(*poly);
                polygon=scene->addPolygon(*poly,QPen(actualColor,actualSize,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),actualColor2);
                actualPoint=scene->addEllipse(point.x()-(actualSize+5)/2,point.y()-(actualSize+5)/2,actualSize+10,actualSize+10, QColor(Qt::red), QColor(Qt::red));
                polygon->setFlag(QGraphicsLineItem::ItemIsSelectable);
                polygon->setFlag(QGraphicsLineItem::ItemIsMovable);
                AddShapeCommand(polygon);
                break;
            }

            case(RECTANGLE):
            {
                //rectangle = scene->addRect(point.x()-(actualSize/2),point.y()-(actualSize/2),actualSize,actualSize,actualColor, actualColor2);
                PreviousPoint = point;

                break;
            }

            case(TRIANGLE):
            {
                if(!pt1){
                    trianglep1=point;
                    pt1=true;
                }else{
                    if(!pt2){
                        trianglep2=point;
                        pt2=true;
                    }else{
                        trianglep3=point;
                        pt1=false;
                        pt2=false;
                        previewtriangle->setFlag(QGraphicsLineItem::ItemIsSelectable);
                        previewtriangle->setFlag(QGraphicsLineItem::ItemIsMovable);
                        AddShapeCommand(previewtriangle);
                        previewtriangle=nullptr;
                    }
                }

                break;
            }

            case(TEXT):
            {
                QGraphicsTextItem *text;
                actualtextFont.setPointSize(actualtextSize);
                text = scene->addText(actualtextContent, actualtextFont);
                text->setPos(point);
                text->setFlag(QGraphicsTextItem::ItemIsSelectable);
                text->setFlag(QGraphicsTextItem::ItemIsMovable);
                text->setFlag(QGraphicsTextItem::ItemIsFocusable);
                AddShapeCommand(text);
                break;
            }
            case(CURSOR):
            {
                bool somethingSelected=false;

                foreach (QGraphicsItem *item, scene->items()) {
                    if(item->contains(item->mapFromScene(QPointF(point.x(),point.y())))&&item->zValue()!=-1000){

                        somethingSelected=true;
                        doRotate=false;
                         if(SelItem!=nullptr){
                             centerPointSet=false;
                             SelItem->setSelected(false);
                             qDebug()<<"Changement d'objet sélectionné";
                         }
                         SelItem=item; 
                         //item->setGraphicsEffect(effect);
                         SelItem->setSelected(true);
                         initX=point.x()-item->x();
                         initY=point.y()-item->y();


                         //Pour détecter le type de l'item sélectionné et afficher les caractéristiques correspondantes
                         pathItem = dynamic_cast<QGraphicsPathItem*>(item);
                         lineItem = dynamic_cast<QGraphicsLineItem*>(item);

                         //affiche les caractéristiques de l'outil sélectionné
                         if(pathItem||lineItem)
                             emit actualToolShowProperty(LINE);

                         polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item);
                         rectItem = dynamic_cast<QGraphicsRectItem*>(item);
                         elliItem = dynamic_cast<QGraphicsEllipseItem*>(item);
                         if(polygonItem||rectItem||elliItem)
                             emit actualToolShowProperty(RECTANGLE);

                         textItem = dynamic_cast<QGraphicsTextItem*>(item);

                         if(textItem){
                             emit actualToolShowProperty(TEXT);
                             emit setTextFont(textItem->font());
                             emit setTextContent(textItem->toPlainText());
                         }

                         if(pathItem){
                             emit  setStrokeColor(pathItem->pen().color());
                             emit  setStrokeSize(pathItem->pen().width());
                         }
                         if(lineItem){
                             emit  setStrokeColor(lineItem->pen().color());
                             emit  setStrokeSize(lineItem->pen().width());
                         }

                         if(polygonItem){
                             emit  setStrokeColor(polygonItem->pen().color());
                             emit  setStrokeSize(polygonItem->pen().width());
                             emit  setFillColor(polygonItem->brush().color());
                         }

                         if(rectItem){
                             emit  setStrokeColor(rectItem->pen().color());
                             emit  setStrokeSize(rectItem->pen().width());
                             emit  setFillColor(rectItem->brush().color());
                         }

                         if(elliItem){
                             emit  setStrokeColor(elliItem->pen().color());
                             emit  setStrokeSize(elliItem->pen().width());
                             emit  setFillColor(elliItem->brush().color());
                         }
                        break;
                     }
                }

                if(!somethingSelected)
                    if(SelItem!=nullptr){
                        SelItem->setSelected(false);
                        SelItem=nullptr;
                        doRotate=false;
                        centerPointSet=false;
                        emit actualToolShowProperty(CURSOR);
                    }
                break;
            }
            default:
            break;
        }
    break;
    }
    default:
        break;
    }
}

void drawZone::showcontextmenu()
{
    menu->exec(QCursor::pos());
}

void drawZone::selectNothing(){
    SelItem=nullptr;
    centerPointSet=false;
}

void drawZone::deleteItem()
{
    if(!SelItem)
        return;
    scene->removeItem(SelItem);
    delete SelItem;
    SelItem=nullptr;

}

void drawZone::DuplicateItem()
{
  QGraphicsItem* actItem =  SelItem;

        rectItem = dynamic_cast<QGraphicsRectItem*>(actItem);
        if(rectItem)
        {
            double rectX = rectItem->rect().x();
            double rectY = rectItem->rect().y();
            double rectW = rectItem->rect().width();
            double rectH = rectItem->rect().height();
            QPen pen = rectItem->pen();
            QBrush brush = rectItem->brush();

            scene->addRect(rectX+5.0, rectY-5.0, rectW, rectH, pen, brush);
            return;
        }

        elliItem = dynamic_cast<QGraphicsEllipseItem*>(actItem);
        if(elliItem)
        {
            double elliX = elliItem->rect().x();
            double elliY = elliItem->rect().y();
            double elliW = elliItem->rect().width();
            double elliH = elliItem->rect().height();
            QPen pen = elliItem->pen();
            QBrush brush = elliItem->brush();

            scene->addEllipse(elliX, elliY, elliW, elliH, pen, brush);
            return;
        }

        pathItem = dynamic_cast<QGraphicsPathItem*>(actItem);
        if(pathItem)
        {
            double pathX, pathY;
            for(int i=0;i<pathItem->path().elementCount();i++)
            {
                pathX = pathItem->path().elementAt(i).x;
                pathY = pathItem->path().elementAt(i).y;
            }

            double pathW = pathItem->pen().width();
            QColor pathColor = pathItem->pen().color();
            QPen pen = pathItem->pen();
            QBrush brush = pathItem->brush();

            //ui->drawzone->getScene()->addPath((*path,QPen(pathColor, s, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            return;
        }

        lineItem = dynamic_cast<QGraphicsLineItem*>(actItem);
        if(lineItem)
        {
            double lineX1 = lineItem->line().x1();
            double lineX2 = lineItem->line().x2();
            double lineY1 = lineItem->line().y1();
            double lineY2 = lineItem->line().x2();
            double lineW = lineItem->pen().width();
            QString lineColor = lineItem->pen().color().name();

            QPen pen = lineItem->pen();

            scene->addLine(lineX1,lineY1, lineX2, lineY2, pen);
            return;
        }

        polygonItem = dynamic_cast<QGraphicsPolygonItem*>(actItem);
        if(polygonItem)
        {
            double polygX, polygY;
            for(int i=0;i<pathItem->path().elementCount();i++)
            {
                polygX = polygonItem->polygon().value(i).x();
                polygY = polygonItem->polygon().value(i).y();
            }

            double polygW = polygonItem->pen().width();
            QString polygColor = polygonItem->pen().color().name();
            QString polygColor2 = polygonItem->brush().color().name();
            QPen pen = polygonItem->pen();
            QBrush brush = polygonItem->brush();

            //ui->drawzone->getScene()->addPolygon(pen, brush);
            return;
        }

        textItem = dynamic_cast<QGraphicsTextItem*>(actItem);
        if(textItem)
        {
             double textX = textItem->x();
             double textY = textItem->y();
             QFont font = textItem->font();

             QString t = textItem->toPlainText().simplified();
           // ui->drawzone->getScene()->addText();
             QGraphicsTextItem *qti =scene->addText(t,font);
                          qti->setPos(textX,textY);
            return;
        }

}

QGraphicsItem* drawZone::getActItem()
{
    return SelItem;
}

drawZone::~drawZone(){
    if(pathitem)
        delete pathitem;
    if(path)
        delete path;
    if(previewPoint)
        delete previewPoint;
    if(poly)
        delete poly;
    if(actualPoint)
        delete actualPoint;
    if(SelItem)
        delete SelItem;
    if(previewcircle)
        delete previewcircle;
    if(previewline)
        delete previewline;
    if(previewtriangle)
        delete previewtriangle;
    if(previewrectangle)
        delete previewrectangle;
    if(previewtext)
        delete previewtext;
}
