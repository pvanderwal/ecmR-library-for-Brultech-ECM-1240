// ECM receiver  shared library

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>		//usleep
#include <sys/stat.h> /* For mode constants */ 
#include <sys/mman.h>
#include <fcntl.h>
#include <termios.h>

#include "ecmR.h" 
#include "errorlog.h"

struct termios tio;
DATA_UNION *dataU;

int tty_fd;  		// Serial port file descriptor
int shm_fd;         // shared memory file descriptor 

const uint32_t SHM_SIZE = 64; /* the size (in bytes) of shared memory object */

// return values 0=success, -1 = error, ErrorCode contains error number see errorlog.h
// ecmSetup() should only be called by the master program that is connected to the ecm tty port
// all other programs call ecmConnect()
int8_t ecmSetup (char* tty_port, char* shmAddr){
    ELog(__func__, 1);  //log any existing errors and zero ErrorCode
    if (*shmAddr == '\0') { //not using shared memory
        dataU = malloc(sizeof(DATA_UNION));
    } else {
        shm_fd = shm_open(shmAddr, O_RDWR, 0666); /* open the shared memory object */
        if (shm_fd < 0) {       // error opening shared memory object, probably doesn't exist
            //perror("shm_open()");
            shm_fd = shm_open(shmAddr, O_CREAT | O_EXCL| O_RDWR , 0666);
            if (shm_fd < 0) {
                perror("ecmSetup()");
                ErrorCode |= E_SHM;  
                return -1;
            }
            ftruncate(shm_fd, SHM_SIZE);
        }
        dataU = (DATA_UNION *)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        DATA_VALID = 2 ; // 2 = initiating, data not valid  Note: this does NOT indicate an error
    }

    
    tty_fd = open(tty_port, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    
    if(!isatty(tty_fd)) { 	
        ErrorCode |= E_NOT_TTY;
		return -1;
    }
    if(tcgetattr(tty_fd, &tio) < 0) {
        ErrorCode |= E_TTY_STAT;
		return -1;
    }
	 
    cfmakeraw(&tio); 
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;

    cfsetospeed(&tio,B19200);            // set baud, normally 19200 on ECM1240
    cfsetispeed(&tio,B19200);            
           
    tcsetattr(tty_fd,TCSANOW,&tio);
    tcflush(tty_fd,TCIOFLUSH);              //clear any old data out of buffer, don't know when it arrived so it would screw up start time. 
	return 0;
}

// used by client programs that are connecting to ecm Shared Memory object 
int8_t ecmConnect (char* shmAddr){
    ErrorCode &= ~E_SHM;
    shm_fd = shm_open(shmAddr, O_CREAT | O_EXCL| O_RDWR , 0666);
    if (shm_fd < 0) {
        perror("ecmConnect()");
        ErrorCode |= E_SHM;  
        return -1;
    }
    dataU = (DATA_UNION *)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	return 0;
}

// ecmR() should only be called by the master program that is connected to the ecm tty port
// timeOut in 1/10 second increments
// DATA_VALID codes: 0 = rx good packet, 1=updating, 2 = checksum error, 4 = timed out no data, 8 = unspecified error 
uint8_t ecmR(uint8_t timeOut, uint8_t interV) {
    uint_fast16_t loop=0, lastRx=0; 
    uint_fast8_t mp=2;
    time_t now;
    uint8_t header = 0;
    uint8_t checksum;

    DATA_VALID = 1;
    INTERVAL = interV;
    PACKET_POSITION = 0;
    dataU->byteD[35] = 0; 

    ELog(__func__, 1);  //log any existing errors and zero ErrorCode

    while (1) {
        while (read(tty_fd,&RX_DATA,1)>0) {
            if (!PACKET_POSITION) {  // in header or lost.
                if (0xFE == RX_DATA) {
                    header = 1;
                } else {
                    if (header == 1) {
                        if (0xFF == RX_DATA) {
                            header = 2;
                        } else header = 0;
                    } else if (header == 2) {  //header = 2
                        if (0x03 == RX_DATA) {
                            PACKET_POSITION = 4;  // AC voltage is first actual value
                            checksum = 0; // 0 == (0xFE + 0xFF + 0x03)&0xFF ;
                        } 
                    } else
                        header = 0;
                }
            } else {
                if (65 == PACKET_POSITION) {
                    break;
                } 
                if (30 == PACKET_POSITION) {
                    dataU->data[1] = dataU->byteD[2] << 8 | dataU->byteD[3] ; // make AC Voltage little endian, Brultech sends this value bigendian
                    mp=24;
                }
                else if (41 == PACKET_POSITION) {
                    mp++;
                } else if (61 == PACKET_POSITION)
                    mp = 0;
                if (63 > PACKET_POSITION) 
                    dataU->byteD[mp++] = RX_DATA;
                checksum += RX_DATA;
                PACKET_POSITION++;
            }
            lastRx=loop;
        } // end of while(read )
            
        if (65 == PACKET_POSITION) { //rxed full packet
            PACKET_POSITION = checksum;
            time(&now);
            dataU->longD[15] = now;
            if (checksum == RX_DATA) {
                DATA_VALID = 0; // good packet      
            } else {
                if(0xFE== RX_DATA)
                    read(tty_fd,&RX_DATA,1);
                if (checksum == RX_DATA)
                    DATA_VALID = 0; // good packet 
                else {
                    ErrorCode |= E_CHECKSUM;  //checksum error
                    DATA_VALID = (uint8_t)E_CHECKSUM;
                }
            }
            return DATA_VALID?-1:0; 
        }
        if (loop > timeOut ) {
            ErrorCode |= E_TIMEOUT;
            DATA_VALID = (uint8_t)E_TIMEOUT;
            return -1; 
        } 
        usleep(100000);  //100,000 microseconds = 1/10 second
        loop++;
    } //end while(1)
    ErrorCode |= E_CODING; // Coding error, program should never reach this point
    DATA_VALID = (uint8_t)E_CODING;
    return -1;
}

// channel  1-4 = Ch1A, Ch2A, Ch1P, Ch2P, channel 5-9 = Aux1-Aux5, 
// channel: 0 = seconds, -1 = Ch1&Ch2 amps, -2 = serial no. 
uint32_t ecmGetCh( int8_t  chan) {
	int_fast8_t  lp;
	union {
		uint32_t val;
		uint8_t cpy[4];
	} rval;
    
	if (1 > chan)
        return dataU->longD[ chan + 8];
    
    if (4 < chan)
        return dataU->longD[ chan + 4];
    chan = chan*5-1;
	for (lp = 0; 4 > lp; lp++) {
		rval.cpy[lp] = dataU->byteD[chan++];
	}
    
	return rval.val;
}
 
// 1-5 returns Aux data, -1 returns seconds counter from ECM
uint32_t  ecmGetAux( int8_t auxCh) {
	return dataU->longD[ auxCh + 8];
}

//0=DC Voltage, 1 = AC Voltage, 14 =  Ch1 Amps, 15 = Ch2 Amps
uint16_t ecmGetVolt ( uint8_t addr) {
    return dataU->data[addr];
}

uint8_t ecmGetByte ( uint8_t addr) {
    return dataU->byteD[addr];
}

// 0 returns as soon as DATA_VALID is valid, negative numbers wait for DATA_VALID on next interval, 
// positive returns when interval = 'until'
// returns current interval when DATA_VALID
uint8_t ecmWaitUntilValid (int8_t until) {
    uint8_t interV = INTERVAL;
    while(1) {
        if (DATA_VALID) {
            if (until) {
                if (until > 0) {
                    if (until == INTERVAL)
                        break;
                } else {
                    if (interV != INTERVAL)
                        break;
                }
            } else {
                break; 
            }
        }
        usleep(10000);  //10,000 microseconds = 1/100 second 
    }        
    return INTERVAL;
}