/*
 * cat6612
 * Driver for CAT6612 HDMI Tx
 *
 * Copyright (C) 2010  VN Inc.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <glib.h>
#include <glib/gprintf.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>
#include "cat6612.h"
#include "hdmi_tx.h"

#define CAT6612_ID_VERSION "1.2.0.2"

const char CAT6612_ID[] = "$Version: "CAT6612_ID_VERSION"  (CAT6612 DRIVER) $";



static struct SyncEmbeddedSetting SyncEmbTable[] = {
 // {FMT,0x90, 0x91,
 //   VIC ,0x90, 0x91, 0x95, 0x96,0x97, 0xA0, 0xA1, 0xA2, 0xA3, 0x90_CCIR8,0x91_CCIR8    ,0x95_CCIR8,0x960_CCIR8   ,0x97_CCIR8  ,  PCLK    ,VFREQ},
    {   1 ,0xF0, 0x31, 0x0E, 0x6E,0x00, 0x0A, 0xC0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0xDD          ,0X00            , 25175000,60},
    {   2 ,0xF0, 0x31, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
    {   3 ,0xF0, 0x31, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
    {   4 ,0x76, 0x33, 0x6c, 0x94,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x76      ,0x33          ,0xDA      ,0x2A          ,0x10            , 74175000,60},
    // {   5 ,0x26, 0x4A, 0x56, 0x82,0x00, 0x02, 0x70, 0x34, 0x92, 0x46      ,0x94          ,0x57      ,0x83          ,0x00            , 74175000,60},
    {   5 ,0x26, 0x4A, 0x56, 0x82,0x00, 0x02, 0x70, 0x34, 0x92, 0x66      ,0x94          ,0xAD      ,0x05          ,0x10            , 74000000,60},
    {   6 ,0xE0, 0x1B, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   7 ,0xE0, 0x1B, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   8 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xFF          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   9 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xFF          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {  10 ,0xe0, 0x1b, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  11 ,0xe0, 0x1b, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  12 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xff          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  13 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xff          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  14 ,0x00, 0xff, 0x1e, 0x9A,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0x00      ,0xff          ,0x3D      ,0x35          ,0x10            , 54000000,60},
    {  15 ,0x00, 0xff, 0x1e, 0x9A,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0x00      ,0xff          ,0x3D      ,0x35          ,0x10            , 54000000,60},
 // {  16 ,0x06, 0xff, 0x56, 0x82,0x00, 0x04, 0x90, 0xFF, 0xFF, 0x00      ,0xff          ,0xFF      ,0xFF          ,0xFF            ,148350000,60},
    {  16 ,0xF6, 0xFF, 0x56, 0x82,0x00, 0x04, 0x90, 0xFF, 0xFF, 0xF6      ,0xFF          ,0xAD      ,0x05          ,0x10            ,148352000,60},
    {  17 ,0xA0, 0x1B, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x95          ,0x00            , 27000000,50},
    {  18 ,0x00, 0xff, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x95          ,0x00            , 27000000,50},
    {  19 ,0x46, 0x59, 0xB6, 0xDE,0x11, 0x05, 0xA0, 0xFF, 0xFF, 0x06      ,0xFF          ,0x6D      ,0xBD          ,0x33            , 74250000,50},
    {  20 ,0x66, 0x73, 0x0e, 0x3A,0x22, 0x02, 0x70, 0x34, 0x92, 0xC6      ,0xE6          ,0x1D      ,0x75          ,0x44            , 74250000,50},
    {  21 ,0xA0, 0x1B, 0x0A, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  22 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  23 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  24 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  25 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 54000000,50},
    {  26 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 54000000,50},
    {  27 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00            , 54000000,50},
    {  28 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00            , 54000000,50},
    {  29 ,0x04, 0xff, 0x16, 0x96,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x2D      ,0x2D          ,0x10            , 54000000,50},
    {  30 ,0x04, 0xff, 0x16, 0x96,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x2D      ,0x2D          ,0x10            , 54000000,50},
//  {  31 ,0x06, 0xff, 0x0e, 0x3a,0x22, 0x04, 0x90, 0xFF, 0xFF, 0x00      ,0xFF          ,0xFF      ,0xFF          ,0xFF            ,148500000,50},
    {  31 ,0xF6, 0xFF, 0x0E, 0x3A,0x22, 0x04, 0x90, 0xFF, 0xFF, 0xF6      ,0xFF          ,0x1D      ,0x75          ,0x44            ,148000000,50},
    {  32 ,0xF6, 0xFF, 0x7C, 0xA8,0x22, 0x04, 0x90, 0xFF, 0xFF, 0xF6      ,0xFF          ,0xF9      ,0x51          ,0x54            , 74000000,24},
    {  33 ,0xF6, 0xFF, 0x0E, 0x3A,0x22, 0x04, 0x90, 0xFF, 0xFF, 0xF6      ,0xFF          ,0x1D      ,0x75          ,0x44            , 74000000,25},
    {  34 ,0xF6, 0xFF, 0x56, 0x82,0x00, 0x04, 0x90, 0xFF, 0xFF, 0xF6      ,0xFF          ,0xAD      ,0x05          ,0x10            , 74000000,30},
    { 0xFF,0xFF, 0xff, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF      ,0xFF          ,0xFF      ,0xFF          ,0xFF            , 0       , 0}
} ;

static int reg_read(const uint8_t reg)
{
    char buf[10] = {0};
    int file;
    char filename[40];
    const gchar *buffer;
    int addr = 0x4c;        // The I2C address of the ADC
    buf[0] = reg;

    sprintf(filename,"/dev/i2c-1");
    if ((file = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    if (ioctl(file,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    if (write(file,buf,1) != 1) {
        /* ERROR HANDLING: i2c transaction failed */
        printf("Failed to write to the i2c bus.\n");
        buffer = g_strerror(errno);
        printf(buffer);
        printf("\n\n");
    }

    // Using I2C Read
     if (read(file,buf,1) != 1) {
         /* ERROR HANDLING: i2c transaction failed */
         printf("Failed to read from the i2c bus.\n");
         buffer = g_strerror(errno);
         printf(buffer);
         printf("\n\n");
     } else {
         printf("get reg %02x: %02x \n",reg,buf[0]);
     }
     close(file);

    return buf[0];
}

static int reg_write( const uint8_t reg,  const uint8_t data)
{
    char buf[10] = {0};
    int file;
    char filename[40];
    const gchar *buffer;
    int addr = 0x4c;        // The I2C address of the ADC
    buf[0] = reg;

    printf("set reg %02x to %02x\n",reg,data);

    sprintf(filename,"/dev/i2c-1");
    if ((file = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    if (ioctl(file,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    if (write(file,buf,1) != 1) {
        /* ERROR HANDLING: i2c transaction failed */
        printf("Failed to write to the i2c bus.\n");
        buffer = g_strerror(errno);
        printf(buffer);
        printf("\n\n");
    }

    // Using I2C Read
     buf[0]=data;
     if (write(file,buf,1) != 1) {
         /* ERROR HANDLING: i2c transaction failed */
         printf("Failed to read from the i2c bus.\n");
         buffer = g_strerror(errno);
         printf(buffer);
         printf("\n\n");
     }
     close(file);
	return 0;

 	//   return i2c_smbus_write_byte_data(cat6612->client, reg, data);
}

static int reg_set(const uint8_t reg, const uint8_t data)
{
    int ret;

    ret = reg_read(reg);
    if (ret < 0)
        return ret;
    return reg_write( reg, ret | data);
}

static int reg_clear(const uint8_t reg,const uint8_t data)
{
    int ret;

    ret = reg_read(reg);
    if (ret < 0)
        return ret;
    return reg_write(reg, ret & ~data);
}

static int reg_compare(const uint8_t reg, const uint8_t data, const uint8_t mask)
{
    unsigned char read_data = reg_read(reg);

    return (data == (read_data&mask)) ? 0 : -1 ;
}

static void config_AVI_InfoFrame(int VICID, int outputColorMode)
{
#define AVI_INFOFRAME_TYPE  0x02
#define AVI_INFOFRAME_VER  0x02
#define AVI_INFOFRAME_LEN 13
    unsigned char AVI_HB[3];
    unsigned char AVI_DB[AVI_INFOFRAME_LEN];
    unsigned char pixelrep = 0;
    int i;
    unsigned char ucData;

    AVI_HB[0] = AVI_INFOFRAME_TYPE|0x80;
    AVI_HB[1] = AVI_INFOFRAME_VER;
    AVI_HB[2] = AVI_INFOFRAME_LEN;

    switch(outputColorMode)
    {
        case FORMAT_YUV444:
            AVI_DB[0] = (2<<5)|(1<<4);
            break;
        case FORMAT_YUV422:
            AVI_DB[0] = (1<<5)|(1<<4);
            break ;
        case FORMAT_RGB444:
        default:
            AVI_DB[0] = (0<<5)|(1<<4);
            break ;
    }

    AVI_DB[1] = 8;
    if ((VICID == 1) || (VICID == 2) || (VICID == 6) || (VICID == 17) || (VICID == 21))
    {
        AVI_DB[1] |= (1<<4);
    }
    else
    {
        AVI_DB[1] |= (2<<4); // 4:3 or 16:9
    }

    if ((VICID == 6) || (VICID == 7) || (VICID == 21) || (VICID == 22))
	    pixelrep = 1;

    AVI_DB[1] |= (1<<6); // always ITU601
    AVI_DB[2] = 0;
    AVI_DB[3] = VICID;
    AVI_DB[4] = pixelrep & 3;
    AVI_DB[5] = 0;
    AVI_DB[6] = 0;
    AVI_DB[7] = 0;
    AVI_DB[8] = 0;
    AVI_DB[9] = 0;
    AVI_DB[10] = 0;
    AVI_DB[11] = 0;
    AVI_DB[12] = 0;

    reg_write( _0F_BankSel, 0x01);

    reg_write( _B1_58_AVIINFO_DB1, AVI_DB[0]);
    reg_write( _B1_59_AVIINFO_DB2, AVI_DB[1]);
    reg_write( _B1_5A_AVIINFO_DB3, AVI_DB[2]);
    reg_write( _B1_5B_AVIINFO_DB4, AVI_DB[3]);
    reg_write( _B1_5C_AVIINFO_DB5, AVI_DB[4]);
    reg_write( _B1_5E_AVIINFO_DB6, AVI_DB[5]);
    reg_write( _B1_5F_AVIINFO_DB7, AVI_DB[6]);
    reg_write( _B1_60_AVIINFO_DB8, AVI_DB[7]);
    reg_write( _B1_61_AVIINFO_DB9, AVI_DB[8]);
    reg_write( _B1_62_AVIINFO_DB10, AVI_DB[9]);
    reg_write( _B1_63_AVIINFO_DB11, AVI_DB[10]);
    reg_write( _B1_64_AVIINFO_DB12, AVI_DB[11]);
    reg_write( _B1_65_AVIINFO_DB13, AVI_DB[12]);

    /* check sum */
    for( i = 0, ucData = 0; i < 13 ; i++ )
    {
        ucData -= AVI_DB[i];
    }
    ucData -= 0x80+AVI_INFOFRAME_VER+AVI_INFOFRAME_TYPE+AVI_INFOFRAME_LEN;
    reg_write( _B1_5D_AVIINFO_SUM, ucData);

    reg_write( _0F_BankSel, 0x00);
    reg_write( _CD_AVIInfoPkt, 0x3);
    printf("cat6612: AVI Info %d, csum %x\n", VICID, ucData);
}


static void SetInputMode(unsigned char VICID, unsigned char inputColorMode, unsigned char inputSignalType)
{
    unsigned char value = 0x00;
    int i;
    reg_write( _0F_BankSel, 0x00);

    if (inputSignalType == INSIG_SYNCEMB)
    {
	printf("signal without H/Vsync, searching\n");

        for( i = 0 ; SyncEmbTable[i].fmt != 0xFF ; i++ )
        {
            if( VICID == SyncEmbTable[i].fmt )
            {
                printf("[CAT6612]: Pick VICID %d\n", SyncEmbTable[i].fmt);
                break ;
            }
        }

        if( SyncEmbTable[i].fmt == 0xFF )
        {
            return;
        }
        /* 16 Bit Embedded */
        reg_write( _90_HVPol,   SyncEmbTable[i].RegHVPol);
        reg_write( _91_HFPixel, SyncEmbTable[i].RegHfPixel);
        reg_write( _95_HSSL,    SyncEmbTable[i].RegHSSL);
        reg_write( _96_HSEL,    SyncEmbTable[i].RegHSEL);
        reg_write( _97_HSH,     SyncEmbTable[i].RegHSH);
        reg_write( _A0_VSS1,    SyncEmbTable[i].RegVSS1);
        reg_write( _A1_VSE1,    SyncEmbTable[i].RegVSE1);
        reg_write( _A2_VSS2,    SyncEmbTable[i].RegVSS2);
        reg_write( _A3_VSE2,    SyncEmbTable[i].RegVSE2);
    } else {
	printf("de and sync included, skipping 90-a3");
        reg_write(0x72,0x00);
        reg_write(0x90,0x00);
        reg_write(0x61,0x03);
        reg_write(0x71,0x01);
        reg_write(0x62,0x18);
        reg_write(0x63,0x10);
        reg_write(0x64,0x0c);
        reg_write(0x71,0x00);

    }

	printf("setting color mode\n");

    switch (inputColorMode)
    {
        case FORMAT_YUV422:
            value |= (1<<6);
            break;
        case FORMAT_YUV444:
            value |= (2<<6);
            break;
        case FORMAT_RGB444:
        default:
            value |= 0x00;
            break;
    }

    if (inputSignalType == INSIG_SYNCEMB)
    {
		printf("adding magic\n");

        value |= (1<<3);
    }
    printf("[CAT6612] 0x70 write value %x - should be 0x00?\n", value);
    //07-06: Input color mode, 0/1/2=RGB/YUV422/YUV444 (default:0)
    //04-04: 0/1=non-CCIR656/CCIR656 (default:0)
    //03-03: 0/1=sync separate/embedded mode (default:0)
    //02-02: 0/1=input SDR/DDR (default:0)
    reg_write( _70_InputMode, value);
}

static void SetCSC(unsigned char inputColorMode, unsigned char outputColorMode)
{
    /* TODO: except force to RGB444, extend other transfer by needed */
    unsigned char cscsel = 0x00;
    reg_write( _0F_BankSel, 0x00);

    switch (inputColorMode)
    {
        case FORMAT_YUV422:
            switch (outputColorMode)
            {
                case FORMAT_YUV422:
                    cscsel = 0x0; // bypass
                    break;
                case FORMAT_YUV444:
                case FORMAT_RGB444:
                    printf("[CAT6612]: yuv to rgb trans\n");
                    cscsel = 0x03; //Reg72, YUVtoRGB
                    break;
            }
            break;
        case FORMAT_RGB444:
            switch (outputColorMode)
            {
                case FORMAT_YUV422:
                    printf("[CAT6612]: rgb to yuv trans\n");
                    cscsel = 0x2; //Reg72, RGBtoYUV
                    break;
                case FORMAT_YUV444:
                case FORMAT_RGB444:
                    printf("[CAT6612]: rgb to rgb bypass\n");
                    cscsel = 0x00; //Reg72, bypass
                    break;
            }
            break;
    }

    if (cscsel == 0x03)
    {
        reg_write( 0x73 ,0x10);
        reg_write( 0x74 ,0x80);
        reg_write( 0x75 ,0x10);
        reg_write( 0x76 ,0x4F);
        reg_write( 0x77 ,0x09);
        reg_write( 0x78 ,0x81);
        reg_write( 0x79 ,0x39);
        reg_write( 0x7A ,0xDF);
        reg_write( 0x7B ,0x3C);
        reg_write( 0x7C ,0x4F);
        reg_write( 0x7D ,0x09);
        reg_write( 0x7E ,0xC2);
        reg_write( 0x7F ,0x0C);
        reg_write( 0x80 ,0x00);
        reg_write( 0x81 ,0x00);
        reg_write( 0x82 ,0x4F);
        reg_write( 0x83 ,0x09);
        reg_write( 0x84 ,0x00);
        reg_write( 0x85 ,0x00);
        reg_write( 0x86 ,0x1E);
        reg_write( 0x87 ,0x10);
    }
    else if (cscsel == 0x02)
    {
        reg_write( 0x73 ,0x10);
        reg_write( 0x74 ,0x80);
        reg_write( 0x75 ,0x10);
        reg_write( 0x76 ,0x09);
        reg_write( 0x77 ,0x04);
        reg_write( 0x78 ,0x0E);
        reg_write( 0x79 ,0x02);
        reg_write( 0x7A ,0xC8);
        reg_write( 0x7B ,0x00);
        reg_write( 0x7C ,0x0E);
        reg_write( 0x7D ,0x3D);
        reg_write( 0x7E ,0x84);
        reg_write( 0x7F ,0x03);
        reg_write( 0x80 ,0x6E);
        reg_write( 0x81 ,0x3F);
        reg_write( 0x82 ,0xAC);
        reg_write( 0x83 ,0x3D);
        reg_write( 0x84 ,0xD0);
        reg_write( 0x85 ,0x3E);
        reg_write( 0x86 ,0x84);
        reg_write( 0x87 ,0x03);
    }

    //07-07: 0/1=Enable dither function (default:0)
    //06-06: 0/1=enable Cr/Cb up/down sampling function (default:0)
    //05-05: 0/1=Dither noise pattern (default:0)
    //01-00: 0/2/3=none/rgb2yuv/yuv2rgb (default:0)
	printf("setting 72 to %02x should be 00\n",cscsel);
    reg_write( _72_CSCCtrl, cscsel);
}

static void setupAFE(int clock_freq)
{
    unsigned char uc;
    reg_write( _0F_BankSel, 0x00);

    //trun off, to be enalbe at last after video stable input.
    reg_write( _61_AFEDrvCtrl, 0x10);

    if (clock_freq > 80)
    {
        reg_write( _62_AFEXPCtrl      , 0x89);
        reg_write( _63_AFEXPCtrl2     , 0x01);
        reg_write( _64_AFEIPCtrl      , 0x56);
        reg_write( _65_AFERing        , 0x00);
    }
    else
    {
        reg_write( _62_AFEXPCtrl      , 0x19);
        reg_write( _63_AFEXPCtrl2     , 0x01);
        reg_write( _64_AFEIPCtrl      , 0x1E); /*0x17 original?*/
        reg_write( _65_AFERing        , 0x00);
    }

    uc = reg_read( _04_RstCtrl);
    uc &= 0xD7;
    reg_write( _04_RstCtrl, uc);
}

static void fireAFE(void)
{
    reg_write( _0F_BankSel    ,0x00);
    reg_write( _61_AFEDrvCtrl ,0x03);
}

int main(){
    HDMI_Video_ID VICID=19; // 1280 x 720 but doesn't matter
    int clock_freq=65; // 1280*720*50 = 46.08 Mhz < 80 .. measured .. it is 65mHz
    unsigned char inputColorMode = FORMAT_RGB444;
//    unsigned char inputSignalType=INSIG_SYNCSEP; // hsync and vsync ok
    unsigned char inputSignalType=INSIG_SYNCEMB; // hsync and vsync ok
    unsigned char outputColorMode = FORMAT_RGB444; // we fix output color to YUV422 cause HDMI Rx CAT6011 was changed.
    int ret, i;
    printf("[CAT6612] VICID %d, input color %d, input signal %d, output color %d\n", VICID, inputColorMode, inputSignalType, outputColorMode);

    /* Detect device */
    ret = reg_compare(_01_VendorID, 0xCA, 0xFF);
    if (ret<0)
        return ret;
    ret = reg_compare(_02_DevID0, 0x11, 0xFF);
    if (ret<0)
        return ret;
    ret = reg_compare(_03_DevID1, 0x16, 0xFF);
    if (ret<0)
        return ret;

	printf("Device ok\n");
	printf("resetting\n");

    // Software reset
    reg_write( _0F_BankSel            ,0x0);
    reg_write( _04_RstCtrl            ,0x3D); // 0011 1101
    sleep(1);
    reg_write( _04_RstCtrl            ,0x1D); // 0001 1101

	printf("enabling clock ring\n");
    //Enable clock ring
    reg_write( _61_AFEDrvCtrl         ,0x10);

	printf("turning all packets off\n");
    //Turn off all packet
    reg_write( _C9_NullPkt            ,0x00);
    reg_write( _CA_ACPPkt             ,0x00);
    reg_write( _CB_ISRC1Pkt           ,0x00);
    reg_write( _CC_ISRC2Pkt           ,0x00);
    reg_write( _CD_AVIInfoPkt         ,0x00);
    reg_write( _CE_AudioInfoPkt       ,0x00);
    reg_write( _CF_SPDInfoPkt         ,0x00);
    reg_write( _D0_MpgInfoPkt         ,0x00);


	// lets enabled a few interrupts
	printf("activating interrupt\n");
    uint8_t uc;
// 09[0] = 0 <-- activates event for hot plug
    uc = reg_read( 0x09);
    uc &= ~(1<<0);
    reg_write( 0x09, uc);
	sleep(1);

// 09[1] = 0 <-- activates event for rx sensing
    uc = reg_read( 0x09);
    uc &= ~(1<<1);
    reg_write( 0x09, uc);
	sleep(1);

//0b[3] = 1 <-- activates event for video stableasd
	uc = reg_read( 0x0b);
	uc &= ~(1<<3);
	reg_write( 0x0b, uc);
	sleep(1);
	uint8_t stage = 0;
	printf("checking for interrupt\n");
	for(i = 0 ; i < 10 ; i++)
	{
        	if(reg_compare(_0E_SysStat, 0x80, 0x80) == 0) // mask hinten
	        {
	        	printf("[CAT6112] Interrupt!\n");
			printf("getting interrupt reason\n");
	        	if( stage==0 && reg_compare(0x06, 0x01, 0x01) == 0) // mask hinten
			{
				printf("hotplug interrupt\n");
		        	if( reg_compare(0x0e, 0x40, 0x40) == 0) // mask hinten
				{
					printf("PLUGGED IN\n");
					printf("clearing interrupt\n");
					// interrupt clear mode
					uc = reg_read( 0x0e);
					uc |= (1<<0);
					reg_write( 0x0e, uc);

					// clear flag 0x00 from 0x0c
					uc = reg_read( 0x0c);
					uc |= (1<<0);
					reg_write( 0x0c, uc);

					// leave interrupt clear mode
					uc = reg_read( 0x0e);
					uc &= ~(1<<0);
					reg_write( 0x0e, uc);
					
					stage++;
				}
				else
				{
					printf("unplugged");
				}
			}
			else if(stage==1 && reg_compare(0x06, 0x02, 0x02) == 0) // mask hinten
			{
				printf("rx sensing interrupt\n");
			        if( reg_compare(0x0e, 0x20, 0x20) == 0) // mask hinten
				{
					printf("rx on\n");
					// interrupt clear mode
					uc = reg_read( 0x0e);
					uc |= (1<<0);
					reg_write( 0x0e, uc);

					// clear flag 0x01 from 0x0c
					uc = reg_read( 0x0c);
					uc |= (1<<1);
					reg_write( 0x0c, uc);

					// leave interrupt clear mode
					uc = reg_read( 0x0e);
					uc &= ~(1<<0);
					reg_write( 0x0e, uc);
		
					stage++;

				}
				else
				{
					printf("rx off");
				}
			}
			else if( reg_compare(0x08, 0x10, 0x10) == 0) // mask hinten
			{
				printf("video stable interrupt\n");
			        if( reg_compare(0x0e, 0x10, 0x10) == 0) // mask hinten
				{
					printf("stable video\n");
					break;
				}
				else
				{
					printf("video not longer stable");
				}
			}
		}
		else
		{
		        printf("[CAT6112] no interrupt!\n");
		        sleep(1);
		}
	}






    //initialize done
	printf("checking bstatus:\n");
reg_write(0x10,0x01);
reg_write(0x11,0x74);
reg_write(0x12,0x41);
reg_write(0x13,0x02);
reg_write(0x15,0x00);
    ret = reg_compare(0x16, 0x82, 0x80);
    if (ret<0){
	printf("error fetching bstatus of fifo empty\n");
	sleep(1);
    } else {
	ret = reg_compare(0x45, 0x10, 0x10);
	if (ret<0){
		printf("bstatus: hdmi\n");
	} else {
		printf("bstatus: dvi\n");
		sleep(3);
	}
    }

    // Check HDMI sink capability, should read from EDID
    // not support now, manual set
	

    // Switch to PC program DDC mode
	printf("setting DDC por to PC tunnel\n");
    reg_write( _10_MasterSel          ,0x01);

    /* Setinput Mode */
    SetInputMode(VICID, inputColorMode, inputSignalType);

    /* Set Color Space Conversion */
    SetCSC(inputColorMode, outputColorMode);

    /* Force to DVI mode */
   // reg_write( _C0_HDMIMode, 0x00);
   // reg_write( _0F_BankSel,  0x01);
   // reg_write( _B1_58_AVIINFO_DB1, 0x00);

    /* Set to HDMI mode */
	printf("switching to hdmi mode\n");
    reg_write( _C0_HDMIMode, 0x01);

    setupAFE(clock_freq);

    //Wait for video stable input
	printf("checking video signal\n");
    for(i = 0 ; i < 10 ; i++)
    {
        if( reg_compare(_0E_SysStat, 0x10, 0x10) == 0)
        {
            printf("[CAT6112] video stable!\n");
            break ;
        }
        printf("[CAT6112] video un-stable!\n");
        sleep(1);
    }

	printf("enabled driver\n");

    // HDMITX VCLK Reset Disable
    reg_write(_04_RstCtrl     ,0x15); // 00[0]1 [0]101

    // Enable HDMITX AFE after video stable
    fireAFE();
    config_AVI_InfoFrame(VICID, outputColorMode);

    // Disable AVMute to start output
	printf("disabling mute send info frame\n");
    reg_write( _C1_Mute               ,0x80);
    reg_write( _C6_GenCtrlPkt, 1|(1<<1));
    return 0;
}


