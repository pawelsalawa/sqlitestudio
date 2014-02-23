DESTDIR = $$PWD/../output/SQLiteStudio
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../output/SQLiteStudio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../output/SQLiteStudio
else:unix: LIBS += -L$$PWD/../output/SQLiteStudio

INCLUDEPATH += $$PWD/coreSQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio
