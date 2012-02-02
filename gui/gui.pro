TEMPLATE = app
TARGET = 
DEPENDPATH += . CurvedSalsaDialog progressBarDialog
INCLUDEPATH += . ..
CONFIG += silent
DEFINES += CURVEDSALSA_GUI 

# Input
HEADERS += curvedsalsadialog.h \
	   progressbardialog.h \
	   ../cubehash.h \
	   ../salsa.h \
	   ../random.h

FORMS += CurvedSalsaDialog/curvedsalsagui.ui \
         progressBarDialog/progressbardialog.ui

SOURCES += curvedsalsadialog.cpp \
	main.cpp \
	progressbardialog.cpp \
	../curvedsalsa.cpp \
	../curve25519-donna.c \
	../cubehash.cpp \
	../salsa.cpp \
	../random.cpp
