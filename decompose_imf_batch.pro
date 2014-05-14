QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_CXXFLAGS += -std=c++11 -pedantic

TEMPLATE = app
CONFIG += c++11 link_prl
DEPENDPATH += . ../cpp_utils/ ../qt_utils/ ../decompose_imf_lib/
INCLUDEPATH += ..

HEADERS  += \
    gui_main_window.h \
    parse_batch.h

SOURCES += \
	main.cpp \
    gui_main_window.cpp \
    parse_batch.cpp

FORMS    += \
    gui_main_window.ui

LIBS += \
	-L../qt_utils -lqt_utils \
	-L../decompose_imf_lib -ldecompose_imf_lib \
	-L../cpp_utils -lcpp_utils \
	-L/usr/lib/ -L/usr/local/lib/ -lopencv_core -lopencv_imgproc -lopencv_highgui \


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../decompose_imf_lib/release/ -ldecompose_imf_lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../decompose_imf_lib/debug/ -ldecompose_imf_lib
else:symbian: LIBS += -ldecompose_imf_lib
else:unix: LIBS += -L$$OUT_PWD/../decompose_imf_lib/ -ldecompose_imf_lib

INCLUDEPATH += $$PWD/../decompose_imf_lib
DEPENDPATH += $$PWD/../decompose_imf_lib

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../decompose_imf_lib/release/decompose_imf_lib.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../decompose_imf_lib/debug/decompose_imf_lib.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../decompose_imf_lib/libdecompose_imf_lib.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qt_utils/release/ -lqt_utils
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qt_utils/debug/ -lqt_utils
else:unix: LIBS += -L$$OUT_PWD/../qt_utils/ -lqt_utils

INCLUDEPATH += $$PWD/../qt_utils
DEPENDPATH += $$PWD/../qt_utils

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qt_utils/release/qt_utils.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qt_utils/debug/qt_utils.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../qt_utils/libqt_utils.a
