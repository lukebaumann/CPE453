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

#remove build files
clean:
	rm -fr *.elf *.hex *.o
