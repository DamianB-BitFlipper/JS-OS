#ifndef CPU_H
#define CPU_H

/*reads the CMOS memory*/
unsigned char readCMOS(unsigned char addr);

/*writes to the CMOS memory*/
void writeCMOS(unsigned char addr, unsigned int value)

/*command to reboot the system*/
void reboot();

#endif
