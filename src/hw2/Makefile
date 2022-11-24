CFLAGS=-lpthread
CXX=gcc

all:
	${CXX} ${CFLAGS} -c server.c
	${CXX} ${CFLAGS} -c client_tester.c
	${CXX} ${CFLAGS} -c socket_utils.c
	${CXX} client_tester.o socket_utils.o ${CFLAGS} ${INC_FLAGS} -o client_tester
	${CXX} server.o socket_utils.o ${CFLAGS} ${INC_FLAGS} -o server
	${CXX} client.c -o client
	# ${CXX} server.c -o server


clean:
	${RM}  socket_utils.o  client_tester.o client_tester client server.o server