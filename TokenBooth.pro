unix {
    INCLUDEPATH += /usr/lib/avr/include
}
win32 {
    #TODO: test in windows and fill this up
}

SOURCES += \
    lcd.c \
    tokenbooth.c

HEADERS += \
    myutils.h \
    lcd.h \
    config.h

OTHER_FILES += \
    Makefile \
    README \
    LICENSE
