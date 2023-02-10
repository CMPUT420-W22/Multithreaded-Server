ECE 420 LAB README 

This README will shortly describe the MAKE functions.

if you were to run make, the all function is called. Then the executables, main1-4, client, and attacker will be made. 
These are made from our various server code. Then you may run each one seperately with either the client or attacker executables.


main1: uses a single mutex to protect theArray.

main2: uses a multiple mutex to protect theArray.

main3: uses a single read-write lock to protect theArray.

main4: uses a multiple read-write lock to protect theArray.

For example: 

On SERVER MACHINE: 

./<serverExe> <ARRAYSIZE> <IP-ADDRESS> <PORTNUM>

On CLIENT MACHINE: 

./<clientExe> <ARRAYSIZE> <IP-ADDRESS> <PORTNUM>


*****MAKE SURE THAT THE THREE ARGUMENTS AFTER THE EXECUTABLES MATCH ON BOTH SIDES*****

make clean will clean all .o*, executables, and the file created by saveTimes(). This can be adjusted simply by commenting out what you dont want to get rid of in the makefile.