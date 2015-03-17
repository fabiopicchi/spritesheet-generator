#ifndef SPRITESHEETEDITOR_H
#define SPRITESHEETEDITOR_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QStringList>

namespace Ui {
class SpriteSheetEditor;
}

class SpriteSheetEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit SpriteSheetEditor(QWidget *parent = 0);
    ~SpriteSheetEditor();

private:
    qreal scale;
    QGraphicsScene* preview;
    QGraphicsScene* exportPreview;
    QList<QImage> frames;
    QGraphicsPixmapItem* reference;
    QImage spriteSheet;
    QList<QGraphicsPixmapItem*> displayedFrames;
    QTransform matrix;
    Ui::SpriteSheetEditor *ui;

    void drawEditArea();

public slots:
    void searchFiles();
    void searchReference();
    void cropTransparency();
    void setScaleValue(int s);
    void setReferenceAlpha(int a);
    void setVisibleFrames(int i);
    void drawSpriteSheetPreview(int width);
    void exportSpriteSheet();
};

#endif // SPRITESHEETEDITOR_H
