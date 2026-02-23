#include "CashRegisterWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CashRegisterWindow window;
    window.show();
    return app.exec();
}
