// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Remote Debugger contributors

#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
