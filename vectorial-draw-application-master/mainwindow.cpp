#include <QMouseEvent>
#include <QDebug>
#include <QColorDialog>
#include <QFileDialog>
#include <QAction>
#include <QImage>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDir>
#include <QUndoView>
#include <QGraphicsPixmapItem>
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "imageloadingtask.h"

Point *mouse_coord;
QLabel *labMouseCoord;
QLabel *labIcon;
QLabel *labMsg;
Tool actualTool;

bool isSaved=false;
bool scaled=true;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "init MainWindow" ;
    ui->setupUi(this);

    menuInit();
    dockWidgetInit();
    propertyWidgetInit();

    mouse_coord = new Point();
    labMouseCoord = new QLabel("",statusBar());
    labIcon = new QLabel("",statusBar());
    labIcon->hide();
    labMsg = new QLabel("",statusBar());

    actualTool = CURSOR;
    mouse_coord->x=0;
    mouse_coord->y=0;
    initStatusBar();

    sharedPropertyInit();
    drawZoneSignalInit();
    createUndoView();

    ui->imageWidget->setFloating(true);
    ui->drawzone->setAcceptDrops(true);
    setAcceptDrops(true);

    noPropertyToolSelected();
    imageToolSelected(false);

    folderModel = new QFileSystemModel(this);
    folderModel->setRootPath("/");
    folderModel->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot);

    QMovie *movie = new QMovie(":/Icons/ressources/tuto.gif");
    ui->tutoLabel->setMovie(movie);
    movie->start();

    ui->ImagelistView->setViewMode(QListView::IconMode);
    ui->ImagelistView->setIconSize(QSize(128,128));
    ui->ImagelistView->setResizeMode(QListView::Adjust);
    ui->ImagelistView->setDragEnabled(true);
    ui->ImagelistView->setDragDropMode( QAbstractItemView::DragDrop);
    ui->ImagelistView->setGridSize(QSize(168,148));
    ui->ImagelistView->setMovement(QListView::Snap);
    ui->ImagelistView->viewport()->installEventFilter(this);

    ui->treeFolder->header()->hide();
    ui->treeFolder->setModel(folderModel);
    ui->treeFolder->setRootIndex(folderModel->index(QDir::homePath()));
    ui->treeFolder->hideColumn(1);
    ui->treeFolder->hideColumn(2);
    ui->treeFolder->hideColumn(3);


}

void MainWindow::sharedPropertyInit(){
    ui->drawzone->setactualSize(ui->horizontalSlider->value());
    ui->drawzone->setactualColor(ui->strokeColorButton2->palette().color(ui->strokeColorButton2->backgroundRole()));
    ui->drawzone->setactualColor2(ui->fillColorButton->palette().color(ui->fillColorButton->backgroundRole()));
    ui->drawzone->setactualtextFont(ui->fontComboBox->currentFont());
    ui->drawzone->setactualtextSize(ui->spinBox->value());
    ui->drawzone->setTextContent(ui->textEdit->toPlainText());
}

void MainWindow::drawZoneSignalInit(){
    connect(ui->drawzone, SIGNAL(actualToolShowProperty(Tool)),SLOT(actualToolChangeProperty(Tool)));
    connect(ui->drawzone, SIGNAL(setStrokeColor(QColor)),SLOT(changeStrokeColor(QColor)));
    connect(ui->drawzone, SIGNAL(setFillColor(QColor)),SLOT(changeFillColor(QColor)));
    connect(ui->drawzone, SIGNAL(setStrokeSize(int)),SLOT(changeStrokeSize(int)));
    connect(ui->drawzone, SIGNAL(setTextFont(QFont)),SLOT(changeTextFont(QFont)));
    connect(ui->drawzone, SIGNAL(setTextContent(QString)),SLOT(changeTextContent(QString)));
}
void MainWindow::dockWidgetInit(){
    ui->drawzone->hide();
    ui->helpWidget->hide();
    connect(ui->actionAide, SIGNAL(triggered()),this, SLOT(helpButtonClicked()));
    connect(ui->actionOutils, SIGNAL(triggered()),this, SLOT(toolButtonClicked()));
    connect(ui->actionProprietes, SIGNAL(triggered()),this, SLOT(propertyButtonClicked()));
    connect(ui->actionAnnuler, SIGNAL(triggered()),this, SLOT(undoButtonClicked()));
    connect(ui->actionR_tablir, SIGNAL(triggered()),this, SLOT(redoButtonClicked()));
    connect(ui->actionListe_D_actions, SIGNAL(triggered()),SLOT(ActionListClicked()));
    connect(ui->imageWidget, SIGNAL(visibilityChanged(bool)),SLOT(ImageWidgetOpened(bool)));

}

void MainWindow::menuInit(){
    connect(ui->actionNouveau, SIGNAL(triggered()),this,SLOT(newFile()));
    connect(ui->actionOuvrir, SIGNAL(triggered()),this,SLOT(openFile()));
    connect(ui->actionEnregistrer, SIGNAL(triggered()),SLOT(save()));
    connect(ui->actionEnregistrer_sous, SIGNAL(triggered()),SLOT(saveAs()));
}

void MainWindow::propertyWidgetInit(){
    connect(ui->squareButton, SIGNAL(clicked()),this,SLOT(shapeToolSelected()));
    connect(ui->triangleButton, SIGNAL(clicked()),this,SLOT(shapeToolSelected()));
    connect(ui->circleButton, SIGNAL(clicked()),this,SLOT(shapeToolSelected()));
    connect(ui->pointButton, SIGNAL(clicked()),this,SLOT(shapeToolSelected()));

    connect(ui->lineButton, SIGNAL(clicked()),this,SLOT(lineToolSelected()));
    connect(ui->freeDrawButton, SIGNAL(clicked()),this,SLOT(lineToolSelected()));
    connect(ui->textButton, SIGNAL(clicked()),this,SLOT(textToolSelected()));

    connect(ui->pictureButton, SIGNAL(toggled(bool)),this,SLOT(imageToolSelected(bool)));
    connect(ui->cursorButton, SIGNAL(clicked()),this,SLOT(noPropertyToolSelected()));

    connect(ui->lineButton, SIGNAL(toggled(bool)),this,SLOT(lineChecked(bool)));
    connect(ui->freeDrawButton, SIGNAL(toggled(bool)),this,SLOT(lineChecked(bool)));
}

MainWindow::~MainWindow()
{
    delete mouse_coord;
    delete labMouseCoord;
    delete labIcon;
    delete labMsg;
    delete ui;
}

void MainWindow::ImageWidgetOpened(bool opened){
    if(!opened){
        noPropertyToolSelected();
        ui->cursorButton->setChecked(true);
    }
}

void MainWindow::initStatusBar()
{
    QIcon *green = new QIcon(":/Icons/ressources/Pointer.png");
    labIcon->setPixmap( green->pixmap(24,24) );


    statusBar()->addWidget(labIcon,0);
    statusBar()->addWidget(labMouseCoord,0);
    statusBar()->addWidget(labMsg,0);
}

void MainWindow::showStatusMessage(const QString &msg){
    labMsg->setText(msg);
}

void MainWindow::actualToolChangeProperty(Tool tool){
    qDebug() << "Edition des caractéristiques de l'outil !";
    switch(tool){
        case LINE:
            lineChecked(true);
            ui->actualProperty->setCurrentIndex(0);
            break;
        case RECTANGLE:
            lineChecked(false);
            ui->actualProperty->setCurrentIndex(0);
            break;
        case TEXT:
            ui->actualProperty->setCurrentIndex(1);
            break;
    case CURSOR:
            ui->actualProperty->setCurrentIndex(2);
            break;
    default:
        break;
    }

}

void MainWindow::newFile()
{

    clearFile(true);
    ui->drawzone->resetMatrix();



}

void MainWindow::clearFile(bool reset_size){
    isSaved = false;
    scaled = true;
    ui->drawzone->clearScene();
    ui->drawzone->show();
    if(reset_size){
        ui->scrollAreaWidgetContents->resize(ui->scrollArea->width()+2,ui->scrollArea->height()+2);
        ui->drawzone->setGeometry(0,0,ui->scrollArea->width(),ui->scrollArea->height());
        ui->drawzone->getScene()->setSceneRect(0,0,ui->scrollArea->width(),ui->scrollArea->height());
        ui->drawzone->setSceneRect(0,0,ui->scrollArea->width(),ui->scrollArea->height());
        ui->drawzone->setWidht(ui->scrollArea->width());
        ui->drawzone->setHeight(ui->scrollArea->height());
        ui->drawzone->updateGeometry();
    }
    ui->drawzone->selectNothing();
    ui->drawzone->setactualTool(CURSOR);

}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }
    QStringList mimeTypeFilters;
       const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
           ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
       for (const QByteArray &mimeTypeName : supportedMimeTypes)
           mimeTypeFilters.append(mimeTypeName);

       //qDebug()<<mimeTypeFilters;
       mimeTypeFilters.sort();
       dialog.setMimeTypeFilters(mimeTypeFilters);
       dialog.selectMimeTypeFilter("image/jpeg");
       if (acceptMode == QFileDialog::AcceptSave)
           dialog.setDefaultSuffix("jpg");
}

void MainWindow::openFile()
{
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog;
    initializeImageFileDialog(dialog,QFileDialog::AcceptOpen);
    QString fileName = dialog.getOpenFileName(this, "Ouvrir un fichier", picturesLocations.last(), "Dessin vectoriel (*.fdv);;Images (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm *.svg)");
    if (!fileName.isEmpty()){
        if(fileName.contains(".fdv"))
            loadFile(fileName);
        else{
            newFile();
            loadImportedFile(fileName);
        }
    }
}

bool MainWindow::save()
{
    if (currentFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(currentFile);
    }
}

bool MainWindow::saveAs()
{
    QString filePath;
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog(this, "Enregistrer un fichier", picturesLocations.last(), "Dessin vectoriel (*.fdv)");

        dialog.setWindowModality(Qt::WindowModal);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec() != QDialog::Accepted)
            return false;
        filePath=dialog.selectedFiles().first();
        if(!filePath.contains(".fdv"))
            filePath.append(".fdv");
        return saveFile(filePath);
}


bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QStringList it;
    it.append("scene_size{"+QString::number(ui->drawzone->getScene()->width())+","+QString::number(ui->drawzone->getScene()->height())+"}\n");
    foreach (QGraphicsItem *item, ui->drawzone->getScene()->items()){
        QString coord, tmp;

        imgItem =dynamic_cast<QGraphicsPixmapItem*>(item);
        if(imgItem){
            qDebug()<<"sauvegarder image";
            tmp.append("img "+QString::number(imgItem->x())+" "+QString::number(imgItem->y())+" "+QString::number(imgItem->rotation())+" { ");
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            imgItem->pixmap().save(&buffer, "PNG");
            auto const encoded = buffer.data().toBase64();
            QString imgData = QLatin1String(encoded);
            tmp.append(imgData);
            tmp.append(" }\n");
        }


        pathItem = dynamic_cast<QGraphicsPathItem*>(item);
        if(pathItem){
            coord.append("{ ");
            for(int i=0;i<pathItem->path().elementCount();i++){
                coord.append(QString::number(pathItem->path().elementAt(i).x)+" "+QString::number(pathItem->path().elementAt(i).y)+" ");
            }
            coord.append("}");
            tmp.append("path "+coord+" "+QString::number(pathItem->rotation())+" "+QString::number(pathItem->pen().width())+" "+pathItem->pen().color().name());
        }
        lineItem = dynamic_cast<QGraphicsLineItem*>(item);
        if(lineItem)
            tmp.append("line "+QString::number(lineItem->line().x1())+" "+QString::number(lineItem->line().y1())+" "+QString::number(lineItem->line().x2())+" "+QString::number(lineItem->line().y2())+" "+QString::number(lineItem->rotation())+" "+QString::number(lineItem->pen().width())+" "+lineItem->pen().color().name());

        polygonItem = dynamic_cast<QGraphicsPolygonItem*>(item);
        if(polygonItem){
            coord.append("{ ");
            for(int i=0;i<polygonItem->polygon().size();i++){
                coord.append(QString::number(polygonItem->polygon().value(i).x())+" "+QString::number(polygonItem->polygon().value(i).y())+" ");
            }
            coord.append("}");
            tmp.append("poly "+coord+" "+QString::number(polygonItem->rotation())+" "+QString::number(polygonItem->pen().width())+" "+polygonItem->pen().color().name()+" "+polygonItem->brush().color().name());
        }

        rectItem = dynamic_cast<QGraphicsRectItem*>(item);
        if(rectItem)
            tmp.append("rect "+QString::number(rectItem->rect().x())+" "+QString::number(rectItem->rect().y())+" "+QString::number(rectItem->rect().width())+" "+QString::number(rectItem->rect().height())+" "+QString::number(rectItem->rotation())+" "+ QString::number(rectItem->pen().width())+" "+rectItem->pen().color().name()+" "+rectItem->brush().color().name());

        elliItem = dynamic_cast<QGraphicsEllipseItem*>(item);
        if(elliItem)
            tmp.append("elli "+QString::number(elliItem->rect().x())+" "+QString::number(elliItem->rect().y())+" "+QString::number(elliItem->rect().width())+" "+QString::number(elliItem->rect().height())+" "+QString::number(elliItem->rotation())+" "+ QString::number(elliItem->pen().width())+" "+elliItem->pen().color().name()+" "+elliItem->brush().color().name());

        textItem = dynamic_cast<QGraphicsTextItem*>(item);
        if(textItem)
            tmp.append("text " + QString::number(textItem->x())+" "+QString::number(textItem->y())+" "+QString::number(textItem->rotation())+" ("+ textItem->font().family()+ ","+QString::number(textItem->font().pointSize()) +") " + textItem->toPlainText().simplified());
        coord="";
        tmp.append("\n");

        it.append(tmp);
    }

    QTextStream outputStream(&file);
    for ( QStringList::Iterator iterator = it.begin(); iterator != it.end(); ++iterator )
                    outputStream << *iterator << "\n";
    file.close();

    showStatusMessage("File saved");
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
//! [46] //! [47]
{
    currentFile = fileName;
    ui->drawzone->isWindowModified();
    setWindowModified(false);

    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

 void MainWindow::setCursorLabelCoord(QMouseEvent* ev){
    labMouseCoord->show();
    labIcon->show();
    labMouseCoord->setText("( "+QString::number(ev->x())+", "+QString::number(ev->y())+" )");
}

 void MainWindow::leaveDrawZone(){
    labMouseCoord->hide();
    labIcon->hide();

}

void MainWindow::shapeToolSelected(){
    ui->actualProperty->setCurrentIndex(0);
    lineChecked(false);
}

void MainWindow::noPropertyToolSelected(){
    ui->actualProperty->setCurrentIndex(2);
}

void MainWindow::helpButtonClicked(){
    if(!ui->helpWidget->isHidden())
        ui->helpWidget->hide();
    else {
        ui->helpWidget->show();
    }
    /*
    ui->helpWidget->show();
    ui->actionAide->setEnabled(false);
    */

}
void MainWindow::toolButtonClicked(){
    if(!ui->toolWidget->isHidden()){
        ui->toolWidget->hide();
        ui->propertyWidget->hide();
    }else {
        ui->toolWidget->show();
        ui->propertyWidget->show();
    }
}

void MainWindow::setDrawzoneSize(int width, int height){
    ui->scrollAreaWidgetContents->resize(width,height);
    ui->drawzone->setGeometry(0,0,width,height);
    ui->drawzone->updateGeometry();
    ui->scrollAreaWidgetContents->resize(width+2,height+2);
    ui->drawzone->setGeometry(0,0,width,height);
    ui->drawzone->setSceneRect(0,0,width,height);
    ui->drawzone->getScene()->setSceneRect(0,0,width,height);
    ui->drawzone->setWidht(width);
    ui->drawzone->setHeight(height);
    ui->drawzone->updateGeometry();
}

void MainWindow::propertyButtonClicked(){
    Propiete prop;
    prop.setModal(true);
    if(prop.exec() == 1){
        setDrawzoneSize(prop.getWidth(),prop.getHeight());
    }
    QVariant larg = ui->drawzone->width();
    QVariant haut = ui->drawzone->height();
    showStatusMessage(larg.toString()+"x"+haut.toString());
}

void MainWindow::on_strokeColorButton2_clicked()
{
    QColor color = QColorDialog::getColor(ui->strokeColorButton2->palette().color(ui->strokeColorButton2->backgroundRole()), this, "Choisir une couleur");
    if( color.isValid() ){
        int r=0, g=0, b=0;
        color.getRgb(&r,&g,&b);
        QString scolor("background-color: rgb(" + QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b) + ");");
        ui->strokeColorButton2->setStyleSheet(scolor);
        ui->drawzone->setactualColor(color);
    }
}

void MainWindow::on_fillColorButton_clicked()
{
    QColor color = QColorDialog::getColor(ui->fillColorButton->palette().color(ui->fillColorButton->backgroundRole()), this, "Choisir une couleur");
    if( color.isValid() ){
        int r=0, g=0, b=0;
        color.getRgb(&r,&g,&b);
        QString scolor("background-color: rgb(" + QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b) + ");");
        ui->fillColorButton->setStyleSheet(scolor);
        ui->drawzone->setactualColor2(color);
    }
}

void MainWindow::lineToolSelected(){
    ui->actualProperty->setCurrentIndex(0);
}
void MainWindow::textToolSelected(){
    ui->actualProperty->setCurrentIndex(1);
    ui->drawzone->setactualTool(TEXT);
}

void MainWindow::lineChecked(bool checked){
    if(checked){
        ui->label_3->setVisible(false);
        ui->fillColorButton->setVisible(false);
    }else{
        ui->label_3->setVisible(true);
        ui->fillColorButton->setVisible(true);
    }
}
void MainWindow::on_squareButton_clicked()
{
    ui->drawzone->setactualTool(RECTANGLE);
}

void MainWindow::on_circleButton_clicked()
{
    ui->drawzone->setactualTool(CIRCLE);
}

void MainWindow::on_triangleButton_clicked()
{
    ui->drawzone->setactualTool(TRIANGLE);
}

void MainWindow::on_pictureButton_clicked()
{
    ui->drawzone->setactualTool(IMAGE);
}

void MainWindow::imageToolSelected(bool selected){
    if(selected){
        ui->imageWidget->show();
        ui->actualProperty->setCurrentIndex(3);
        ui->tutoLabel->show();
        ui->tutoLabel2->setText("Sélectionnez un dossier pour pouvoir insérer une/des image(s)");
        ui->ImagelistView->hide();
    }else
        ui->imageWidget->hide();
}

void MainWindow::on_cursorButton_clicked()
{
    ui->drawzone->setactualTool(CURSOR);
}

void MainWindow::on_freeDrawButton_clicked()
{
    ui->drawzone->setactualTool(FREE);
}

void MainWindow::on_pointButton_clicked()
{
    ui->drawzone->setactualTool(POLYGON);
}

void MainWindow::on_actionZoomPlus_triggered()
{
    ui->drawzone->scale(2.0,2.0);
}

void MainWindow::on_actionZoomMoins_triggered()
{
     ui->drawzone->scale(0.5,0.5);
}

void MainWindow::on_actionExporter_triggered()
{
    QFileDialog dialog(this);
   dialog.setWindowModality(Qt::WindowModal);
   dialog.setAcceptMode(QFileDialog::AcceptSave);
   initializeImageFileDialog(dialog, QFileDialog::AcceptSave);
   while (dialog.exec() == QDialog::Accepted && !saveExportFile(dialog.selectedFiles().first())) {}
}

bool MainWindow::saveExportFile(const QString &fileName){
    qDebug() << __FUNCTION__ <<currentFile<<"save basique";
        if(ui->drawzone->saveFile(fileName)){
            setCurrentFile(fileName);
            isSaved = true;
            return true;
        }else{
            return false;
        }
}

/*
void MainWindow::on_actionImporter_triggered()
{
    if(ui->drawzone->isHidden()){
                ui->drawzone->show();
        }

        isSaved = false;
        QFileDialog dialog(this, tr("Open File"));
        initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
        while (dialog.exec() == QDialog::Accepted && !loadImportedFile(dialog.selectedFiles().first())) {}
}
*/

bool MainWindow::loadImportedFile(const QString &fileName){

    QPixmap pixm;
    QImage read;
    read.load(fileName);
    pixm = QPixmap::fromImage(read);
    if (pixm.isNull()){
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), "Not a picture"));
        return false;

    }
    else{
        //redimensionnement de la zone de travail
        if(scaled)
        {
            ui->scrollAreaWidgetContents->resize(pixm.width()+2,pixm.height()+2);
            ui->drawzone->setGeometry(0,0,pixm.width(),pixm.height());
            ui->drawzone->setSceneRect(0,0,pixm.width(),pixm.height());
            ui->drawzone->getScene()->setSceneRect(0,0,pixm.width(),pixm.height());
            ui->drawzone->setWidht(pixm.width());
            ui->drawzone->setHeight(pixm.height());
            ui->drawzone->updateGeometry();
            scaled = false;
        }

        //Pour indiquer que cette image est une image de fond, on lui donne une valeur de z facilement discriminatoire
        QGraphicsPixmapItem *qpip = ui->drawzone->getScene()->addPixmap(pixm);
        qpip->setZValue(-1000);
        setCurrentFile(fileName);
        return true;
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->drawzone->setactualSize(value);
}

void MainWindow::on_lineButton_clicked()
{
     ui->drawzone->setactualTool(LINE);
}

void MainWindow::changeStrokeColor(QColor color){
    int r=0, g=0, b=0;
    color.getRgb(&r,&g,&b);
    QString scolor("background-color: rgb(" + QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b) + ");");
    ui->strokeColorButton2->setStyleSheet(scolor);
}
void MainWindow::changeFillColor(QColor color){
    int r=0, g=0, b=0;
    color.getRgb(&r,&g,&b);
    QString scolor("background-color: rgb(" + QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b) + ");");
    ui->fillColorButton->setStyleSheet(scolor);
}
void MainWindow::changeStrokeSize(int value){
    ui->horizontalSlider->setValue(value);
}
void MainWindow::changeTextFont(QFont font){
    ui->fontComboBox->setCurrentFont(font);
    ui->spinBox->setValue(font.pointSize());
}
void MainWindow::changeTextContent(QString text){
    ui->textEdit->setText(text);
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    ui->drawzone->setactualtextSize(arg1);
}

void MainWindow::on_fontComboBox_currentFontChanged(const QFont &f)
{
     ui->drawzone->setactualtextFont(f);
}

void MainWindow::on_textEdit_textChanged()
{
    ui->drawzone->setactualtextContent(ui->textEdit->toPlainText());
}

void MainWindow::createUndoView()
{
    undoView = new QUndoView(ui->drawzone->getactualStack());
    undoView->setWindowTitle(tr("Dernières actions effectuées"));
    undoView->setAttribute(Qt::WA_QuitOnClose, false);
/*
    ui->ActionListView = new QUndoView(ui->drawzone->getactualStack());
    ui->ActionListView->update();
    ui->ActionListView->show();
*/
}

void MainWindow::undoButtonClicked()
{
    ui->drawzone->undo();
}

void MainWindow::redoButtonClicked()
{
    ui->drawzone->redo();
}

void MainWindow::ActionListClicked(){
    if(undoView->isVisible()){
        undoView->hide();
    }else{
        undoView->show();
    }
}
bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(nullptr, "Logiciel de dessin vectoriel","Format de fichier incompatible");
        return false;
    }
    QTextStream flux(&file);
    QString ligne,src;
    qDebug()<<"Traitement du fichier";

    while(! flux.atEnd())
    {
        //traitement de la ligne
        ligne = flux.readLine();

            bool isize=false, iimg=false, ipoly=false, ielli=false, ipath=false, irec=false, itext=false, iline=false, xRead=true,firstPointSet=false;
            int x=-1,y=-1,x2=-1,y2=-1,w=-1,h=-1,s=-1,tsize=0,drawW=0, drawH=0;
            double r=-1.0;
            QColor qa, qb;
            QString t;
            QPolygonF *poly=new QPolygonF();
            QPainterPath *path=new QPainterPath();
            QString points="", imgData="";
            QPointF coord;
            QFont font;

            QRegExp rsize("(scene_size\\{)([.\\d]+)(,)([.\\d]+)(\\})");
            QRegExp rrect("(rect )([-\\d]+)( )([-\\d]+)( )([\\d]+)( )([\\d]+)( )([-.\\d]+)( )([\\d]+)( )(#[\\w]+)( )(#[\\w]+)");
            QRegExp relli("(elli )([-\\d]+)( )([-\\d]+)( )([\\d]+)( )([\\d]+)( )([-.\\d]+)( )([\\d]+)( )(#[\\w]+)( )(#[\\w]+)");
            QRegExp rpath("(path )([\\{ ])([-. \\d]+)([ \\}])( )([-.\\d]+)( )([\\d]+)( )(#[\\w]+)");
            QRegExp rpoly("(poly )([\\{ ])([-. \\d]+)([ \\}])( )([-.\\d]+)( )([\\d]+)( )(#[\\w]+)( )(#[\\w]+)");
            QRegExp rtext("(text )([-\\d]+)( )([-\\d]+)( )([-.\\d]+)( )[\(]([ \\w]+)(,)(\\d+)[\\)]( )([^\\n]+)");
            QRegExp rline("(line )([-\\d]+)( )([-\\d]+)( )([-\\d]+)( )([-\\d]+)( )([.-\\d]+)( )([\\d]+)( )(#[\\w]+)");
            QRegExp rimag("(img )([\\d]+)( )([\\d]+)( )([-.\\d]+)([\\{ ])([^\\}]+)([ \\}])");

            if(rsize.indexIn(ligne)!=-1){
                qDebug()<<"Taille zone dessin";
                drawW = rsize.cap(2).toInt();
                drawH = rsize.cap(4).toInt();
                isize=true;

            }

            if(rimag.indexIn(ligne)!=-1){
                qDebug()<<"lecture image";
                x=rimag.cap(2).toInt();
                y=rimag.cap(4).toInt();
                r=rimag.cap(6).toDouble();
                imgData=rimag.cap(8);
                iimg=true;
            }

            //le premier mot capturé commence à 1
            if(rrect.indexIn(ligne)!=-1){
                qDebug()<<"lecture rectangle";
                x=rrect.cap(2).toInt();
                y=rrect.cap(4).toInt();
                w=rrect.cap(6).toInt();
                h=rrect.cap(8).toInt();
                r=rrect.cap(10).toDouble();
                s=rrect.cap(12).toInt();
                qa.setNamedColor(rrect.cap(14));
                qb.setNamedColor(rrect.cap(16));
                irec=true;
            }

            if(relli.indexIn(ligne)!=-1){
                qDebug()<<"lecture rond";
                x=relli.cap(2).toInt();
                y=relli.cap(4).toInt();
                w=relli.cap(6).toInt();
                h=relli.cap(8).toInt();
                r=relli.cap(10).toDouble();
                s=relli.cap(12).toInt();
                qa.setNamedColor(relli.cap(14));
                qb.setNamedColor(relli.cap(16));
                ielli=true;
            }
            if(rpath.indexIn(ligne)!=-1){
                qDebug()<<"lecture tracé libre";
                points=rpath.cap(3);
                r=rpath.cap(6).toDouble();
                s=rpath.cap(8).toInt();
                qa.setNamedColor(rpath.cap(10));
                ipath=true;
            }
            if(rpoly.indexIn(ligne)!=-1){
                qDebug()<<"lecture polygone";
                points=rpoly.cap(3);
                r=rpoly.cap(6).toDouble();
                s=rpoly.cap(8).toInt();
                qa.setNamedColor(rpoly.cap(10));
                qb.setNamedColor(rpoly.cap(12));
                ipoly=true;
            }
            if(rtext.indexIn(ligne)!=-1){
                qDebug()<<"lecture texte";
                x=rtext.cap(2).toInt();
                y=rtext.cap(4).toInt();
                r=rtext.cap(6).toDouble();
                font= QFont(rtext.cap(8));
                tsize=rtext.cap(10).toInt();
                t=rtext.cap(12);
                itext=true;
            }
            if(rline.indexIn(ligne)!=-1){
                qDebug()<<"lecture ligne";
                x=rline.cap(2).toInt();
                y=rline.cap(4).toInt();
                x2=rline.cap(6).toInt();
                y2=rline.cap(8).toInt();
                r=rline.cap(10).toDouble();
                s=rline.cap(12).toInt();
                qa.setNamedColor(rline.cap(14));
                iline=true;
            }

            QStringList word= points.split(" ",QString::SkipEmptyParts);

            for (auto src : word){
                if(xRead){
                    coord.setX(src.toInt());
                    qDebug()<<"a";
                }else{
                    coord.setY(src.toInt());
                    if(!firstPointSet){
                        poly->append(coord);
                        path->moveTo(coord);
                        firstPointSet=true;
                    }else{
                        poly->append(coord);
                        path->lineTo(coord);
                    }
                    qDebug()<<"b";
                }
                xRead=!xRead;
            }

         QGraphicsItem *qi=nullptr;
         if(isize){
             qDebug()<<"Le fichier est de taille : "<<drawW<<" * "<<drawH;
             clearFile(false);
             setDrawzoneSize(drawW,drawH);
         }
         if(iimg){
             auto const encoded = imgData.toLatin1();
             QPixmap p;
             p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
             qi=ui->drawzone->getScene()->addPixmap(p);
             qi->setPos(x,y);
         }
         if(irec){
             qi=ui->drawzone->getScene()->addRect(x,y,w,h,QPen(qa,s),QBrush(qb));
         }
         if(ielli){
             qi=ui->drawzone->getScene()->addEllipse(x,y,w,h,QPen(qa,s),QBrush(qb));
         }
         if(ipath){
             qi=ui->drawzone->getScene()->addPath(*path,QPen(qa, s, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
         }
         if(ipoly){
             qi=ui->drawzone->getScene()->addPolygon(*poly,QPen(qa,s,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),QBrush(qb));
         }
         if(iline){
             qi=ui->drawzone->getScene()->addLine(x,y,x2,y2,QPen(qa,s,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
         }
         if(itext){
             font.setPointSize(tsize);
             qi = ui->drawzone->getScene()->addText(t,font);
             qi->setPos(x,y);
         }
         if(qi){
            qi->setFlag(QGraphicsItem::ItemIsSelectable);
            qi->setFlag(QGraphicsItem::ItemIsMovable);
            qi->setTransformOriginPoint(qi->boundingRect().center());
            qi->setRotation(r);
         }

 /* ______________________________________________________________________________________________________________________________________________________ */
 /* ______________________________________________________________________________________________________________________________________________________ */
 /* ______________________________________________________________________________________________________________________________________________________ */

    }
    showStatusMessage("Fichier chargé");
    return true;
}


void MainWindow::on_treeFolder_clicked(const QModelIndex &index)
{

    ui->tutoLabel->hide();
    ui->tutoLabel2->setText("Glissez-déposez une image sur la zone de dessin");
    ui->ImagelistView->show();

    qDeleteAll(m_mapFileNameListWidgetItem);
    m_mapFileNameListWidgetItem.clear();
    mPath = folderModel->fileInfo(index).absoluteFilePath();
    QStringList filters;
    filters << "*.gif" << "*.jpg" << "*.jpg" << "*.jpeg" << "*.JPEG" << "*.png" << "*.tiff" << "*.bmp" << "*.dib" << "*.svg" << "*.svgz" << "*.ico" << "*.wbmp"<< "*.webp" << "*.icns" << "*.bpm" << "*.ppm" << "*.pgm" << "*.tga" << "*.icb" << "*.tpic" << "*.vda" << "*.vst" << "*.xbm" << "*.xpm";
    QDir directory(mPath);
    QStringList images = directory.entryList(filters,QDir::Files);


    foreach(QString filename, images) {
        QString fullpath=mPath+"/"+filename;
        if (m_mapFileNameListWidgetItem.find(filename) != m_mapFileNameListWidgetItem.end())
                    continue;

                QListWidgetItem *item = new QListWidgetItem(ui->ImagelistView);
                item->setSizeHint(QSize(150, 150));
                ui->ImagelistView->addItem(item);
                m_mapFileNameListWidgetItem[fullpath] = item;

                ImageLoadingTask *sub = new ImageLoadingTask(filename,fullpath);
                connect(sub, SIGNAL(finished(QString, QString, QImage, QByteArray)), this, SLOT(imageLoaded(QString, QString, QImage, QByteArray)), Qt::QueuedConnection);
                QThreadPool::globalInstance()->start(sub);

    }
}

void MainWindow::imageLoaded(QString fileName, QString fullPath, QImage img, QByteArray bytes)
{
    QListWidgetItem *item = m_mapFileNameListWidgetItem[fullPath];
    if (!item) {
        return;
    }
    item->setText(fileName);
    item->setIcon(QIcon(QPixmap::fromImage(img)));
    showStatusMessage("icone "+fileName+" chargée !");
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj!=ui->ImagelistView->viewport())
        return false;
    QMouseEvent *ev = static_cast<QMouseEvent *>(event);
    if (event->type() == QEvent::MouseButtonPress)
    {
        //QModelIndex index = ;
        //QString itemText = index.data(Qt::DisplayRole).toString();
        QListWidgetItem *selectedItem;
        selectedItem = ui->ImagelistView->itemAt(ev->x(),ev->y());
        QString filename = ui->ImagelistView->itemAt(ev->x(),ev->y())->data(Qt::DisplayRole).toString();
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        QList<QUrl> url;
        url.append(QUrl(mPath+"/"+filename));
        mimeData->setUrls(url);
        drag->setMimeData(mimeData);
        drag->setPixmap(selectedItem->icon().pixmap(128,128));

        Qt::DropAction dropAction = drag->exec();
        if(dropAction==Qt::MoveAction||dropAction==Qt::CopyAction)
            qDebug()<<"Drag & drop effectué";

        //do something
    }
        return QObject::eventFilter(obj, event);
}
