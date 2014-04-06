#Compile the code
main: main.c serial.c 
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o main.elf main.c serial.c
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
program: main.hex
	avrdude -pm328p -P /dev/tty.usbmodem621 -c arduino -F -u -U flash:w:main.hex

#remove build files
clean:
	rm -fr *.elf *.hex *.o
