CCFLAGS=-Wall -fPIC -g -I/kbool/include

OBJECTS=Area.o booleng.o graph.o graphlst.o instonly.o line.o link.o lpoint.o node.o PythonStuff.o record.o scanbeam.o

SHARED_LIBRARY=./area.so

${SHARED_LIBRARY}: ${OBJECTS}
	gcc -shared -fPIC -Wl,-soname,area.so.0 -o ${SHARED_LIBRARY} ${OBJECTS} -lstdc++ -lpython2.5

Area.o: Area.cpp
	gcc -c $? ${CCFLAGS} -o $@

booleng.o: booleng.cpp
	gcc -c $? ${CCFLAGS} -o $@

graph.o: graph.cpp
	gcc -c $? ${CCFLAGS} -o $@

graphlst.o: graphlst.cpp
	gcc -c $? ${CCFLAGS} -o $@

instonly.o: instonly.cpp
	gcc -c $? ${CCFLAGS} -o $@

line.o: line.cpp
	gcc -c $? ${CCFLAGS} -o $@

link.o: link.cpp
	gcc -c $? ${CCFLAGS} -o $@

lpoint.o: lpoint.cpp
	gcc -c $? ${CCFLAGS} -o $@

node.o: node.cpp
	gcc -c $? ${CCFLAGS} -o $@

PythonStuff.o: PythonStuff.cpp
	gcc -c $? ${CCFLAGS} -o $@

record.o: record.cpp
	gcc -c $? ${CCFLAGS} -o $@

scanbeam.o: scanbeam.cpp
	gcc -c $? ${CCFLAGS} -o $@

clean:
	-rm ${SHARED_LIBRARY} ${OBJECTS}


