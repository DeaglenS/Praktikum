QT = core network sql concurrent

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        clientsocket.cpp \
        customsocket.cpp \
        databasemanager.cpp \
        databaseworker.cpp \
        main.cpp \
        server.cpp \
        testclient.cpp \
        worker.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    clientsocket.h \
    customsocket.h \
    databasemanager.h \
    databaseworker.h \
    server.h \
    testclient.h \
    worker.h
