/*
 * make_initrd.c
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

// Based on code from JamesM's kernel development tutorials.

#include <stdio.h>

#define NUMBER_OF_HEADERS	     64

struct initrd_header
{
	unsigned char magic;
	char name[128];
	unsigned int offset;
	unsigned int length;
};

int main(char argc, char **argv)
{
	//get the number of files user wants to have in initrd
	int nheaders = (argc - 1) / 2;
	struct initrd_header headers[NUMBER_OF_HEADERS];
	printf("size of header: %d\n", sizeof(struct initrd_header));

	//offset = NUMBER_OF_HEADERS structs of the initrd_header + the one integer that tells the number of headers
	unsigned int off = sizeof(struct initrd_header) * NUMBER_OF_HEADERS + sizeof(int);

	int i;
	for(i = 0; i < nheaders; i++)
	{
		printf("writing file %s->%s at 0x%x\n", argv[i*2+1], argv[i*2+2], off);
		strcpy(headers[i].name, argv[i*2+2]);

		//add terminating 0 to name of file
		headers[i].name[strlen(argv[i*2+2])] = 0;

		//write the offset
		headers[i].offset = off;
		FILE *stream = fopen(argv[i*2+1], "r");
		if(stream == 0)
		{
			printf("Error: file not found: %s\n", argv[i*2+1]);
			return 1;
		}

		//finds the length of the file and saves it to headers[i].length
		fseek(stream, 0, SEEK_END);
		headers[i].length = ftell(stream);
		//finds the length of the file and saves it to headers[i].length

		off += headers[i].length;
		fclose(stream);
		headers[i].magic = 0xBF;
	}

	FILE *wstream = fopen("./initrd.img", "w");
	//~ unsigned char *data = (unsigned char *)malloc(off);

	//write the nheaders to the first sizeof(int) of the initrd.img file
	fwrite(&nheaders, sizeof(int), 1, wstream);

	//write the header information after the nheaders integer ini intird.img
	fwrite(headers, sizeof(struct initrd_header), NUMBER_OF_HEADERS, wstream);

	for(i = 0; i < nheaders; i++)
	{
		FILE *stream = fopen(argv[i*2+1], "r");

		//create a FILE buffer in the memory that will store the data of stream
		unsigned char *buf = (unsigned char *)malloc(headers[i].length);

		//write the contents of stream to out FILE buffer
		fread(buf, 1, headers[i].length, stream);

		//write that data in the FILE buffer to wstream
		fwrite(buf, 1, headers[i].length, wstream);
		
		fclose(stream);
		free(buf);
	}

	fclose(wstream);
	//~ free(data);

	return 0;
}
