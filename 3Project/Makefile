#Compile the code
program_3: program3.c os.c synchro.c serial.c 
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o program3.elf program3.c os.c synchro.c serial.c
	avr-objcopy -O ihex program3.elf program3.hex
	avr-size program3.elf

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
upload: program3.hex
	avrdude -pm328p -P /dev/tty.usbmodem411 -c arduino -F -u -U flash:w:program3.hex

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
