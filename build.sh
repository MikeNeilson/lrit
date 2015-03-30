
g++ -g -c -o lrit.o lrit.cpp
g++ -g -c -o fileproc.o fileproc.cpp
g++ -g -c -o main.o main.cpp
g++ -g -lsocket -lnsl  -o lrit main.o lrit.o fileproc.o
