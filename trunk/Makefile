CCFLAGS=-Wall -fPIC -g -I/usr/include/python2.5 -I./ -I./kbool/include

OBJECTS=Arc.o Area.o booleng.o Circle.o graph.o graphlst.o instonly.o line.o link.o lpoint.o node.o PythonStuff.o record.o scanbeam.o

SHARED_LIBRARY=./area.so

${SHARED_LIBRARY}: ${OBJECTS}
	gcc -shared -fPIC -Wl,-soname,area.so.0 -o ${SHARED_LIBRARY} ${OBJECTS} -lstdc++ -lpython2.5

Arc.o: Arc.cpp
	gcc -c $? ${CCFLAGS} -o $@

Area.o: Area.cpp
	gcc -c $? ${CCFLAGS} -o $@

booleng.o: kbool/src/booleng.cpp
	gcc -c $? ${CCFLAGS} -o $@

Circle.o: Circle.cpp
	gcc -c $? ${CCFLAGS} -o $@

graph.o: kbool/src/graph.cpp
	gcc -c $? ${CCFLAGS} -o $@

graphlst.o: kbool/src/graphlst.cpp
	gcc -c $? ${CCFLAGS} -o $@

instonly.o: kbool/src/instonly.cpp
	gcc -c $? ${CCFLAGS} -o $@

line.o: kbool/src/line.cpp
	gcc -c $? ${CCFLAGS} -o $@

link.o: kbool/src/link.cpp
	gcc -c $? ${CCFLAGS} -o $@

lpoint.o: kbool/src/lpoint.cpp
	gcc -c $? ${CCFLAGS} -o $@

node.o: kbool/src/node.cpp
	gcc -c $? ${CCFLAGS} -o $@

PythonStuff.o: PythonStuff.cpp
	gcc -c $? ${CCFLAGS} -o $@

record.o: kbool/src/record.cpp
	gcc -c $? ${CCFLAGS} -o $@

scanbeam.o: kbool/src/scanbeam.cpp
	gcc -c $? ${CCFLAGS} -o $@

clean:
	-rm ${SHARED_LIBRARY} ${OBJECTS}


