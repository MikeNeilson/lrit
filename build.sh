

gcc -c -o lrit.o lrit.c
gcc -lsocket -lnsl  -o lrit main.c lrit.o
