/*
 * websys.c
 * 
 * Copyright 2017  <pi@raspberrypi>
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#define PI_IS_ON 1
#define ERROR_HALT 1
#define FILE_DEBUG 0
#define USE_WPI 1
#define OPTION_COUNT 1
#define MAX_STRING 200
#define MAX_FUNCTION_STRING 200
#define timeout 10

#define FUNCTION_PATH "/var/www/html/control/websysproc/pending.kmf"
#define WEB_PATH "/var/www/html/control/websysproc/webstate.kmf"

char function_name[MAX_FUNCTION_STRING] = {0};

void piusleep(int sleeptime);
void check_file(FILE* out_file);
void write_function(FILE* out_file, char* in_string);

unsigned int file_ready;

int main(int argc, char **argv)
{
	FILE* functionfile;
	
	//if(USE_WPI) if(wiringPiSetup() == -1) return 1;
	
	functionfile = fopen(FUNCTION_PATH, "r+b");
	if(functionfile != NULL){
		printf("----- Successfully Opened Function File -----\n");
	} else {
		printf("...Failed to open function file...\n");
		return EXIT_FAILURE;
	}
	
	if(argc ==  2) {
		if(FILE_DEBUG) printf("New Function!\n");
		
		check_file(functionfile);
		
		if(file_ready){
			if(FILE_DEBUG) printf("File Was Ready!\n");
			write_function(functionfile, argv[1]);
		} else {
			if(FILE_DEBUG) printf("File Wasn't Ready...Closing\n");
		}
	} else {
		if(FILE_DEBUG) printf("No Function...Closing\n");
	}
	
	return 0;
}

void check_file(FILE* out_file){
	int localcount;
	char tempval;
	char restart;
	
	localcount = 0;
	restart = 0;
	while (!file_ready && !restart && (localcount < timeout)){
		if(localcount){
			if(FILE_DEBUG) printf("Waiting before retry...\n");
			if(USE_WPI) delay(1000);
			else piusleep(1000000);
		}
		tempval = getc(out_file);
		if(tempval > 0){
			if(tempval < 48){
				if(FILE_DEBUG) printf("Invalid First...\n");
				file_ready = 0;
				localcount = localcount + 1;
				if (localcount == timeout) restart = 1;
				else restart = 0;
			} else {
				tempval = tempval - 48;
				if (tempval == 0){
					if(FILE_DEBUG) printf("First was 0!\n");
					file_ready = 1;
					restart = 0;
				}
				if (tempval == 1) {
					if(FILE_DEBUG) printf("First was 1...\n");
					file_ready = 0;
					restart = 0;
					localcount = localcount + 1;
				}
				if (tempval > 1) {
					if(FILE_DEBUG) printf("First was %d, not 0 or 1...\n", tempval);
					file_ready = 0;
					localcount = localcount + 1;
					if (localcount == timeout) restart = 1;
					else restart = 0;
				}
			}
		} else {
			if(FILE_DEBUG) printf("Empty File...\n");
			file_ready = 0;
			restart = 1;
		}
		rewind(out_file);
	}
	
	if(!file_ready){
		if(restart){
			if(FILE_DEBUG) printf("Restarting File...\n");
			putc('0', out_file);
			rewind(out_file);
			restart = 0;
			file_ready = 1;
		} else {
			if(FILE_DEBUG) printf("No Restart...\n");
		}
	}
}

void write_function(FILE* out_file, char* in_string){
	strcpy(function_name, in_string);
	if(FILE_DEBUG) printf("Function is %s\n", function_name);
	putc('0', out_file);
	putc('%', out_file);
	fprintf(out_file, "%s", function_name);
	putc('%', out_file);
	rewind(out_file);
	putc('1', out_file);
	rewind(out_file);
}

#define microtime 88
void piusleep(int sleeptime){
	int localcount;
	while(sleeptime){
		localcount = microtime;
		while(localcount){
			 localcount = localcount - 1;
		 }
		 sleeptime = sleeptime - 1;
	}
}
