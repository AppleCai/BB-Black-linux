#include <stdio.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <string.h>  
#include <errno.h>  
#include <sys/types.h>  
  
int main (void)   
{  
    int n, fd;  
    char read_buf[256] ={};  
    char *p =NULL;  
    char write_buff[256]="beijing 15071 bu yao shui jiao";  
  
    puts("1:this is read_buf:");  
    printf("read_buf = %s\n", read_buf);  
  
    fd = open ("/dev/hello", O_RDWR);  
    if (fd < 0) {  
        perror("open");  
        exit(0);  
    }  
    printf ("\n/dev/hello opened, fd=%d\n", fd);  
  
    /* read */  
    printf ("Read returns %d\n", (int)read (fd, read_buf, sizeof(read_buf)));  
  
    puts("\n2:this is read_buf:");  
    printf("read_buf = %s\n", read_buf);  
  
    /* write */  
    printf("write buffer: %s\n", write_buff);  
    if (write (fd, write_buff, strlen(write_buff)) < 0)  
    {  
        perror("fail to write");  
    }  
  
    close (fd);  
    printf ("/dev/hello closed :)\n");  
    return 0;  
  
}  
