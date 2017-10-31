#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define DEVICE_FILE_NAME "/dev/BufferedMem"

int main(int argc, char **argv)
{
	int device;
	char *input  = atoi(argv[1]);

	device = open(DEVICE_FILE_NAME, O_RDWR|O_NDELAY);
	if (device >= 0)
	{
		unlocked_ioctl(device, 1, input);
	}
	else
		perror("Device file open fail");
	close(device);

	return 0;
}