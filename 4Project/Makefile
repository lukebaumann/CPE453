#Compile the code
program_4: program4.c
	gcc program4.c -o ext2reader

#Moves all necessary files into a temporary folder for pushing
prepare:
	mkdir toPush
	cp os.c ./toPush/
	cp synchro.c ./toPush/
	cp os.h ./toPush/
	cp globals.h ./toPush/
	cp program3.c ./toPush/
	cp serial.c ./toPush/
	cp Makefile ./toPush/

handin:
	handin jseng 453_program3_3 os.c synchro.c os.h globals.h program3.c serial.c Makefile

#remove build files
clean:
	rm -fr *.elf *.hex *.o toPush
