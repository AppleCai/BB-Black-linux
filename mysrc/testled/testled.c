#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char * argv)
{
int i, n, fd;

fd = open("/dev/leds", O_RDWR);
if (fd < 0)
{
printf("can't open /dev/leds!\n");
exit(1);
}

while (1) {
ioctl(fd, 1, 1);
sleep(1);

ioctl(fd, 0, 1);
sleep(1);
}

close(fd);

return 0;
}