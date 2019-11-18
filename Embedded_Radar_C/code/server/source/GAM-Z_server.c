#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h> //file read write
#include <linux/input.h> // touch screen
#include <linux/fb.h>
//math.h ∏¶ include«ﬂ±‚ ∂ßπÆø° ƒƒ∆ƒ¿œΩ√ -ln ¿ª ≤¿ ∏ÌΩ√«ÿ¡‡æﬂ«‘.
//ex) gcc -o server GAM-Z_server.c -ln
#include <math.h>

#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define MAX_BUTTON  9
#define MAX_BUFF 32
#define LINE_BUFF 16
#define  BUFF_SIZE   512
#define LED_DEVICE "/dev/fpga_led"

static int *send_flag;
static int *receive_flag;
static int *logout;
static int *up_data;
static int *update_flag;
static int *semaphore;
void client_receive_process(int*);
void update_process(short *, struct fb_var_screeninfo);
void make_radar(short *, struct fb_var_screeninfo,int *);

unsigned char quit = 0;
 
void user_signal1(int sig)
{
    quit = 1;
}


int main( void)
{
   	int   server_socket;
   	int   client_socket;
   	int   client_addr_size;
	pid_t pid;
   	struct sockaddr_in   server_addr;
   	struct sockaddr_in   client_addr;
	struct fb_var_screeninfo fvs; 
	short *pfbdata;
	int val = 1;
   	char buff_rcv[BUFF_SIZE+5];
   	char buff_snd[BUFF_SIZE+5];
	int frame_fd, check, str_size;

	if((frame_fd = open("/dev/fb0",O_RDWR))<0) {
        	perror("Frame Buffer Open Error!");
        	exit(1);
    	}
	if((check=ioctl(frame_fd,FBIOGET_VSCREENINFO,&fvs))<0) {
		perror("Get Information Error - VSCREENINFO!");
		exit(1);
	}
	pfbdata = (short *) mmap(0, fvs.xres*fvs.yres*(sizeof(short)), PROT_READ|\
		 PROT_WRITE, MAP_SHARED, frame_fd, 0);
	
   	server_socket  = socket( PF_INET, SOCK_STREAM, 0);
   	if( -1 == server_socket)
   	{
      	printf( "server socket fail\n");
    	exit( 1);
   	}

   	memset( &server_addr, 0, sizeof( server_addr));
   	server_addr.sin_family     = AF_INET;
	server_addr.sin_port       = htons( 4000);
	server_addr.sin_addr.s_addr= htonl( INADDR_ANY);
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,(char*)&val, sizeof(val)) <0)
	{
		perror("setsockopt");
		close(server_socket);
		return -1;
	}

   	if( -1 == bind( server_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) )
	{
	      printf( "bind() error\n");
	      exit( 1);
	}

  	if( -1 == listen(server_socket, 5))
   	{
   		printf( "listen() error\n");
    	exit( 1);
	}

   	client_addr_size  = sizeof(client_addr);
	client_socket     = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
   	if ( -1 == client_socket)
   	{
   		printf( "fail to open server\n");
       		exit(1);
   	}
	(void)signal(SIGINT, user_signal1);

	send_flag = mmap(NULL, sizeof *send_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    receive_flag = mmap(NULL, sizeof *receive_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	logout = mmap(NULL, sizeof *logout, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	update_flag = mmap(NULL, sizeof *update_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	up_data = mmap(NULL,(sizeof *up_data)*70, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	semaphore = mmap(NULL, sizeof *semaphore, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*send_flag = 0;
	*receive_flag = 0;
	*logout = 0;
	//? ??? ?? ? ??? ? ??? ? ??.
	//?? ???? while? ???...
	//??? ?? ? ??? ?? ??
	//??? ?? ? ? ??? ????! ?? ??? ??..
	pid = fork();
	if(pid != 0)
	{
		client_receive_process(&client_socket);
		kill(pid,SIGINT);
		close(client_socket);
		close(frame_fd);
		munmap(up_data,(sizeof *up_data)*70);
		munmap(logout,sizeof *logout);
		munmap(update_flag,sizeof *update_flag);
		munmap(receive_flag,sizeof *receive_flag);
		munmap(send_flag,sizeof *send_flag);
		munmap(semaphore  ,sizeof *semaphore );
		munmap(pfbdata,fvs.xres*fvs.yres*(sizeof(short)));
		return 0;
	}
	else
	{
		update_process(pfbdata, fvs);
		return 0;
	}
}

void update_process(short *pfbdata, struct fb_var_screeninfo fvs)
{
	unsigned char push_sw_buff[MAX_BUTTON];
	unsigned char string[32];
	char str_buff[10][512] = {0};
	int data[10][70];
	char cha_buff;
	char temp[4] = {0};
	char str[16]={0}, led_num;
	int dev, dev_text, i, n, j,z,data_index, str_size, buff_size, num;
	int dev_led;
	FILE *fp;
	printf("I'm client send process\n");
	//switch
	dev = open("/dev/fpga_push_switch", O_RDWR);
	if (dev<0){
		printf("Device Open Error\n");
	}
	dev_led = open(LED_DEVICE, O_RDWR);
	if (dev_led<0) {
		printf("Device open error : %s\n",LED_DEVICE);
		exit(1);
	}

	dev_text = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
    if (dev_text < 0) {
       	printf("Device open error : %s\n",FPGA_TEXT_LCD_DEVICE);
    }
	buff_size=9;

	led_num = 0;			
	write(dev_led,&led_num,1);
	memset(string,0,sizeof(string));
	strcpy(str,"rada_server");
	str_size=strlen(str);
	strncat(string,str,str_size);
    memset(string+str_size,' ',LINE_BUFF-str_size);
	strcpy(str,"1. hist/9. exit");
	str_size=strlen(str);
	strncat(string,str,str_size);
    memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
	write(dev_text,string,MAX_BUFF);
	while(!quit){
		usleep(10000);		
		read(dev, &push_sw_buff,9);
		if(*update_flag){
			printf("update complete!!\n");
			while(*semaphore){}
			make_radar(pfbdata, fvs, up_data);	
			sprintf(str,"%4d-%02d-%02d %02d:%02d", *(up_data + 61) + 1900, *(up_data + 62), *(up_data + 63), *(up_data + 64), *(up_data + 65));
			memset(string,0,sizeof(string));
			str_size=strlen(str);
			strncat(string,str,str_size);
			memset(string+str_size,' ',LINE_BUFF-str_size);
			strcpy(str,"1. hist/9. exit");
			str_size=strlen(str);
			strncat(string,str,str_size);
			memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
			write(dev_text,string,MAX_BUFF);
			*update_flag = 0;
		}
		if(push_sw_buff[0] == 1){
			*semaphore = 1;
			fp = fopen("input.txt", "r");  // ?åÏùº ?¥Í∏∞
			i = 0;
			n = 0;
			while(1)  // ?åÏùº???ùÏù¥ ?ÑÎãà?ºÎ©¥
	        {
	            fgets(str_buff[i], 512, fp);  // ÏµúÎ? 80Ïπ∏ÏßúÎ¶??úÏ§Ñ ?ΩÍ∏∞
	            if(feof(fp))
	                break;
	            n++;
	            i++;
	        }
	        *semaphore = 0;
	        fclose(fp);
			z = 0;
			data_index = 0;
			memset(data,0,700);
			for(i = 0; i < n; i++){
				for(j = 0; j < strlen(str_buff[i]) + 1; j++){
					cha_buff = str_buff[i][j];
					if(cha_buff != ' ')
						temp[z++] = cha_buff;
					else if(cha_buff == ' ' || cha_buff == '\0'){
						temp[z] = '\0';
						z = 0;
						data[i][data_index++] = atoi(temp);
						memset(temp,0,4);
					}
				}
				data_index = 0;
			}
			num = n-1;
			make_radar(pfbdata, fvs,data[num]);
			led_num = (char) pow(2.0,(double) num);			
			write(dev_led,&led_num,1);	
			sprintf(str,"%4d-%02d-%02d %02d:%02d", data[num][61] + 1900, data[num][62], data[num][63], data[num][64], data[num][65]);
			memset(string,0,sizeof(string));
			str_size=strlen(str);
			strncat(string,str,str_size);
			memset(string+str_size,' ',LINE_BUFF-str_size);
			strcpy(str,"1:<-/2:->/3.exit");
			str_size=strlen(str);
			strncat(string,str,str_size);
			memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
			write(dev_text,string,MAX_BUFF);
			//history start
			while(1){
				usleep(10000);
		        read(dev, &push_sw_buff,9);
				if(push_sw_buff[2] == 1){
					memset(string,0,sizeof(string));
					strcpy(str,"rada_server");
					str_size=strlen(str);
					strncat(string,str,str_size);
				    memset(string+str_size,' ',LINE_BUFF-str_size);
					strcpy(str,"1. hist/9. exit");
					str_size=strlen(str);
					strncat(string,str,str_size);
				    memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
					write(dev_text,string,MAX_BUFF);
					led_num = 0;			
					write(dev_led,&led_num,1);
					break;
				}
				else if(push_sw_buff[0] == 1){ 
					if(num > 0)
						num--;
					led_num = (char)pow(2.0,(char) num);			
					write(dev_led,&led_num,1);
					make_radar(pfbdata, fvs,data[num]);		
					sprintf(str,"%4d-%02d-%02d %02d:%02d", data[num][61] + 1900, data[num][62], data[num][63], data[num][64], data[num][65]);
					memset(string,0,sizeof(string));
					str_size=strlen(str);
					strncat(string,str,str_size);
					memset(string+str_size,' ',LINE_BUFF-str_size);
					strcpy(str,"1:<-/2:->/3.exit");
					str_size=strlen(str);
					strncat(string,str,str_size);
					memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
					write(dev_text,string,MAX_BUFF);
				}
				else if(push_sw_buff[1] == 1){
					if(num < n-1)
						num++;
					led_num = (char)pow(2.0,(double) num);			
					write(dev_led,&led_num,1);
					make_radar(pfbdata, fvs,data[num]);	
					sprintf(str,"%4d-%02d-%02d %02d:%02d", data[num][61] + 1900, data[num][62], data[num][63], data[num][64], data[num][65]);
					memset(string,0,sizeof(string));
					str_size=strlen(str);
					strncat(string,str,str_size);
					memset(string+str_size,' ',LINE_BUFF-str_size);
					strcpy(str,"1:<-/2:->/3.exit");
					str_size=strlen(str);
					strncat(string,str,str_size);
					memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
					write(dev_text,string,MAX_BUFF);
				}

			}			
		}
	}	
	close(dev);
	close(dev_text);
	close(dev_led);
}

void client_receive_process(int *sock)
{
    char buff[512];
    int i, j, n;
	FILE *fp;
	char str_buff[10][512] = {0}; //¬ø¬ø¬ø ¬ø¬ø¬ø¬ø ¬ø¬ø¬ø 80¬ø¬ø¬ø ¬ø¬ø!
	printf("I'm client receive process\n");
	
	while(1)
    {                               
        memset(buff,0,sizeof(buff)); 
        read(*sock, buff, BUFF_SIZE);
        printf("%s\n",buff);
        if(!strcmp(buff,"logout"))
        {
            printf("%s\n",buff);
            *logout = 1;
            break;
        }
        else if(!strcmp(buff,"data_in"))
        {        
            memset(buff,0,sizeof(buff)); 
            read(*sock, buff, BUFF_SIZE);
			printf("now: %d-%d-%d %d:%d:%d\n", buff[61] + 1900, buff[62], buff[63],buff[64], buff[65], buff[66]);
			
        	memset(str_buff,0,sizeof(str_buff)); 
			*semaphore = 1;	
            fp = fopen("input.txt", "r");  // ?åÏùº ?¥Í∏∞
            putchar('\n');
            i = 0;
			n = 0;
            while(1)  // ?åÏùº???ùÏù¥ ?ÑÎãà?ºÎ©¥
            {
            fgets(str_buff[i], 512, fp);  // ÏµúÎ? 80Ïπ∏ÏßúÎ¶??úÏ§Ñ ?ΩÍ∏∞
				if(feof(fp))    
                    break;    
				n++;
				i++;
            }    
			fclose(fp);
            printf("now, we have  %d data set. add new data set.\n\n",n);
            if(n < 8){
            	fp = fopen("input.txt", "a+");  // ?åÏùº ?¥Í∏∞
				for(i = 0; i < 70; i++) {
					*(up_data + i) = buff[i];
					fprintf(fp,"%d ",buff[i]);
					//printf("%d\n",*(up_data + i));
				}
				fprintf(fp,"\n");
			} else {
            	fp = fopen("input.txt", "w");  // ?åÏùº ?¥Í∏∞
   		        for(i = 0; i < n - 1; i++){
					strcpy(str_buff[i],str_buff[i+1]);
					//printf("%s\n",str_buff[i]);
				}
				for(i = 0; i < n - 1 ; i++){
                 	fprintf(fp,"%s",str_buff[i]);
				}
				for(i = 0; i < 70; i++) {
					*(up_data + i) = buff[i];
	                fprintf(fp,"%d ",buff[i]);
	            }
	            fprintf(fp,"\n");
			}
			*update_flag = 1;
			fclose(fp);
			*semaphore = 0;	
        }
     } 
}

void make_radar(short *pfbdata, struct fb_var_screeninfo fvs, int *ptr)
{
	int tan_list[16] = {0, 524, 1051, 1583, 2125, 2679, 3249, 3838, 4452, 5095, 5773, 6494, 7265,     8097, 9004, 10000};
	int r[61] = {0}, i, repx, repy, x, y, offset, temp;
	int temp_warn, warn_val,str_size;
	short color;

	if((unsigned)pfbdata == (unsigned)-1) {
        	perror("Error Mapping!\n");
    }
	for(repy = 599; repy >= 0; repy--)
	{
		offset = repy * fvs.xres;
		for(repx = 0; repx < 1024; repx++)
		{
			x = repx - 512;
			y = 599 - repy;
			if((x)*(x) + (y)*(y) <= 250000 + 10000 && (x*x)+(y*y) > 250000){
				*(pfbdata+offset+repx) = 0xffff;
			}
		}
	}
	for(i = 0; i < 61; i++)
		r[i] = *(ptr + i);
	temp_warn = (*(ptr + 67)) * 2;
	warn_val = *(ptr + 67);
	printf("warn_val = %d\n",warn_val);
	for(repy = 598; repy >= 0; repy--)
	{
		offset = repy * fvs.xres;
		for(repx = 0; repx < 1024; repx++)
		{
			x = repx - 512;
			y = 599 - repy;
			if((x)*(x) + (y)*(y) <= 250000 + 10000 && (x*x)+(y*y) > 250000){
				*(pfbdata+offset+repx) = 0xffff;
			}
			else if((x)*(x) + (y)*(y) <= 250000){
				if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn+ 10)  && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
					*(pfbdata+offset+repx) = 0b1111111111100000;
				}
			}
			else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
			{
				if((x)*(x) + (y)*(y) <= 500*500){
					*(pfbdata+offset+repx) = 0xffff;
				}
			}
			else if(2 * y >= x - 2 &&  2 * y <= x + 2)
			{
				if((x)*(x) + (y)*(y) <= 500*500){
					*(pfbdata+offset+repx) = 0xffff;
				}
			}
			else if(y >=(-2 * x) - 1 &&  y <= (-2*x) + 1)
			{
				if((x)*(x) + (y)*(y) <= 500*500){
					*(pfbdata+offset+repx) = 0xffff;
				}
			}
			else if(2 * y >= (-1 * x) - 2 &&  2 * y <=(-1 *  x) + 2)
			{
				if((x)*(x) + (y)*(y) <= 500*500){
					*(pfbdata+offset+repx) = 0xffff;
				}
			}
			else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
				*(pfbdata+offset+repx) = 0xffff;
			}
			else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
				*(pfbdata+offset+repx) = 0xffff;
			}
			else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
				*(pfbdata+offset+repx) = 0xffff;
			}
			else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
				*(pfbdata+offset+repx) = 0xffff;
			}
			else {
				*(pfbdata+offset+repx) = 0x0000;
			}
		}
	}	
		//right circle
		for(i = 0; i <= 15; i++)
		{
			if(r[i] -1 < warn_val){
				color = 0b1111100000000000;
			} else {
				color = 0b0000001111100000;
			}
			r[i] = (r[i]- 1) * 2;
			for(repy = 599; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 513; repx < 1013; repx++)
				{
					x = repx - 512;
					y = 599 - repy;
					temp = (int) (y*10000)/x;
					if((x)*(x) + (y)*(y) <= 250000 + 10000 && (x*x)+(y*y) > 250000){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						*(pfbdata+offset+repx) = 0b1111111111100000;
					}
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if(i == 0){
						if(temp <= tan_list[i]){
							if((x)*(x) + (y)*(y) <= r[i]*r[i])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							if((x)*(x) + (y)*(y) <= r[i]*r[i])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
		}
	
		for(i = 15; i > 0; i--)
		{
			if(r[15 + (16 - i)] - 1< warn_val){
				color = 0b1111100000000000;
			} else {
				color = 0b0000001111100000;
			}
			r[15 + (16 - i)] = (r[15 + (16 - i)]-1) * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 513; repx < 1013; repx++)
				{
					x = repx - 512;
					y = 599 - repy;
					temp = (int) (x*10000)/y;
					if((x)*(x) + (y)*(y) <= 250000 + 10000 && (x*x)+(y*y) > 250000){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} 
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else {
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							if((x)*(x) + (y)*(y) <= r[15 + (16 - i)]*r[15 + (16 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
	
				}
			}
		}
		for(repy = 599 ; repy >= 100; repy--)
		{
			offset = repy * fvs.xres;
			*(pfbdata+offset+512) = 0xffff;
			
		}
		
		//leftcircle
		for(i = 1; i <= 15; i++)
		{
			if((r[30 +  i]-1) < warn_val){
				color = 0b1111100000000000;
			} else {
				color = 0b0000001111100000;
			}
			r[30 +  i] = (r[30 +  i] - 1) * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 0; repx < 512; repx++)
				{
					x = repx - 512;
					x = -1 * x;
					y = 599 - repy;
					temp = (int) (x*10000)/y;
					if((x)*(x) + (y)*(y) <= 250000 + 10000 && (x*x)+(y*y) > 250000){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} 				
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							if((x)*(x) + (y)*(y) <= r[30 + i]*r[30 + i])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
		}
		for(i = 15; i > 0; i--){
			if((r[46 +  (15 - i)] - 1) < warn_val){
				color = 0b1111100000000000;
			} else {
				color = 0b0000001111100000;
			}
			r[46 +  (15 - i)] = (r[46 +  (15 - i)]- 1) * 2;
			for(repy = 599; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 0; repx < 512; repx++)
				{
					x = repx - 512;
					x = -1*x;
					y = 599 - repy;
					temp = (int) (y*10000)/x;				
					if((x)*(x) + (y)*(y) <= 250000 + 10000 && (x*x)+(y*y) > 250000){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if( (x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} 
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						*(pfbdata+offset+repx) = 0xffff;
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							if((x)*(x) + (y)*(y) <= r[46 + (15 - i)]*r[46 + (15 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
	}
}



