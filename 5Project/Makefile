#Compile the code
program_5: program5.c os.c synchro.c serial.c os_util.c SdReader.c ext2.c
	avr-gcc -Wall -mmcu=atmega328p -DF_CPU=16000000 -O2 -o program5.elf program5.c os.c os_util.c SdReader.c synchro.c serial.c ext2.c
	avr-objcopy -O ihex program5.elf program5.hex
	avr-size program5.elf

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
upload: program5.hex
	avrdude -pm328p -P /dev/tty.usbmodem411 -c arduino -F -u -U flash:w:program5.hex

#Upload target for Ubuntu systems
upload_u: program5.hex
	avrdude -pm328p -P /dev/ttyACM0 -c arduino -F -u -U flash:w:program5.hex

#Moves all necessary files into a temporary folder for pushing
prepare:
	mkdir toPush
	cp os.c ./toPush/
	cp synchro.c ./toPush/
	cp os.h ./toPush/
	cp globals.h ./toPush/
	cp program5.c ./toPush/
	cp serial.c ./toPush/
	cp Makefile ./toPush/

handin:
	handin jseng 453_program5_3 os.c synchro.c os.h globals.h program5.c serial.c Makefile

#remove build files
clean:
	rm -fr *.elf *.hex *.o toPush
