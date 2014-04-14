#Compile the code
lab1_part1: lab1_part1.c serial.c 
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o lab1_part1.elf lab1_part1.c serial.c
	avr-objcopy -O ihex lab1_part1.elf lab1_part1.hex
	avr-size lab1_part1.elf

lab1_part2: lab1_part2.c serial.c 
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o lab1_part2.elf lab1_part2.c serial.c
	avr-objcopy -O ihex lab1_part2.elf lab1_part2.hex
	avr-size lab1_part2.elf

lab1_part3: lab1_part3.c serial.c 
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o lab1_part3.elf lab1_part3.c serial.c
	avr-objcopy -O ihex lab1_part3.elf lab1_part3.hex
	avr-size lab1_part3.elf

main: main.c serial.c 
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o main.elf main.c serial.c
	avr-objcopy -O ihex main.elf main.hex
	avr-size main.elf

#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your computer
#On Windows, change the argument after -P to appropriate COM port
program_part1: lab1_part1.hex
	avrdude -pm328p -P /dev/tty.usbmodem411 -c arduino -F -u -U flash:w:lab1_part1.hex

program_part2: lab1_part2.hex
	avrdude -pm328p -P /dev/tty.usbmodem411 -c arduino -F -u -U flash:w:lab1_part2.hex

program_part3: lab1_part3.hex
	avrdude -pm328p -P /dev/tty.usbmodem411 -c arduino -F -u -U flash:w:lab1_part3.hex

program_main: main.hex
	avrdude -pm328p -P /dev/tty.usbmodem411 -c arduino -F -u -U flash:w:main.hex

#remove build files
clean:
	rm -fr *.elf *.hex *.o
