
#include "sys_control.h"
#include "sys_config.h"
#include "sys_functions.h"

// function name to match web call string
char function_name[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
     "kmfsetport",
     "kmfsetip",
     "kmfsi",
     "kmfac",
     "kmfrc",
     "kmfoc",
     "kmfsid",
     "kmfrid",
     "pingsend",
     "pingreply",
     "starttightvnc",
     "stoptightvnc",
     "startvideo",
     "stopvideo",
     "startaudio",
     "stopaudio"
 };
    
// function call for system
char function_call[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
     "",
     "",
     "",
     "",
     "",
     "",
     "",
     "tightvncserver",
     "pkill Xtightvnc",
     "omxplayer -o hdmi -b \"/home/pi/linux-main-share/MovieHD/\" </usr/share/myfolder/mysysproc/moviectrl/omxctrl >/usr/share/myfolder/mysysproc/moviectrl/omxlog & >/usr/share/myfolder/mysysproc/moviectrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
     "echo -n \"q\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
     "omxplayer -o hdmi -b \"/home/pi/linux-main-share/MusicHD/\" </usr/share/myfolder/mysysproc/musicctrl/omxctrl >/usr/share/myfolder/mysysproc/musicctrl/omxlog & >/usr/share/myfolder/mysysproc/musicctrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl",
     "echo -n \"q\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl"
 };
 
// length of the function call strings
int function_length[FUNCTION_COUNT] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
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
    // 4 - config
    // 5 - function
    0x18, // config extra text
    0x18,
    0x8,
    0x8,
    0x8,
    0x0,
    0x0,
    0x2, // starter
    0x4, // stopper
    0xA, // starter with extra text
    0x4, // stopper
    0xA, // starter with extra text
    0x4 // stopper
};

// option_status that "start" or "stop" is linked to
int option_link[FUNCTION_COUNT] = {
    0,
    1,
    0,
    0,
    0,
    0,
    0, // option 0 - tightvncserver
    0, // option 0 - tightvncserver
    1, // option 1 - startvideo
    1, // option 1 - stopvideo
    2, // option 2 - startaudio
    2  // option 2 - stopaudio
};

// extra_text_offset
char extra_offset[FUNCTION_COUNT] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    56,
    0,
    56,
    0
};

int option_status[OPTION_COUNT] = {0};
char option_name[OPTION_COUNT][MAX_STRING] = {
    {10},
    {8},
    "tightvncserver",
    {10}, // length of command constant
    {10} // length of command constant
};

void checkfunctionfile(char use_timeout){
    int newfunction = 0;
    int functionready = 0;
    int localcount = 0;
    int in_file = 0;
    //char clearfile = 0;
    char fend = 0;
    pid_t forkid;
    //int clrcnt = 0;
    
    if(use_timeout){
      fvalid = 0;
      forkid = fork();
    } else {
       forkid = 1;
    }
    if(forkid == 0){
        // child id -- timeout catch
        printf("called fork!\n");
        sleep(10);
        in_file = open(RX_PATH, O_WRONLY, 0x0);
        write(in_file, "T", 1);
        exit(EXIT_SUCCESS);
    } else {
        // Parent id -- read and handle timeout
        in_file = open(RX_PATH, O_RDONLY, 0x0);
        //newfunction = fscanf(in_file, "%400[^\n]", filestring);
        newfunction = read(in_file, filestring, 400);
        
        if(FILE_DEBUG) printf("Read: %d, String: %s\n", newfunction, filestring);
    //  rewind(in_file);
        if(newfunction > 0){
            //clearfile = 1;
            if(filestring[0] == 'T'){
                 kill(forkid, SIGKILL);
                 printf("Timeout!\n");
            } else {
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
        //if(clearfile) {
        //  clearfile = 0;
        //  fputc('0', in_file);
        //  fputc('\0', in_file);
        //  rewind(in_file);
        //  for(clrcnt=0;clrcnt<MAX_STRING;clrcnt++) filestring[clrcnt] = 0;
        //}
        close(in_file);
        process_function();
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
            if(function_type[localcount] == 0x18){
                if(option_link[localcount] < OPTION_COUNT){
                     extra_cnt = func_const;
                     tmp_cnt = 0;
                     for(cust_cnt=extra_cnt;cust_cnt<flength;cust_cnt++){
                        cust_func[tmp_cnt] = funcstring[cust_cnt];
                        tmp_cnt = tmp_cnt + 1;
                     }
                     printf("String: %s\n", cust_func);
                     if(localcount == 0) web_port_update(&cust_func[0]);
                     if(localcount == 1) web_server_ip_update(&cust_func[0]);
                     if(localcount == 2) web_add_client(&cust_func[0]);
                     if(localcount == 3) web_remove_client(&cust_func[0]);
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

void transmit(int tx_port, char* tx_ip, char* tx_data){
   
}
