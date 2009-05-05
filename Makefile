#Makefile for libarea

PREFIX	= /usr
HOME	= .
CXX     = g++
CC      = gcc
LD      = g++
LDFLAGS = -shared -rdynamic `python-config --ldflags`
LIBS    = -lstdc++ `python-config --libs`
CFLAGS  = -Wall -I/usr/include `python-config --includes` -I./ -I./kbool/include -g -fPIC

LIBNAME	= libarea
LIBOBJS	= Arc.o Area.o booleng.o Circle.o graph.o graphlst.o instonly.o line.o link.o lpoint.o node.o PythonStuff.o record.o scanbeam.o
LIBOUT	= $(LIBNAME).so

all:	$(LIBOUT)

$(LIBOUT): $(LIBOBJS)
	$(LD) $(LDFLAGS) -fPIC -Wl,-soname,libarea.so.0 $(LIBOBJS) -o $(LIBOUT) $(LIBS)

clean:
	@-rm -f $(LIBOBJS)
	@-rm -f $(LIBOUT)

install: $(LIBOUT)
	strip $^
	chmod 644 $^
	install $^ $(PREFIX)/lib

Arc.o: Arc.cpp
	$(CC) -c $? ${CFLAGS} -o $@

Area.o: Area.cpp
	$(CC) -c $? ${CFLAGS} -o $@

booleng.o: kbool/src/booleng.cpp
	$(CC) -c $? ${CFLAGS} -o $@

Circle.o: Circle.cpp
	$(CC) -c $? ${CFLAGS} -o $@

graph.o: kbool/src/graph.cpp
	$(CC) -c $? ${CFLAGS} -o $@

graphlst.o: kbool/src/graphlst.cpp
	$(CC) -c $? ${CFLAGS} -o $@

instonly.o: kbool/src/instonly.cpp
	$(CC) -c $? ${CFLAGS} -o $@

line.o: kbool/src/line.cpp
	$(CC) -c $? ${CFLAGS} -o $@

link.o: kbool/src/link.cpp
	$(CC) -c $? ${CFLAGS} -o $@

lpoint.o: kbool/src/lpoint.cpp
	$(CC) -c $? ${CFLAGS} -o $@

node.o: kbool/src/node.cpp
	$(CC) -c $? ${CFLAGS} -o $@

PythonStuff.o: PythonStuff.cpp
	$(CC) -c $? ${CFLAGS} -o $@

record.o: kbool/src/record.cpp
	$(CC) -c $? ${CFLAGS} -o $@

scanbeam.o: kbool/src/scanbeam.cpp
	$(CC) -c $? ${CFLAGS} -o $@

