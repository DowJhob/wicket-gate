#-------------------------------------------------
#
# Project created by QtCreator 2019-02-13T17:46:42
#
#-------------------------------------------------

QT       += core network sql websockets
QT       += scxml

#QT       += httpserver
QT       -= gui


CONFIG += c++11
CONFIG += console
#CONFIG += exception

GIT_VERSION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD/../ describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"
VERSION = $$GIT_VERSION
    win32 {
        VERSION ~= s/-\d+-g[a-f0-9]{6,}//
    }



#DEFINES += GIT_CURRENT_SHA1="\\\"$(shell git -C \""$$_PRO_FILE_PWD_"\" describe)\\\""
#GIT_HASH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""
#GIT_BRANCH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --abbrev-ref HEAD)\\\""
#BUILD_TIMESTAMP="\\\"$$system(date -u +\""%Y-%m-%dT%H:%M:%SUTC\"")\\\""
#DEFINES += GIT_HASH=$$GIT_HASH GIT_BRANCH=$$GIT_BRANCH #BUILD_TIMESTAMP=$$BUILD_TIMESTAMP

QMAKE_TARGET_COMPANY = Platinum Arena
QMAKE_TARGET_PRODUCT = wicket gate service
QMAKE_TARGET_DESCRIPTION = control access system
QMAKE_TARGET_COPYRIGHT = copyright eulle@ya.ru

CONFIG(debug, debug|release){TARGET = service-debug}
else
{TARGET = service-$$GIT_VERSION}

CONFIG(release, release|debug){
            win32-g++ {
                            QMAKE_CXXFLAGS  += -flto -funroll-loops
                            CONFIG += -static
                        }
                    }


        win32-g++ {
                        QMAKE_CXXFLAGS  += -fforce-addr
                        QMAKE_CXXFLAGS  += -m32
                        QMAKE_CFLAGS    += -Ofast
                        QMAKE_CXXFLAGS  += -Ofast
  #                      QMAKE_CXXFLAGS  += -march=core2 -mtune=core2
                        #QMAKE_CXXFLAGS  += -mfpmath=sse
                        QMAKE_CXXFLAGS  += -msse4
                        QMAKE_LFLAGS_RELEASE += -static -static-libgcc
                    }
        win32-msvc {
                        QMAKE_LFLAGS_RELEASE += /LTCG
                        QMAKE_CXXFLAGS  += /O2
                    }


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
#    http/cookietestcontroller.cpp \
    covidcertchecker.cpp \
    db_worker.cpp \
    fsm/checkerModel.cpp \
    fsm/imodel.cpp \
    http/staticfilecontroller.cpp \
    main.cpp \
    main_server.cpp \
    network.cpp \
    reader-connector.cpp \
    ssl_websocket_server.cpp \
    fsm/wicketModel.cpp \
    tcpserver.cpp \
    wicketgate.cpp

HEADERS += \
    command.h \
    common_types.h \
#    http/cookietestcontroller.h \
    covidcertchecker.h \
    db_worker.h \
    fsm/checkerModel.h \
    fsm/imodel.h \
    http/db-query-processor.h \
    http/logincontroller.h \
    http/requestmapper.h \
    http/staticfilecontroller.h \
    http_server.h \
    logger.h \
    main_server.h \
    network.h \
    reader-connector.h \
    ssl_websocket_server.h \
    fsm/scxmlexternmonitor2.h \
    fsm/wicketModel.h \
    tcpserver.h \
    wicketgate.h



include(httpserver/httpserver.pri)


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=

LIBS += -LC:/Qt/Tools/OpenSSL/Win_x64/bin
LIBS += -LC:/Qt/Tools/OpenSSL/Win_x86/bin

#QMAKE_CXXFLAGS += -Wno-psabi
#QMAKE_CFLAGS += -O3
#QMAKE_CXXFLAGS += -O3

DISTFILES += \
    docroot/index.html \
    docroot/js/sha1.js \
    docroot/login.html \
    settings.ini

STATECHARTS += \
    fsm/checker.scxml \
    fsm/locker.scxml
