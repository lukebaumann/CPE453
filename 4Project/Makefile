#Compile the code
program_4: program4.c 
	gcc program4.c -o ext2reader

#Moves all necessary files into a temporary folder for pushing
prepare:
	mkdir toPush
	cp program4.c ./toPush/
	cp program4.h ./toPush/
	cp ext2.h ./toPush/
	cp Makefile ./toPush/

handin:
	handin jseng 453_program4_3 program4.c program4.h ext2.h Makefile

#remove build files
clean:
	rm -fr *.elf *.hex *.o toPush
