
# turn off struct alignment for this section, speed shouldn't be a huge deal here 
# and it really simplfies the code for now.
# in the future it should problem be changed
gcc -fpack-struct=1 -c -o lrit.o lrit.c
gcc -lsocket -lnsl  -o lrit main.c lrit.o
