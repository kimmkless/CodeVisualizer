QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += debug_and_release
CONFIG += precompile_header
PRECOMPILED_HEADER = stdafx.h

# 项目名称
TARGET = CodeVisualizer
TEMPLATE = app

# 源文件
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    codeexecutor.cpp \
    visualizer.cpp \
    codeparser.cpp

# 头文件
HEADERS += \
    mainwindow.h \
    codeeditor.h \
    codeexecutor.h \
    visualizer.h \
    codeparser.h

# 在Release模式下禁用调试输出
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# 默认规则使应用程序适用于Windows
# win32:RC_ICONS += resources/app_icon.ico

# 资源文件
RESOURCES += \
    resources.qrc

# 附加包含路径
INCLUDEPATH += . 

FORMS += \
    mainwindow.ui 