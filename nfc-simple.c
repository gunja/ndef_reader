//
//  main.c
//  init
//

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>

#define CHECK() if (r<0) {perror("write"); return -2;}

int my_read(int fd)
{
    fd_set mask;
    struct timeval timeout = { .tv_sec = 0, .tv_usec = 10 * 1000 };
    int r, n;
    char buffer[1024];

    FD_ZERO(&mask);
    FD_SET(fd, &mask);

    n = 0;
    do
    {
        r = select(fd+1, &mask, NULL, NULL, &timeout);
        if (r > 0)
        {
            r =  read(fd, buffer, sizeof buffer);
            printf(".(%d)",r);
	    if (r > 0)
	    printf("%02x", (unsigned int)buffer[0]);
            n++;
        }
    } while (r>0);
    printf("\n");
        
    if (r < 0)
        perror("select");

    return n;
}

int main(int argc, const char * argv[]) {
    ssize_t r;
    int fd;

    fd = open("/dev/ttyS0", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    /* wake up */
    printf("wake up\n");
    r = write(fd, "\x55\x55\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    CHECK()

    /* 0x14 : SAMConfiguration */
    printf("SAMConfiguration\n");
    r = write(fd, "\x00\x00\xff\x03\xfd\xd4\x14\x01\x17\x00", 10);
    CHECK()
    my_read(fd);

    /* 0x00 : Diagnose */
    printf("Diagnose\n");
    r = write(fd, "\x00\x00\xff\x09\xf7\xd4\x00\x00\x6c\x69\x62\x6e\x66\x63\xbe\x00", 16);
    CHECK()
    my_read(fd);

    /* 0x02 : GetFirmwareVersion */
    printf("GetFirmwareVersion\n");
    r = write(fd, "\x00\x00\xff\x02\xfe\xd4\x02\x2a\x00", 9);
    CHECK()
    my_read(fd);

    /* 0x12 : SetParameters */
    printf("SetParameters\n");
    r = write(fd, "\x00\x00\xff\x03\xfd\xd4\x12\x14\x06\x00", 10);
    CHECK()
    my_read(fd);

    /* 0x06 : ReadRegister */
    printf("ReadRegister\n");
    r = write(fd, "\x00\x00\xff\x0c\xf4\xd4\x06\x63\x02\x63\x03\x63\x0d\x63\x38\x63\x3d\xb0\x00", 19);
    CHECK()
    my_read(fd);

    /* 0x08 : WriteRegister */
    printf("WriteRegister\n");
    r = write(fd, "\x00\x00\xff\x08\xf8\xd4\x08\x63\x02\x80\x63\x03\x80\x59\x00", 15);
    CHECK()
    my_read(fd);

    /* 0x32 : RFConfiguration */
    printf("RFConfiguration\n");
    r = write(fd, "\x00\x00\xff\x04\xfc\xd4\x32\x01\x00\xf9\x00", 11);
    CHECK()
    my_read(fd);

    /* 0x32 : RFConfiguration */
    printf("RFConfiguration\n");
    r  = write(fd, "\x00\x00\xff\x04\xfc\xd4\x32\x01\x01\xf8\x00", 11);
    CHECK()
    my_read(fd);

    /* 0x32 : RFConfiguration */
    printf("RFConfiguration\n");
    r = write(fd, "\x00\x00\xff\x06\xfa\xd4\x32\x05\xff\xff\xff\xf8\x00", 13);
    CHECK()
    my_read(fd);

    /* 0x06 : ReadRegister */
    printf("ReadRegister\n");
    r = write(fd, "\x00\x00\xff\x0e\xf2\xd4\x06\x63\x02\x63\x03\x63\x05\x63\x38\x63\x3c\x63\x3d\x19\x00", 21);
    CHECK()
    my_read(fd);

    /* 0x08 : WriteRegister */
    printf("WriteRegister\n");
    r = write(fd, "\x00\x00\xff\x08\xf8\xd4\x08\x63\x05\x40\x63\x3c\x10\xcd\x00", 15);
    CHECK()
    my_read(fd);

    /* 0x60 : InAutoPoll */
    printf("InAutoPoll\n");
    r = write(fd, "\x00\x00\xff\x0a\xf6\xd4\x60\x14\x02\x20\x10\x03\x11\x12\x04\x5c\x00", 17);
    CHECK()
    my_read(fd);

    close(fd);
    
    return 0;
}
