
# turn off struct alignment for this section, speed shouldn't be a huge deal here 
# and it really simplfies the code for now.
# in the future it should problem be changed
#gcc -fpack-struct=1 -c -o lrit.o lrit.c
g++ -g -c -o lrit.o lrit.cpp
g++ -g -c -o fileproc.o fileproc.cpp
g++ -g -c -o main.o main.cpp
g++ -g -lsocket -lnsl  -o lrit main.o lrit.o fileproc.o
