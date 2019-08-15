/* Achro-i.MX6Q External Sensor Test Application
File : ext_sensor_app.c
Auth : gmlee@huins.com */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
int main(void)
{
	int fd;
	int retn;
	char buf[10] = {0};
	int loop = 0;
	int count = 500;
	fd = open("/dev/ext_sens", O_RDWR);
	printf("fd = %d\n", fd);
	if(fd<0){
		perror("/dev/ext_sens error");
		exit(-1);
	}else{
		printf("< ext_sens device has been detected >\n");
	}
	count = 0;
	while(1)
	{
		buf[0] = 1;
		read(fd,buf,sizeof(buf));
		usleep(count);
		buf[0] = 0;
		read(fd,buf,10);
		usleep(20000 - count);
		count+=100;
		if(count > 2500)
			count = 500;
		sleep(1);
	}
	/*
	while(1){
		read(fd,buf,10);
		if(buf[0] == '0')
		{
			printf("< Detected >\n");
		} else if(buf[0] == '1') {
			printf("< Searching >\n");
		}
		for(loop = 0; loop < 1000000; loop++) {};
	}
	*/
	close(fd);
	return 0;
}
