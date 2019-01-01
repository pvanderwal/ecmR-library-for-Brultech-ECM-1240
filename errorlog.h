/*************************************************************************
 errorLog.h library ETSD time series database 

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


#ifndef __errorlog_h__
#define __errorlog_h__

#ifdef __cplusplus
extern "C" {
#endif

// Error Codes, these are a subset of the ETSD error codes.  If using with ETSD, use the errorlog files included with ETSD
// Error Codes 1-16 are Info, 32-512 are warnings are 'Notices' not displayed if LogLvl is less than 2
// error levels between 1024 and 4096 are 'Warnings', generally operator error, 2048 and up are hard errors
#define E_EOF             2     // End of file reached
#define E_NOT_TTY      2048     // Device specified is not a TTY device
#define E_TTY_STAT     8192     // Can't get status of tty device
#define E_SHM         16384     // Can't open/create shared memory object
#define E_CHECKSUM   262144     // Check Sum error on ECM packet
#define E_TIMEOUT    524288     // Timed out waiting on data from ECM-1240
#define E_CODING  2147483648    // Unspecified coding error, program shouldn't get to the point that generated this error

//LogLvl  0 = no logging, 1 = minimal error logging, 2 = detailed error logging, 3 = log data output, 4 log data input
extern int8_t LogLvl;
extern uint32_t ErrorCode;

// only call if using a logfile instead of syslog.
void LogSetup(const char *fName);

// cfnm = calling function name + optional text
// Clear = clear ErrorCode before returning unless zero
void ELog(const char *cfnm, uint8_t clear);

// uses printf() formating
void Log(const char *format, ...);

// Logs an array of data in hex format, 16 bytes per line
// "type" is a short (<5 char) description, size is the size of the array in bytes.
void LogBlock(void *array, char *type, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif 
