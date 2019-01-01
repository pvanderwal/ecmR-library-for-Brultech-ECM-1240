/*************************************************************************
 errorLog.c library ETSD time series database 

Copyright 2018 Peter VanDerWal 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2.0 as published by
    the Free Software Foundation
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*********************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>     /* va args */
#include <time.h>
#include <string.h>

#include "errorlog.h"

char *LogFile;
int8_t LogLvl;
uint32_t ErrorCode;

char *errLvl[]={
    "<0>Panic",
    "<1>Alert",
    "<2>Critical",
    "<3>Error",
    "<4>Warning",
    "<5>Notice",
    "<6>Info",
    "<7>Debug"
};

// only required if using a logfile instead of syslog.
void LogSetup(const char *fName){
    LogFile = (char *) malloc(strlen(fName)+1);
    strcpy(LogFile,fName); 
    ErrorCode = 0;
}

// cfnm = calling function name + optional text
// Clear = clear ErrorCode before returning unless zero
void ELog(const char *cfnm, uint8_t clear){
    uint8_t idx=0, eLvl=5;
    uint32_t eCode=ErrorCode;

    char *summary[]={
        "",
        "End of file reached",
        "", "", "", "", "", "", "", "", "",
        "Device specified is not a TTY device",
        "",
        "Can't get status of tty device",
        "Can't open/create shared memory object",
        "", "", "",
        "Check Sum error",
        "Timed out waiting on serial data",
        "", "", "", "", "", "", "", "", "", "", "", 
        "Unspecified coding error"
        };

    if(0<LogLvl){       // only log errors when log level is greater than 0
        if(2>LogLvl){   // only log <5>Notice' level errors if LogLvl is 2 or higher
            eCode >>= 10;
            idx=10;
        }
        while(eCode){
            if(eCode&1)
                if(9<idx) { // '<5>Notice' level errors are bits 0-9
                    if(12<idx)
                        eLvl=5; // '<3>Error' is bits 13-31
                    else        
                        eLvl=4;  // '<4>Warning' is levels bits 10-12
                } 
                Log("%s: %s %s \n", errLvl[eLvl], cfnm, summary[idx]);
            eCode >>= 1;
            idx++;
        }
    }
    if(clear)
        ErrorCode = 0;
}

// uses printf() formating
void Log(const char *format, ...){
    time_t now;
    char *currentTime;
    FILE *pLog = 0;
    va_list args;
    
    time(&now);

    if (NULL != LogFile) 
        pLog=fopen(LogFile,"a");
    va_start(args, format);
    
    if (pLog > 0) {
        currentTime = ctime(&now);
        currentTime[strlen(currentTime)-1] = '\0';
        fprintf(pLog, "%s ", currentTime);
        vfprintf (pLog, format, args);
    } else {
        //printf("%s ", currentTime);
        vfprintf (stderr, format, args);
    }
    va_end(args);
    if (pLog > 0)
        fclose (pLog);
}


// Logs an array of data in hex format, 16 bytes per line
// "type" is a short (<5 char) description, size is the array size.
void LogBlock(void *ptr, char *type, uint16_t size){
    char scratch[70], sblock[2000];
    uint8_t lp, lp2, *array = (uint8_t*)ptr; 
    uint16_t addr=0;
    
    sprintf(sblock, "<6> %4s block", type);
    for (lp=0; lp < (size+15)/16; lp++){
        sprintf(scratch, "\n[%2d] ", lp);
        strcat(sblock, scratch);
        for (lp2=0; lp2<16; lp2++){
            if (lp*16+lp2 > size)
                sprintf(scratch, "   ");
            else
                sprintf(scratch, "%02X ", array[lp*16+lp2]);
            strcat(sblock, scratch);
            if (7==lp2)
                strcat(sblock, "| ");
        }
    }
    strcat(sblock,"\n");
    Log(sblock);
}
