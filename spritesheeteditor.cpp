#include "spritesheeteditor.h"
#include "ui_spritesheeteditor.h"

#include <QFileDialog>
#include <QDebug>
#include <QtMath>
#include <QImageWriter>

SpriteSheetEditor::SpriteSheetEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SpriteSheetEditor),
    frames(QList<QImage>()),
    displayedFrames(QList<QGraphicsPixmapItem*>()),
    matrix(QTransform()),
    scale(1.0),
    reference(nullptr)
{
    ui->setupUi(this);

    preview = new QGraphicsScene(QRectF(0, 0, ui->graphicsView->width(), ui->graphicsView->height()));
    exportPreview = new QGraphicsScene(QRectF(0, 0, ui->graphicsView_2->width(), ui->graphicsView_2->height()));
    ui->graphicsView->setScene(preview);
    ui->graphicsView_2->setScene(exportPreview);

    connect(ui->actionImport, SIGNAL(triggered()), this, SLOT(searchFiles()));
    connect(ui->actionImport_reference, SIGNAL(triggered()), this, SLOT(searchReference()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    ui->horizontalSlider->setDisabled(true);
    ui->horizontalSlider_2->setDisabled(true);
    ui->pushButton->setDisabled(true);
    ui->pushButton_2->setDisabled(true);
    ui->spinBox->setDisabled(true);
    ui->spinBox_2->setDisabled(true);
    ui->spinBox_3->setDisabled(true);

    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setScaleValue(int)));
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(setReferenceAlpha(int)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(cropTransparency()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(exportSpriteSheet()));
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(setVisibleFrames(int)));
    connect(ui->spinBox_2, SIGNAL(valueChanged(int)), this, SLOT(setVisibleFrames(int)));
    connect(ui->spinBox_3, SIGNAL(valueChanged(int)), this, SLOT(drawSpriteSheetPreview(int)));
}

SpriteSheetEditor::~SpriteSheetEditor()
{
    delete preview;
    delete ui;
}

void SpriteSheetEditor::drawSpriteSheetPreview(int width)
{
    exportPreview->clear();
    QPainter* exported = new QPainter();
    spriteSheet = QImage(QSize(width * frames.at(0).width(), (qCeil(frames.length() / width) + 1) * frames.at(0).height()), QImage::Format_ARGB32_Premultiplied);
    spriteSheet.fill(Qt::GlobalColor::transparent);
    exported->begin(&spriteSheet);
    exported->scale(scale, scale);
    for(int i = 0, l = frames.length(); i < l; i++)
        exported->drawImage((i % width) * frames.at(i).width(), qFloor(i / width) * frames.at(i).height(), frames.at(i));
    exported->end();
    delete exported;

    qreal previewScale = qreal(ui->graphicsView_2->width()) / (width * frames.at(0).width());
    if(previewScale < 1)
    {
        QTransform m;
        m.setMatrix(
            previewScale, m.m12(), m.m13(),
            m.m21(), previewScale, m.m23(),
            m.m31(), m.m32(), m.m33());
        ui->graphicsView_2->setTransform(m);
    }
    ui->spinBox_3->setValue(width);
    exportPreview->addPixmap(QPixmap::fromImage(spriteSheet));
}

void SpriteSheetEditor::drawEditArea()
{
    QPixmap pic;
    if(reference != nullptr)
        pic = reference->pixmap();

    preview->clear();
    displayedFrames.clear();
    for(auto frame : frames)
        displayedFrames.append(preview->addPixmap(QPixmap::fromImage(frame)));
    if(!pic.isNull()) reference = preview->addPixmap(pic);
}

void SpriteSheetEditor::searchReference()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Image"), QDir::homePath(), tr("Images (*.png)"));
    reference = preview->addPixmap(QPixmap::fromImage(QImage(file)));
    drawEditArea();

    ui->horizontalSlider_2->setDisabled(false);
    ui->horizontalSlider_2->setValue(ui->horizontalSlider_2->maximum());
}

void SpriteSheetEditor::searchFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Images"), QDir::homePath(), tr("Images (*.png)"));
    files.sort();

    frames.clear();
    for(auto file : files)
        frames.append(QImage(file));

    drawEditArea();
    drawSpriteSheetPreview(qSqrt(frames.length()));

    ui->horizontalSlider->setDisabled(false);
    ui->pushButton->setDisabled(false);
    ui->pushButton_2->setDisabled(false);
    ui->spinBox->setDisabled(false);
    ui->spinBox_2->setDisabled(false);
    ui->spinBox_3->setDisabled(false);

    ui->horizontalSlider->setValue(ui->horizontalSlider->maximum());
    ui->spinBox->setMaximum(frames.length() - 1);
    ui->spinBox_2->setMaximum(frames.length() - 1);
    ui->spinBox->setValue(0);
    ui->spinBox_2->setValue(frames.length() - 1);
    ui->spinBox_3->setMaximum(frames.length());
}

void SpriteSheetEditor::setReferenceAlpha(int a)
{
    qreal alpha = qreal(a) / ui->horizontalSlider->maximum();

    reference->setOpacity(alpha);

    QString text = QString();
    text.setNum(alpha);
    ui->lineEdit_2->setText(text);

    drawSpriteSheetPreview(ui->spinBox_3->value());
}

void SpriteSheetEditor::setScaleValue(int s)
{
    scale = qreal(s) / ui->horizontalSlider->maximum();
    matrix.setMatrix(
        scale, matrix.m12(), matrix.m13(),
        matrix.m21(), scale, matrix.m23(),
        matrix.m31(), matrix.m32(), matrix.m33());

    for(auto disp : displayedFrames)
        disp->setTransform(matrix);

    QString text = QString();
    text.setNum(scale);
    ui->lineEdit->setText(text);

    drawSpriteSheetPreview(ui->spinBox_3->value());
}

int minOpaqueX(QImage frame)
{
    for(int i = 0, width = frame.width(); i < width; i++)
        for(int j = 0, height = frame.height(); j < height; j++)
            if(qAlpha(frame.pixel(i, j)) != 0)
                return i;
    return 0;
}

int minOpaqueY(QImage frame)
{
    for(int j = 0, height = frame.height(); j < height; j++)
        for(int i = 0, width = frame.width(); i < width; i++)
            if(qAlpha(frame.pixel(i, j)) != 0) return j;
    return 0;
}

int maxOpaqueX(QImage frame)
{
    for(int i = frame.width() - 1; i >= 0; i--)
        for(int j = 0, height = frame.height(); j < height; j++)
            if(qAlpha(frame.pixel(i, j)) != 0) return i;
    return 0;
}

int maxOpaqueY(QImage frame)
{
    for(int j = frame.height() - 1; j >= 0; j--)
        for(int i = 0, width = frame.width(); i < width; i++)
            if(qAlpha(frame.pixel(i, j)) != 0) return j;
    return 0;
}

void SpriteSheetEditor::cropTransparency()
{
    int minX = INT_MAX;
    int maxX = 0;
    int minY = INT_MAX;
    int maxY = 0;
    for(auto frame : frames)
        if(minX > minOpaqueX(frame)) minX = minOpaqueX(frame);
    for(auto frame : frames)
        if(maxX < maxOpaqueX(frame)) maxX = maxOpaqueX(frame);
    for(auto frame : frames)
        if(minY > minOpaqueY(frame)) minY = minOpaqueY(frame);
    for(auto frame : frames)
        if(maxY < maxOpaqueY(frame)) maxY = maxOpaqueY(frame);

    QRect crop(minX, minY, maxX - minX + 1, maxY - minY + 1);

    auto newFrameList = QList<QImage>();
    for(auto frame : frames)
        newFrameList.append(frame.copy(crop));
    frames = newFrameList;

    drawEditArea();

    drawSpriteSheetPreview(ui->spinBox_3->value());
}

void SpriteSheetEditor::setVisibleFrames(int i)
{
    int min = ui->spinBox->value();
    int max = ui->spinBox_2->value();

    for(auto d : displayedFrames) d->setVisible(false);

    for(int i = min; i <= max; i++)
        displayedFrames.at(i)->setVisible(true);
}

void SpriteSheetEditor::exportSpriteSheet()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save SpriteSheet"), QDir::homePath(), tr("Images (*.png)"));
    if(!file.endsWith(".png")) file.append(".png");
    QImageWriter fileWrite(file);
    fileWrite.write(spriteSheet);
}
