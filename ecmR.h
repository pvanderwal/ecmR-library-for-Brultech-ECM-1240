/*************************************************************************
ecmR.h library used to receive and parse serial data from an Brultech ECM-1240

Copyright 2018 Peter VanDerWal 

    Note: this code was writen using information from Brultech's document: ECM1240_Packet_format_ver9.pdf
    Brultech has kindly agreed to let me release this code as 'opensource' software.
    However, I am not a lawyer and since this code was derived from Brultech's proprietary information, 
    and I don't have the right to 'give away' their proprietary information, if you intend to use 
    or distribute this code in a commercial application, you should probably contact Brultech first.  
    https://www.brultech.com/contact/
    
    This code is free: with the above stipulation you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2.0 as published by
    the Free Software Foundation.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*********************************************************************************/
#ifndef __ecmR_h__
#define __ecmR_h__

//	#include <stdarg.h>
#ifdef _WINDOWS
#define strcasecmp stricmp
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef union {
	uint32_t longD[16];  //13
	uint16_t data[32];   //26
	uint8_t byteD[64];   //52
    char Char[64];
} DATA_UNION;

extern DATA_UNION *dataU;

#define INTERVAL dataU->byteD[56]
#define DATA_VALID dataU->byteD[57]   //zero = valid, 1 = updating, 2=initializing, other numbers = error code
#define PACKET_POSITION dataU->byteD[58]
#define RX_DATA dataU->byteD[59]

// ecmSetup() should only be called by the master program that is connected to the ecm tty port
// all other programs call ecmConnect()
// return values 0=success, -1= error see errorlog.h for details
int8_t ecmSetup (char* tty_port, char* shmAddr);

// used by client programs that are connecting to ecm Shared Memory object 
// return values 0=success, -1= error see errorlog.h for details
int8_t ecmConnect (char* shmAddr);

// ecmR() should only be called by the master program that is connected to the ecm tty port
// return codes: 0 = rx good packet, 3 = checksum error, 4 = timed out no data, 5 = unspecified error 
// timeOut is in 1/10s of a second, only need interval to populate SHM object
uint8_t ecmR(uint8_t timeOut, uint8_t interval);

// channel  1-4 = Ch1A, Ch2A, Ch1P, Ch2P, channel 5-9 = Aux1-Aux5, 
// channel: 0 = seconds, -1 = Ch1&Ch2 amps, -2 = serial no. 
uint32_t ecmGetCh( int8_t  channel);

// 1-5 returns Aux data, -1 returns seconds counter from ECM
uint32_t  ecmGetAux( int8_t auxCh);

//0=DC Voltage, 1 = AC Voltage, 14 =  Ch1 Amps, 15 = Ch2 Amps
uint16_t ecmGetVolt ( uint8_t addr);

uint8_t ecmGetByte ( uint8_t addr);

// 0 returns as soon as DATA_VALID is valid, negative numbers wait for DATA_VALID on next interval, 
// positive returns when interval = 'until'
// returns current interval
uint8_t ecmWaitUntilValid (int8_t until);

#ifdef __cplusplus
}
#endif

#endif
