/*
 * fdc.h
 * 
 * Copyright 2013 JS <js@duck-squirell>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#ifndef FDC_H
#define FDC_H

//DMA length
#define DMA_LEN       0x4800     //18 KB, most efficient transfer size for 1.44MB floppy disks
#define SECTOR_SIZE   512        //512 bytes sector size
#define FLOPPY_SIZE   0x168000   //1440 KB (1.44MB)

//floppy persisten storage
extern u32int _FloppyStorage;

/*initialize the floppy disk*/
u32int init_floopy();

/*initialize the floppy dma*/
u32int init_floppy_dma();

/*set the dam to read*/
void floppy_dma_read();

/*set the dma to write*/
void floppy_dma_write();

/*waits untill the floppy irq has been recieved*/
void floppy_wait_irq();

/*the irq handler for the floppy*/
void floppy_handler();

/*send a command to the floppy*/
void floppy_send_command(u8int command);

/*read the FIFO*/
u8int floppy_read_data();

/*get the status of the floppy*/
u8int floppy_status();

/*write to the digital output register (dor)*/
void floppy_write_dor(u8int val);

/*write to the configuration control register (ccr)*/
void floppy_write_ccr(u8int val);

/*resets (restarts) the floppy controller*/
u32int floppy_reset();

/*checks the interrupt status of the floppy controller*/
void floppy_check_int(u32int *st0, u32int *cyl);

/*sets controlling info to mechanical drive*/
void floppy_drive_data(u32int stepr, u32int loadt, u32int unloadt, u32int dma);

/*calibrate a floppy drive*/
u32int floppy_calibrate(u8int drive);

/*sets the floppy motor to an input state (on/off)*/
void floppy_motor(u32int state, u8int drive);

/*read data from the floppy*/
u8int *floppy_read(u32int sectorLBA, u32int size);

/*write data to the floppy*/
u8int *floppy_write(u32int *data, u32int size, u32int sectorLBA);

/*move r/w head to input cylinder*/
u32int floppy_seek(u32int cyl, u8int head);

/*read data from a specific sector*/
void floppy_read_sector(u8int head, u8int track, u8int sector, u32int size);

/*write data to a specific sector*/
void floppy_write_sector(u8int head, u8int track, u8int sector, u32int size);

/*change floppy drives*/
u32int floppy_change_drive(u8int drive);

#endif //FDC_H
