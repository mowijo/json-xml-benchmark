TEMPLATE = app
TARGET = benchmark
QT += network xml

SOURCES += main.cpp

#
# Sources for flavios qjson
#
SOURCES += qjson/src/parser.cpp \
	   qjson/src/qobjecthelper.cpp \
	   qjson/src/json_scanner.cpp \
	   qjson/src/json_parser.cc \
	   qjson/src/parserrunnable.cpp \
	   qjson/src/serializer.cpp \
	   qjson/src/serializerrunnable.cpp
	  
HEADERS += qjson/src/parser.h \
	   qjson/src/parserrunnable.h \
	   qjson/src/qobjecthelper.h \
	   qjson/src/serializer.h \
	   qjson/src/serializerrunnable.h \
	   qjson/src/qjson_export.h
