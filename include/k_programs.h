/*
 * k_programs.h
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

#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <system.h>

/*function that gets input index and runs corresponding function*/
void runShellFunction(int runFunction, char *arguements, u32int priority, u32int burst_time);

/*program list starts here*/
void program_ascii(char *arguements); //0 index --prints ascii animals

void program_echo(char *arguements); //1 index --prints the arg put in

void program_tinytext(char *arguements); //2 index --very simple text editor

//PONG//
void program_GUI_pong(char *arguements); //3 index --play pong
void movePongBall();
void pongRestart();
//PONG//

void program_song(char *arguements); //4 index --plays some small tunes

void program_JS_viewer(char *arguements); //5 index --a simple image viewer

void program_start(char *arguements); //6 index --starts programs (like start X)

void program_ls(char *arguements); //7 index --lists the files in a current directory

int program_cd(char *arguements); //8 index --changes directory

void program_now(char *arguements); //9 index --gives the current hr:min:sec month/day/year

void program_mkdir(char *arguements); //10 index --creates a directory

void program_cp(char *arguements); //11 index --copies a file to dest (2 argument)

void program_cat(char *arguements); //12 index --prints the contents of a file, byte by byte

void program_rm(char *arguements); //13 index --removes a file

void program_pwd(char *arguements); //14 index --prints the path from root to the current dir

void program_help(char *arguements); //15 index --prints help contents

void program_mv(char *arguements); //16 index --move (rename) a file to a dest

void program_find(char *arguments); //17 index --find a file in a directory

#endif //PROGRAMS_H

