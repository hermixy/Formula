# Project Configurations for QMake

TARGET = formula-cli
CONFIG -= qt
macx: CONFIG -= app_bundle

DEPENDPATH = . frontend backend
INCLUDEPATH = . frontend backend 

# C++11 on Unix
unix {
	QMAKE_CXXFLAGS += -std=c++0x
}

# C++11 on Mac OS X
macx {
	QMAKE_CXXFLAGS += -std=c++0x
	QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++
	LIBS += -stdlib=libc++ -mmacosx-version-min=10.7
}

HEADERS += frontend/Semantic.h \
	frontend/CodeGen.h \
	backend/Code.h \
	backend/Operand.h \
	backend/Function.h \
	backend/VM.h

SOURCES += main.cpp \
	frontend/Semantic.cpp \
	frontend/CodeGen.cpp \
	backend/Code.cpp \
	backend/Operand.cpp \
	backend/Function.cpp \
	backend/VM.cpp

######################################################################
# Generating lexer and parser with custom commands
######################################################################

FLEXSOURCES += frontend/lexer.l
BISONSOURCES += frontend/parser.y

flexsource.input = FLEXSOURCES
flexsource.output = ${QMAKE_FILE_BASE}.cpp
flexsource.commands = flex --header-file=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flexsource

flexheader.input = FLEXSOURCES
flexheader.output = ${QMAKE_FILE_BASE}.h
flexheader.commands = @true
flexheader.variable_out = HEADERS
flexheader.name = Flex Headers ${QMAKE_FILE_IN}
flexheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += flexheader

bisonsource.input = BISONSOURCES
bisonsource.output = ${QMAKE_FILE_BASE}.cpp
bisonsource.commands = bison -d --defines=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
bisonsource.variable_out = SOURCES
bisonsource.name = Bison Sources ${QMAKE_FILE_IN}
bisonsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += bisonsource

bisonheader.input = BISONSOURCES
bisonheader.output = ${QMAKE_FILE_BASE}.h
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers ${QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonheader
