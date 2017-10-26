/*
 * mysys.c
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
#define FILE_DEBUG 1
#define USE_WPI 1
#define OPTION_COUNT 3
#define FUNCTION_COUNT 6
#define MAX_STRING 400
#define MAX_FUNCTION_STRING 400

#define FUNCTION_PATH "/var/www/html/control/websysproc/pending.kmf"
#define WEB_PATH "/var/www/html/control/websysproc/webstate.kmf"

// function name to match web call string
char function_name[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
		"starttightvnc",
		"stoptightvnc",
		"startvideo",
		"stopvideo",
		"startaudio",
		"stopaudio",
	};
// function call for system
char function_call[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
		"tightvncserver",
		"pkill Xtightvnc",
		"omxplayer -o hdmi -b \"/home/pi/linux-main-share/MovieHD/\" </usr/share/myfolder/mysysproc/moviectrl/omxctrl >/usr/share/myfolder/mysysproc/moviectrl/omxlog & >/usr/share/myfolder/mysysproc/moviectrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
		"echo -n \"q\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
		"omxplayer -o hdmi -b \"/home/pi/linux-main-share/MusicHD/\" </usr/share/myfolder/mysysproc/musicctrl/omxctrl >/usr/share/myfolder/mysysproc/musicctrl/omxlog & >/usr/share/myfolder/mysysproc/musicctrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl",
		"echo -n \"q\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl"
	};
// length of the function call strings
int function_length[FUNCTION_COUNT] = {
	14,
	15,
	271,
	34,
	271,
	34
};
// type of call: 0 - no condition, 1 - start, 2 - stop, 3 - extra text
int function_type[FUNCTION_COUNT] = {
	// one hot options:
	// 0 - no options
	// 1 - starter
	// 2 - stopper
	// 3 - extra text
	0x2, // starter
	0x4, // stopper
	0xA, // starter with extra text
	0x4, // stopper
	0xA, // starter with extra text
	0x4  // stopper
};
// option_status that "start" or "stop" is linked to
int option_link[FUNCTION_COUNT] = {
	0, // option 0 - tightvncserver
	0, // option 0 - tightvncserver
	1, // option 1 - startvideo
	1, // option 1 - stopvideo
	2, // option 2 - startaudio
	2 // option 2 - stopaudio
};
// extra_text_offset
char extra_offset[FUNCTION_COUNT] = {
	0,
	0,
	56,
	0,
	56,
	0
};

int option_status[OPTION_COUNT] = {0};
char option_name[OPTION_COUNT][MAX_STRING] = {
	"tightvncserver",
	{10}, // length of command constant
	{10} // length of command constant
	};

void piusleep(int sleeptime);
void checkfunctionfile(FILE* in_file);
void updatewebstate(FILE* out_file);
void init_webstate(FILE* out_file);
void process_function(void);

char filestring[MAX_STRING];
char funcstring[MAX_STRING];
char fvalid = 0;
int flength = 0;

int main(int argc, char **argv)
{
	FILE* functionfile;
	FILE* webfile;
	
	//if(USE_WPI) if(wiringPiSetup() == -1) return 1;
	
	printf("\n\n----- Starting Kyle's System -----\n\n");
	
	functionfile = fopen(FUNCTION_PATH, "r+b");
	if(functionfile != NULL){
		printf("----- Successfully Opened Function File -----\n");
	} else {
		printf("...Failed to open function file...\n");
		return EXIT_FAILURE;
	}
	
	webfile = fopen(WEB_PATH, "r+b");
	if(webfile != NULL){
		printf("----- Successfully Opened Web File -----\n");
	} else {
		printf("...Failed to open web file...\n");
		return EXIT_FAILURE;
	}
	
	printf("----- Resetting Web State -----\n");
	init_webstate(webfile);
	
	printf("----- Starting Main Loop -----\n");
	while(PI_IS_ON){
		// wait, so pi only checks function file every second or so...
		if(USE_WPI) delay(2500);
		else piusleep(1000000);
		
		//check to see if file changed
		checkfunctionfile(functionfile);
		
		//process functions
		// have a function list
		// have function states that get sent to php site...
		process_function();
		
		//update web page file
		updatewebstate(webfile); // use hash to make sure states are stable
	}
	return 0;
}

void checkfunctionfile(FILE* in_file){
	int newfunction = 0;
	int functionready = 0;
	int localcount = 0;
	char clearfile = 0;
	char fend = 0;
	int clrcnt = 0;
	
	fvalid = 0;
	newfunction = fscanf(in_file, "%400[^\n]", filestring);
	if(FILE_DEBUG) printf("Read: %d, String: %s\n", newfunction, filestring);
	rewind(in_file);
	if(newfunction > 0){
		clearfile = 1;
		if(filestring[0] < 48){ // character
			if(FILE_DEBUG) printf("First was %c, not 0 or 1\n", filestring[0]);
			functionready = 0;
		} else {
			filestring[0] = filestring[0] - 48;
			if(filestring[0] == 1){
				if(FILE_DEBUG) printf("First was 1, Valid set\n");
				functionready = 1;
			} else {
				if(FILE_DEBUG) printf("First was %d, not 1\n", filestring[0]);
				functionready = 0;
			}
		}
	} else {
		if(FILE_DEBUG) printf("Couldn't Read File...\n");
		functionready = 0;
	}
	
	if(functionready){
		if(FILE_DEBUG) printf("Found Valid Function!\n");
		if(filestring[1] == '%'){
			if(FILE_DEBUG) printf("Second was %%!\n");
			fend = 0;
			while(!fend && (localcount < (MAX_FUNCTION_STRING - 2))){
				if(filestring[2 + localcount] == '%'){
					if(FILE_DEBUG) printf("Found end %%!\n");
					fvalid = 1;
					fend = 1;
					flength = localcount;
					if(FILE_DEBUG) printf("Function Is: %s\n", funcstring);
					if(FILE_DEBUG) printf("Function Length Is: %d\n", flength);
				} else {
					funcstring[localcount] = filestring[2 + localcount];
				}
				localcount = localcount + 1;
			}
		} else {
			if(FILE_DEBUG) printf("Second was %c, not %%\n", filestring[1]);
		}
	}
	if(clearfile) {
		clearfile = 0;
		fputc('0', in_file);
		fputc('\0', in_file);
		rewind(in_file);
		for(clrcnt=0;clrcnt<MAX_STRING;clrcnt++) filestring[clrcnt] = 0;
	}
}

void process_function(void){
	int localcount = 0;
	char match = 0;
	int func_const = 0;
	int total_cnt = 0;
	int extra_cnt = 0;
	int cust_cnt = 0;
	int tmp_cnt = 0;
	char cust_func[MAX_FUNCTION_STRING] = {0};
	if(fvalid){
		if(FILE_DEBUG) printf("Function: %s\n", funcstring);
		
		for(localcount=0;localcount<FUNCTION_COUNT;localcount=localcount+1){
			if(function_type[localcount] & 0x8){ // check for extra text type
				func_const = option_name[option_link[localcount]][0];
				if(!strncmp(funcstring,function_name[localcount],func_const)){
					match = 1;
					break;
				}
			} else {
				if(!strcmp(funcstring,function_name[localcount])){
					match = 1;
					break;
				}
			}
		}
		if(match == 1){
			if(function_type[localcount] == 0){
				system(function_call[localcount]);
			}
			if(function_type[localcount] == 0x2){
				if(option_link[localcount] < OPTION_COUNT){
					if(option_status[option_link[localcount]] == 0){
						option_status[option_link[localcount]] = 1;
						system(function_call[localcount]);
					} else {
						printf("wrong state to process\n");
					}
				} else {
					printf("link is wrong\n");
				}
			}
			if(function_type[localcount] == 0x4){
				if(option_link[localcount] < OPTION_COUNT){
					if(option_status[option_link[localcount]] == 1){
						system(function_call[localcount]);
						option_status[option_link[localcount]] = 0;
					}else {
						printf("wrong state to process\n");
					}
				} else {
					printf("link is wrong\n");
				}
			}
			if(function_type[localcount] == 0xA){
				if(option_link[localcount] < OPTION_COUNT){
					if(option_status[option_link[localcount]] == 0){
						if(FILE_DEBUG) printf("flength: %d, offset: %d\n", flength, extra_offset[localcount]);
						extra_cnt = flength - func_const;
						total_cnt = extra_cnt + function_length[localcount];
						if(FILE_DEBUG) printf("extra: %d, tot: %d\n", extra_cnt, total_cnt);
						tmp_cnt = 0;
						for(cust_cnt=0;cust_cnt<total_cnt;cust_cnt++){
							if(cust_cnt < extra_offset[localcount]){
								cust_func[cust_cnt] = function_call[localcount][cust_cnt];
							} else {
								if(cust_cnt < (extra_offset[localcount] + extra_cnt)){
									cust_func[cust_cnt] = funcstring[func_const + tmp_cnt];
									tmp_cnt = tmp_cnt + 1;
								} else {
									cust_func[cust_cnt] = function_call[localcount][cust_cnt - extra_cnt];
								}
							}
						}
						if(FILE_DEBUG) printf("custom is: %s\n", cust_func);
						system(cust_func);
						// make that function call with additional text too..
						option_status[option_link[localcount]] = 1;
						// have to fix/link status stuff with this
					}else {
						printf("wrong state to process\n");
					}
				} else {
					printf("link is wrong\n");
				}
			}
		} else {
			printf("Wasn't Known Function\n");
		}
		
		fvalid = 0;
		for(localcount = 0;localcount<MAX_FUNCTION_STRING;localcount=localcount+1){
			funcstring[localcount] = 0;
			filestring[localcount] = 0;
		}
	}
}

void updatewebstate(FILE* out_file){
	int localcount = 0;
	
	for(localcount=0;localcount<OPTION_COUNT;localcount=localcount+1){
		if(localcount > 0) fprintf(out_file, "\n");
		fprintf(out_file, "%s: %d", option_name[localcount], option_status[localcount]);
	}
	
	rewind(out_file);
}

void init_webstate(FILE* out_file){
	int localcount = 0;
	
	for(localcount=0;localcount<OPTION_COUNT;localcount=localcount+1){
		if(localcount > 0) fprintf(out_file, "\n");
		fprintf(out_file, "%s: %d", option_name[localcount], option_status[localcount]);
	}
	
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