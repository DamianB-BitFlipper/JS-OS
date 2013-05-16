/*
 * fdc.c
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

#include <system.h>

///MAY BE USEFUL: http://www.brokenthorn.com/Resources/OSDev20.html

/*if this is false, functions that want to write to the persistent
 * storage floppy will be denyed, this is used for any usermode
 * function not to write over sensitive boot data over the boot floppy*/
u32int _FloppyStorage = FALSE;

static volatile u32int floppy_base = 0;
static volatile u32int floppy_recieved_int = FALSE;

//number of drives
static volatile u8int nDrives = 0;
//the current drive
static volatile u8int _CurrentDrive;

//our buffer for the DMA
static volatile u8int floppy_dmaBuf[DMA_LEN];
//~ static volatile u8int *floppy_dmaBuf;

//Floppy Registers
enum floppy_registers
{
  FLOPPY_DOR  = 2,  // digital output register
  FLOPPY_MSR  = 4,  // master status register, read only
  FLOPPY_FIFO = 5,  // data FIFO, in DMA operation for commands
  FLOPPY_CCR  = 7   // configuration control register, write only
};

//floppy directions, read or write
typedef enum
{
  FLOPPY_DIR_READ = 1,
  FLOPPY_DIR_WRITE
} floppy_dir_t;

//main statur register masks
enum FLOPPY_MSR_MASK
{
	FLOPPY_MSR_MASK_DRIVE1_POS_MODE =	1,	     //00000001
	FLOPPY_MSR_MASK_DRIVE2_POS_MODE =	2,	     //00000010
	FLOPPY_MSR_MASK_DRIVE3_POS_MODE =	4,	     //00000100
	FLOPPY_MSR_MASK_DRIVE4_POS_MODE =	8,	     //00001000
	FLOPPY_MSR_MASK_BUSY	=	16,	               //00010000
	FLOPPY_MSR_MASK_DMA	=	32,	                 //00100000
	FLOPPY_MSR_MASK_DATAIO	=	64, 	           //01000000
	FLOPPY_MSR_MASK_DATAREG =	128	             //10000000
};

//floppy disk commands
enum FLOPPY_CMD
{
	FDC_CMD_READ_TRACK =	2,
	FDC_CMD_SPECIFY	=	3,
	FDC_CMD_CHECK_STAT =	4,
	FDC_CMD_WRITE_SECT =	5,
	FDC_CMD_READ_SECT	=	6,
	FDC_CMD_CALIBRATE	=	7,
	FDC_CMD_CHECK_INT	=	8,
	FDC_CMD_WRITE_DEL_S	=	9,
	FDC_CMD_READ_ID_S	=	0xa,
	FDC_CMD_READ_DEL_S =	0xc,
	FDC_CMD_FORMAT_TRACK =	0xd,
	FDC_CMD_SEEK =	0xf
};

enum FLOPPY_CMD_EXT
{
	FDC_CMD_EXT_SKIP =	0x20,      	//00100000
	FDC_CMD_EXT_DENSITY	=	0x40,	    //01000000
	FDC_CMD_EXT_MULTITRACK =	0x80	//10000000
};

enum FLOPPY_SECTOR_DTL
{ 
	FLOPPY_SECTOR_DTL_128	=	0,
	FLOPPY_SECTOR_DTL_256	=	1,
	FLOPPY_SECTOR_DTL_512	=	2,
	FLOPPY_SECTOR_DTL_1024	=	4
};

enum FLOPPY_GAP3_LENGTH
{
 
	FLOPPY_GAP3_LENGTH_STD = 42,
	FLOPPY_GAP3_LENGTH_5_14 = 32,
	FLOPPY_GAP3_LENGTH_3_5 = 27
};

enum
{
  FLOPPY_MOTOR_OFF = 0,
  FLOPPY_MOTOR_ON
};

enum
{
  FLOPPY_SECTORS_PER_TRACK = 18
};

void floppy_wait_irq()
{
  //loop untill we have recieved the int (floppy_recieved_int becomes TRUE)
  while(floppy_recieved_int == FALSE);

  //reset floppy_recieved_int to default (FALSE)
  floppy_recieved_int = FALSE;
}

void floppy_handler()
{
  //we have recieve the interupt
  floppy_recieved_int = TRUE;

  //~ k_printf("\tIRQ6 interrupt\n");

}

u8int floppy_status()
{
	//return main status register
	return inb(floppy_base + FLOPPY_MSR);
}

void floppy_send_command(u8int command)
{
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  u32int i;
	//loop a certain amount of times, untill 60 second timeout
	for(i = 0; i < 600; i++)
  {
    //sleep for 10ms
    mSleep(10);

    //if the most significant bit is 1, then the data register is ready
		if(floppy_status() & FLOPPY_MSR_MASK_DATAREG)
			return (void)outb(floppy_base + FLOPPY_FIFO, command);
  }
}

//read data from the FIFO
u8int floppy_read_data()
{
  u32int i;
	//loop a certain amount of times, untill data register is ready
	for(i = 0; i < 500; i++)
  {
    //if the most significant bit is 1, then the data register is ready
		if(floppy_status() & FLOPPY_MSR_MASK_DATAREG)
			return inb(floppy_base + FLOPPY_FIFO);
  }
}

void floppy_check_int(u32int *st0, u32int *cyl)
{
  //send the command to check interrupt status
  //~ floppy_send_command(floppy_base + FDC_CMD_CHECK_INT);
  floppy_send_command(FDC_CMD_CHECK_INT);

  //gets the status register 0 from the FIFO
  *st0 = floppy_read_data();

  //gets the current cylinder from the FIFO
  *cyl = floppy_read_data();
}

u32int floppy_change_drive(u8int drive)
{
  //we can only have 4 floppys, a maximum value for u8int drive being 3
  if(drive >= 4)
  {
    k_printf("Floppy change drive, invalid drive to change to\n");

    //error
    return 1;
  }

  /*if the drive we want to switch to is greater than the nDrives that are
   * detected, nDrives starts from 1 for floppy 0, so decrement by 1 to count
   * for that*/
  if(drive > nDrives - 1)
  {
    k_printf("Floppy change drive, floppy wanting to switch to does not exist");

    //error
    return 2;
  }

  //no need to switch drives, we already have it initialized
  if(drive == _CurrentDrive)
    return 0; //technically, nothing went wrong, so no error

  //set the transfer seed to 500KB/s, default for 1.44MB 3.5in floppys
  floppy_write_ccr(0);

  //set the 4 least significant bits of data
  u8int dor_data = 0b1100 | (drive & 0x3);
  floppy_write_dor(dor_data);
  
  //set _CurrentDrive to its appropriate drive number
  _CurrentDrive = drive;

  //sucess!
  return 0;
}

void floppy_drive_data(u32int stepr, u32int loadt, u32int unloadt, u32int dma)
{
  u8int data;

  //we want to specify the contolling info
  floppy_send_command(FDC_CMD_SPECIFY);

  /*data is set to the 4 least significant bits of stepr
   * shifted 4 to the left and then the 4 least significant bits of unloadt
   * added to the end */
	data = ((stepr & 0xf) << 4) | (unloadt & 0xf);

  //sets the steprate and the unload time
  floppy_send_command(data);

  /*set the least significant bit of loadt to
   * either 1 if dma is set to TRUE and
   * 0 is dma is set to FALSE */
  data = (loadt << 1) | dma == TRUE ? 1 : 0;

  //sets load time
  floppy_send_command(data);

  k_printf("Floppy drive data sent!\n");
}

void floppy_write_dor(u8int val)
{
  //write to dor floppy register
  outb(floppy_base + FLOPPY_DOR, val);

}

void floppy_write_ccr(u8int val)
{
	//write to ccr floppy register
	outb(floppy_base + FLOPPY_CCR, val);
}

void floppy_motor(u32int state, u8int drive)
{

  //test if u8int drive is a valid input
  if(drive >= 4) //we cannot have more than 4 drives
    return;

  ///look at the digital output register specifications
  ///for a better understanding

  //set the 4 least significant bits of data
  u8int data = 0b1100 | (drive & 0x3);

  switch(state)
  {
    case FLOPPY_MOTOR_ON: //if we want to turn the motor on
      switch(drive)
      {
        //sets the 4 significant bits of data corresponding to the drive number's motor
        case 0:
          data = (0b1 << 4) | data;
          break;
        case 1:
          data = (0b10 << 4) | data;
          break;
        case 2:
          data = (0b100 << 4) | data;
          break;
        case 3:
          data = (0b1000 << 4) | data;
          break;
        default:
          return;
      };
      floppy_write_dor(data);
      break;
    case FLOPPY_MOTOR_OFF:
    
      //set the 4 significant bits of data to 0
      data = data & 0xf;
      floppy_write_dor(data);
      break;
    default:
      break;
  };
}

u32int floppy_calibrate(u8int drive)
{
  u32int st0, cyl;

  //test if u8int drive is a valid input
  if(drive >= 4) //we cannot have more than 4 drives
    return 2;

  //turn the floppy motor on
  floppy_motor(FLOPPY_MOTOR_ON, drive);

  u32int i;

  //run calibration 10 times untill sucess
  for(i = 0; i < 10; i++)
  {
    //we want to callibrate the floppy
    floppy_send_command(FDC_CMD_CALIBRATE);

    //calibrate floppy u8int drive
    floppy_send_command(drive);

    //wait for irq to finnish
    floppy_wait_irq();

    //get the interupt status
    floppy_check_int(&st0, &cyl);

    if(st0 & 0xC0) //tests if reset procudure passed
    {
      //shifts the two most significant bits to become the two least
      switch((st0 >> 6) & 0x3)
      {
        case 0b01: //error
          k_printf("calibrating st0 error st0 == 0b01\n");
          break;
        case 0b10: //st0 is invalid
          k_printf("calibrating st0 is invalid\n");
          break;
        case 0b11: //st0 is valid, continue
          k_printf("calibrating st0 is valid\n");
          break;
      };

      continue;
    }

    //if we found cylinder 0, we are done
    if(!cyl)
    {
      floppy_motor(FLOPPY_MOTOR_OFF, drive);

      k_printf("Calibration Sucessful!\n");
      //sucess!
      return 0;
    }

  }

  //we did not calibrate in 10 runs, something is wrong
  floppy_motor(FLOPPY_MOTOR_OFF, drive);

  k_printf("Exceeded floppy calibration cycle: error\n");

  //exceeded floppy calibration cycle error
  return 3;

}

u32int floppy_reset()
{
  //~ asm volatile("sti");
  //~ init_timer(globalFreq); // Initialise timer to globalFreq-Hz

	u32int st0, cyl;

  //disable the controller by setting all of the dor bits to 0
  floppy_write_dor(0x0);
  //~ mSleep(3); //sleep 3 milliseconds while changes are applied

  //set data rate to 500K/s
  //~ outb(0x3f4, 0x0);

  /*enable the controller
   * setting bits 0-1 to 0 indicates controller for drive 0
   * setting bit 2 to 1 indicates to enable the controller
   * setting bit 3 to 1 indicates to use DMA mode
   * setting bits 4-7 to 0 indicates to stop the motor of all driver */
  floppy_write_dor(0xC);

  //~ mSleep(100); //sleep 100 milliseconds while changes are applied

  floppy_wait_irq();

  u32int i;
  //check all of the drives for an interupt and their status
  for(i = 0; i < 4; i++)
    floppy_check_int(&st0, &cyl);

  //set the transfer seed to 500KB/s, default for 1.44MB 3.5in floppys
  floppy_write_ccr(0);

  /*set the controlling data of the floppy to default
   * steprate = 3ms, load time = 16ms, unload time = 240ms, with DMA*/
  floppy_drive_data(3, 16, 240, TRUE);

  //calibrate the floppy
  if(floppy_calibrate(_CurrentDrive))
    return 1; //error

  //sucess!
  return 0;
}

u32int init_floopy()
{
  //get the amount of drives
  u8int drives = readCMOS(0x10);

  u8int a, b;
  a = drives >> 4;
  b = drives & 0xF;

  //Check how many floppy drives are installed
  if(a != 0x4) //There are none, so do not bother
  {
    k_printf("No floppy drives were detected\n");
    //error
    return 1;
  }

  //default current floppy drive is floppy number 0
  _CurrentDrive = 0;

  if(b != 4)
    nDrives = 1;
  else
    nDrives = 2;

  k_printf("Detected %d floppy drive(s)!\n", nDrives);

  //Init to primary floppy controller base
  floppy_base = 0x3f0;

  register_interrupt_handler(IRQ6, &floppy_handler);
  k_printf("IRQ6 set up\n");

  //initialize the floppy dma
  if(init_floppy_dma())
  {
    //an error occured in the DMA initialization
    return 2;
  }

  if(floppy_reset())
  {
    k_printf("Floppy Drive Controler reset was not completed successfully...\n");
    
    //error
    return 1;
  }else
    k_printf("Floppy Drive Controler initialized successfully\n");

  ///Set up secondary floppy disk persistent storage, if applicable
  //if there is only one floppy, cannot set up persistent storage on single disk
  if(nDrives < 2)
  {
    k_printf("Cannot set up floppy persistent storage, only one floppy drive\n");

    /*do not enable floppy persistent storage, we do not want any usermode
     * function writing over sensitive boot information on the boot floppy*/
    _FloppyStorage = FALSE;
    
    return 2; //error
  }

  //set up default storage drive to the sencond floppy (number 1)
  if(floppy_change_drive(1))
  {
    /*do not enable floppy persistent storage, we do not want any usermode
     * function writing over sensitive boot information on the boot floppy*/
    _FloppyStorage = FALSE;
        
    return 3; //error
  }else{
    k_printf("Floppy persistent storage set up to default, second floppy disk\n");

    //enable floppy persistent storage on second storage floppy
    _FloppyStorage = TRUE;
  }
    
  //Sucess!
  return 0;
}

u32int init_floppy_dma()
{
  //made it a union for future expansion and development
  union
  {
    u8int buf[4];
    u32int len;    // 4 bytes
  } addr, count;   // address and count

  //kmalloc space for the dma buffer
  //~ floppy_dmaBuf = (volatile u8int*)kmalloc(DMA_LEN);

  //copy the physical address of the dma buffer
  addr.len = (u32int)&floppy_dmaBuf;

  //get the length of the DMA
  count.len = (u32int)DMA_LEN - 1;

  /*check that address is at most 24-bits (under 16MB)
   * check that count is at most 16-bits (DMA limit)
   * check that if we add count and address, we do not get a carry
   * (DMA can't deal with such a carry, this is the 64k boundary limit) */
  if((addr.len >> 24) || (count.len >> 16) || (((addr.len & 0xffff) + count.len) >> 16))
  {
    k_printf("floppy_dma_init: DMA buffer problem\n");

    //give error message of what caused the DMA problem

    if(addr.len >> 24)
      k_printf("DMA physical address exceeds 16MB size\n");

    if(count.len >> 16)
      k_printf("DMA size is over 64KB limit\n");

    if(((addr.len & 0xffff) + count.len) >> 16)
      k_printf("DMA buffer cannot deal with carries, due to the 64KB limit\n");

    //dump useful information about the DMA count and address
    k_printf("DMA ERROR INFO: starting addr: %h, ending addr: %h, size %d bytes", addr.len, addr.len + count.len, count.len);

    //error
    return 1;
  }

  outb(0x0a, 0x06);           // mask DMA channel 2

  outb(0x0c, 0xff);           // reset flip-flop
  outb(0x04, addr.buf[0]);    // the beginning addr of the dma - address low byte
  outb(0x04, addr.buf[1]);    // the beginning addr of the dma - address high byte

  outb(0x0c, 0xff);           // reset flip-flop
  outb(0x05, count.buf[0]);   // the size of the dma - count low byte
  outb(0x05, count.buf[1]);   // the size of the dma - count high byte

  outb(0x81, addr.buf[2]);    // external page register

  outb(0x0a, 0x02);           // unmask chan 2

  k_printf("DMA initialized sucessfully!\n");

  //sucess!
  return 0;
}

void floppy_dma_read()
{
	outb(0x0a, 0x06); //mask dma channel 2
	outb(0x0b, 0x56); //single transfer, address increment, autoinit, read, channel 2
	outb(0x0a, 0x02); //unmask dma channel 2
}

void floppy_dma_write()
{
	outb(0x0a, 0x06); //mask dma channel 2
	outb(0x0b, 0x5a); //single transfer, address increment, autoinit, write, channel 2
	outb(0x0a, 0x02); //unmask dma channel 2
}

u32int floppy_seek(u32int cyl, u8int head)
{
  //_CurrentDrive can only have upto 4 drives, 0-3
  if(_CurrentDrive >= 4)
    return 0; //error

  u32int st0, cyl0, i;

  //the motor must be on to be able to seek
  floppy_motor(FLOPPY_MOTOR_ON, _CurrentDrive);

  //attempt to position to input cylinder 10 times until sucess
  for(i = 0; i < 10; i++)
  {
    /*to seek to a specific cylinder,
     * send seek command 0xf
     * 1st parameter byte: bits 0-1 are the drive, bit 2 is the head
     * 2nd parameter byte: the cylinder to set to */

    floppy_send_command(FDC_CMD_SEEK);
    floppy_send_command((head << 2) | _CurrentDrive);
    floppy_send_command(cyl);

    //wait for the interupt
    floppy_wait_irq();

    floppy_check_int(&st0, &cyl0);

    if(cyl0 == cyl)
    {
      //~ k_printf("Seek finished sucessfully\n");
      return 0; //sucess
    }
  }

  //loop has been exhausted 10 times, if it has not worked, abort
  k_printf("Floppy seek has been exhausted 10 times, error\n");
  floppy_motor(FLOPPY_MOTOR_OFF, _CurrentDrive);
  return 2; //error
}

void floppy_lba_to_chs(u32int lba, u32int *head, u32int *track, u32int *sector)
{

  *(head) = (lba % (FLOPPY_SECTORS_PER_TRACK * 2)) / (FLOPPY_SECTORS_PER_TRACK);
  *(track) = lba / (FLOPPY_SECTORS_PER_TRACK * 2);
  *(sector) = lba % FLOPPY_SECTORS_PER_TRACK + 1;

}

void floppy_read_sector(u8int head, u8int track, u8int sector, u32int size)
{
  u32int st0, cyl;

  //the floppy must be on in order to read from it
  floppy_motor(FLOPPY_MOTOR_ON, _CurrentDrive);

  floppy_dma_read();
  //~ k_printf("Floppy read sector, dma set to read\n");

  //send command that we want to read from a sector with some generic flags
  floppy_send_command(FDC_CMD_READ_SECT | FDC_CMD_EXT_MULTITRACK |
                      FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);

  //specify which head and drive to use
  floppy_send_command((head << 2) | _CurrentDrive);
  floppy_send_command(track);    //specify the track
  floppy_send_command(head);     //specify the head
  floppy_send_command(sector);   //specify the sector
  floppy_send_command(FLOPPY_SECTOR_DTL_512); //specify to use 512 bytes per sector

  /*if sector, for some reasong is larger than usual, use that value, else
   * use the default value for the amount of sectors per track */
  //~ floppy_send_command(((sector + 1) > FLOPPY_SECTORS_PER_TRACK) ?
                        //~ FLOPPY_SECTORS_PER_TRACK : sector + 1);

  /*send command to read the ceil of the number of sectors that make
   * up size (size - 1) / SECTOR_SIZE, add 1 to make it ceiling
   *
   * the -1 in (size - 1) is used if size == SECTOR_SIZE, then the output
   * of this calucation would be still 1 sector, without the -1, the ouput
   * would be 2 which is incorrect*/
  floppy_send_command((int)((size - 1) / SECTOR_SIZE) + 1);      

  //state that we are using a 3.5in floppy disk
  floppy_send_command(FLOPPY_GAP3_LENGTH_3_5);
  floppy_send_command(0xff); //floppy data length, set to 0xff if sector size != 0

  //wait for interupt
  floppy_wait_irq();

  //read the status info
  u32int i;
  for(i = 0; i < 7; i++)
    floppy_read_data();

  //let the FDC aknowledge that the interupt has been handled
  floppy_check_int(&st0, &cyl);
}

u8int *floppy_read(u32int sectorLBA, u32int size)
{
  //_CurrentDrive can only have upto 4 drives, 0-3
  if(_CurrentDrive >= 4)
    return 0; //error


  //Convert the linear block address (LBA) sector to cyliner, head, sector (CHS)
  u32int head = 0, track = 0, sector = 1;
  floppy_lba_to_chs(sectorLBA, &head, &track, &sector);

  //~ k_printf("Floppy read, seeking head\n");
  if(floppy_seek(track, head))
  {
    k_printf("Floppy read seek head returned with error, aborted\n");
    return 0; //error
  }else
    //~ k_printf("Head seeked sucessfully\n");

  /*after completing floppy_read_sector, the data for that sector should
   * be stored in the dma buffer */
  floppy_read_sector(head, track, sector, size);

  //We are done reading, shut the motor off
  floppy_motor(FLOPPY_MOTOR_OFF, _CurrentDrive);

  /*return the pointer to the dma buffer, this is where the data we got
   * from the floppy is stored */
  return(u8int*)floppy_dmaBuf;
}

void floppy_write_sector(u8int head, u8int track, u8int sector, u32int size)
{

  //size cannot exceed DMA_LEN, if greater, print warning and set to DMA_LEN
  if(size > DMA_LEN)
  {
    size = DMA_LEN;
    k_printf("WARNING, floppy wanted to write a chunck of data larger than DMA_LEN (18KB)\n");
  }
  u32int st0, cyl;

  //the floppy must be on in order to read from it
  floppy_motor(FLOPPY_MOTOR_ON, _CurrentDrive);

  floppy_dma_write();
  //~ k_printf("Floppy write sector, dma set to write\n");

  //send command that we want to read from a sector with some generic flags
  floppy_send_command(FDC_CMD_WRITE_SECT | FDC_CMD_EXT_MULTITRACK |
                      FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);

  //specify which head and drive to use
  floppy_send_command((head << 2) | _CurrentDrive);
  floppy_send_command(track);    //specify the track
  floppy_send_command(head);     //specify the head
  floppy_send_command(sector);   //specify the sector
  floppy_send_command(FLOPPY_SECTOR_DTL_512); //specify to use 512 bytes per sector

  //~ k_printf("Sectors writing to %d, track %d, head %d\n", sector + 1, track, head);

  /*send command to write the ceil of the number of sectors that make
   * up size (size - 1) / SECTOR_SIZE, add 1 to make it ceiling
   *
   * the -1 in (size - 1) is used if size == SECTOR_SIZE, then the output
   * of this calucation would be still 1 sector, without the -1, the ouput
   * would be 2 which is incorrect*/
  floppy_send_command((int)((size - 1) / SECTOR_SIZE) + 1);                      

  //state that we are using a 3.5in floppy disk
  floppy_send_command(FLOPPY_GAP3_LENGTH_3_5);
  floppy_send_command(0xff); //floppy data length, set to 0xff if sector size != 0

  //wait for interupt
  floppy_wait_irq();

  //read the status info
  u32int i;
  for(i = 0; i < 7; i++)
    floppy_read_data();

  //let the FDC aknowledge that the interupt has been handled
  floppy_check_int(&st0, &cyl);
}

u8int *floppy_write(u32int *data, u32int size, u32int sectorLBA)
{

  //_CurrentDrive can only have upto 4 drives, 0-3
  if(_CurrentDrive >= 4)
    return 0; //error

  //if no persistent storage is enabled, then it is most likly we do not want to write data to any drive
  if(_FloppyStorage == FALSE)
    k_printf("%cr**WARNING** Floppy writing to boot floppy%cw\n"); //print a warning
    
  memset(floppy_dmaBuf, 0, DMA_LEN);
  memcpy(floppy_dmaBuf, data, size > DMA_LEN ? DMA_LEN : size);

  //Convert the linear block address (LBA) sector to cyliner, head, sector (CHS)
  u32int head = 0, track = 0, sector = 1;
  floppy_lba_to_chs(sectorLBA, &head, &track, &sector);

  //~ k_printf("Floppy write, seeking head\n");
  if(floppy_seek(track, head))
  {
    k_printf("Floppy write, seek head returned with error, aborted\n");
    return 0; //error
  }//else
    //~ k_printf("Head seeked sucessfully\n");

  /*after completing floppy_read_sector, the data for that sector should
   * be stored in the dma buffer */
  floppy_write_sector(head, track, sector, size);

  //We are done reading, shut the motor off
  floppy_motor(FLOPPY_MOTOR_OFF, _CurrentDrive);

  /*return the pointer to the dma buffer, this is where the data we got
   * from the floppy is stored */
  return(u8int*)floppy_dmaBuf;  
}
