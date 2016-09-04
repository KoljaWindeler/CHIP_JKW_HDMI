#include <glib.h>
#include <glib/gprintf.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void main(void) {
    int file;
    char filename[40];
    const gchar *buffer;
    int addr = 0x4c;        // The I2C address of the ADC

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

    char buf[10] = {0};
    float data;
    char channel;
    //unsigned char reg = 0x10; // Device register to access
    //buf[0] = reg;
    buf[0] = 0x01;

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
            printf("Channel %02x \n",buf[0]);
        }


}
