#include "spritesheeteditor.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SpriteSheetEditor w;
    w.show();

    return a.exec();
}
