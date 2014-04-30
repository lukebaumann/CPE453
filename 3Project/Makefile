#Compile the code
program2: program2.c os.c serial.c 
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o program2.elf program2.c os.c serial.c
	avr-objcopy -O ihex program2.elf program2.hex
	avr-size program2.elf

osS: os.c
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 os.c -S

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
program_main: program2.hex
	avrdude -pm328p -P /dev/tty.usbmodem411 -c arduino -F -u -U flash:w:program2.hex

#Moves all necessary files into a temporary folder for pushing
prepare:
	mkdir toPush
	cp os.c ./toPush/
	cp os.h ./toPush/
	cp globals.h ./toPush/
	cp program2.c ./toPush/
	cp serial.c ./toPush/
	cp Makefile ./toPush/

handin:
	handin jseng 453_program2_3 os.c os.h globals.h program2.c serial.c Makefile

#remove build files
clean:
	rm -fr *.elf *.hex *.o
