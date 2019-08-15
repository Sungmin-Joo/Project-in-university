/*term project*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //fork header
#include <sys/socket.h>
#include <sys/mman.h>//공유메모리 사용을 위해 추가한 헤더
#include <signal.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h> //file read write
#include <linux/input.h> // touch screen
#include <linux/fb.h>
#include <sys/stat.h>
#include "./fpga_dot_arrow.h"
#include <time.h>


#define MAX_BUFF 32
#define LINE_BUFF 16
#define FPGA_DOT_DEVICE "/dev/fpga_dot"
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define FRAME_BUFF "/dev/fb0"
#define BUFF_SIZE	1024
#define MAX_BUTTON	9
#define deep_puple 44026	//버튼이 눌렸을 때의 RGB값
#define puple 60957			//버튼이 안 눌렸을 때의 RGB값

static int *warn_line;		// 10단위로 올리고 내리면서 경고 거리(부저가 울리게 되는 거리) 설정.
static int *buz_flag;		//0~1 단지 부저 flag 용도로 사용.
static int *up_down_pwm;	// n ~ m 까지 pwm저장해서 사용.
static int *side_pwm;		//좌우 회전용 pwm, 설정은 위와 동일.
static int *detect_cm;		//7-seg 출력 거리, 0~250, Dot_matrix 가 0이 아닐떄 출력.
static int *detect_dir;		//Dot_matrix 출력 방향, 1~7, 0은 발견 안됨.
static int *send_flag;		//보낼 data가 있으면 1 아니면 0.
static int *receive_flag;		//수신 data가 있으면 1 아니면 0.
static int *t_x, *t_y, *deep;	//터치스크린 이벤트에 관한 정보를 기록.
static int *semaphore;		//운영체제의 세마포 알고리즘을 구현하기 위한 변수.
static int *up_down_flag;		//FPGA보드의 스위치를 누를때 만 up_down pwm이 변하게 하기위한 변수.
static int *connected_flag;		//server와 정상적으로 통신이 됐는지 확인하는 flag
static int *data_semaphore;		//데이터 통신에서 세마포 알고리즘 구현하기 위한 변수.
static char *data_buf;			//통신에 사용하는 버퍼.
static char *warn_data;			//현재 설정된 *warn_line을 서버에 보내기 위한 변수.


//소켓통신 재료
static int client_socket;
static struct sockaddr_in server_addr;


unsigned char quit = 0;

//SIGINT 핸들러
void user_signal1(int sig)
{
    quit = 1;
}

//메모리 할당 함수.
void mmset(void);

//메모리 해제 함수.
void un_mmap(void);

//서버에 send 를 담당하는 프로세스.
void client_send_process(void);

//서버에서 receive를 담당하는 프로세스. (추후 구현을위해 만들어 둠)
void client_receive_process(void);

//레이더의 동작 담당 프로세스.
void radar_process(short*, struct fb_var_screeninfo);

//레이더의 세팅을 담당하는 프로세스. (터치 이벤트 처리)
void set_process(void);

//FPGA의 기능들을 대부분 담당하는 프로세스.
void fpga_process(pid_t*,short *,struct fb_var_screeninfo,int);

//서버와 통신이 되지않았을 경우 재접속을 시도하게되는 프로세스.
void retry(void);

//초음파의 상, 하 동작을 담당하는 서보모터 용 pwm 프로세스.
void pwm_updown_process(void);

//초음파의 좌, 우 동작을 담당하는 서보모터 용 pwm 프로세스.
void pwm_right_left_process(void);

//터치스크린 상단에 버튼을 그려주는 함수. (x좌표 위치//색상//'X', 'O', '▲', '▼'//)
void make_button(int, short*, struct fb_var_screeninfo, short, int);

//터치스크린 밝기 조절을 담당하는 함수.
int set_brightness(int, int);


void main(int argc, char **argv)
{
	pid_t pid[6];
	/*================================================
	= 0. 서버에 데이터 주는 프로세스
	= 1. 서버에서 요청을 받는 프로세스 (추후에 구현)
	= 2. 거리측정 하면서 원을 그리는 프로세스
	= 3. 직접 레이더 환경설정하는 프로세스 (터치 이벤트 관련) 
	= 4. pwm up_down 프로세스
	= 5. pwm left_right 프로세스
	=
	= 6. Dot_matrix, 7-seg, Text LCD, Buzzer, Key 관련??
	==================================================*/
	
	int i;
	int frame_fd, check, str_size;
	struct fb_var_screeninfo fvs; 
	short *pfbdata;
	char cmd_buffer[80];

	//Initialize shared memmory.
	mmset();


	//Initialize touchscreen's pwm val/ 
	system("echo 100 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");


	//---------------------------- Initializesocket. ------------------------------------------
	client_socket  = socket( PF_INET, SOCK_STREAM, 0);

	if((frame_fd = open(FRAME_BUFF,O_RDWR))<0) {
        	perror("Frame Buffer Open Error!");
        	exit(1);
    	}
	if((check=ioctl(frame_fd,FBIOGET_VSCREENINFO,&fvs))<0) {
		perror("Get Information Error - VSCREENINFO!");
		exit(1);
	}

	if( -1 == client_socket)
	{
		printf( "socket fail\n");
		exit(1);
	}

	//서버 IP는 192.168.43.110, 포트는 4000 으로 설정.
	memset( &server_addr, 0, sizeof( server_addr));
	server_addr.sin_family     = AF_INET;
	server_addr.sin_port       = htons(4000);
	server_addr.sin_addr.s_addr= inet_addr( "192.168.43.110");
    *connected_flag = 1;

	if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) )
	{
      	printf( "WiFi error!\n");
		*connected_flag = 0;
   	}
	//-----------------------------------------------------------------------------------------

	//공유변수들의 초기값 설정.
	*warn_line = 20;
	*buz_flag = 0;
	*up_down_flag = 1;
	*up_down_pwm = 1600;
	*detect_dir = 0;
	*detect_cm = 0;
	*send_flag = 0;
	*receive_flag = 0;
	*t_x = 0;
	*t_y = 0;
	*deep = 0;
	*semaphore = 1;
	pfbdata = (short *)mmap(0, fvs.xres*fvs.yres*(sizeof(short)), PROT_READ | \
		PROT_WRITE, MAP_SHARED, frame_fd, 0);

	for(i = 0; i < 6; i++)
	{
		//자식 프로세스를 하나씩 만들면서 각 자식 프로세스는 반복문을 탈출.
		pid[i] = fork();
		if(pid[i] == 0)
			break;
	}


	if(pid[0] == 0)
	{
		//서버에 send 를 담당하는 프로세스가 실행되기 전 소켓의 상태를 검사하게됨.
		retry();
	}
	else if(pid[1] == 0)
	{
		//서버에서 receive를 담당하게 되는 프로세스. 현재 미구현.
		(void)signal(SIGINT, user_signal1);
		client_receive_process();
		while(!quit){}
	}
	else if(pid[2] == 0)
	{
		//레이더의 동작 담당 프로세스.
		(void)signal(SIGINT, user_signal1);
		radar_process(pfbdata, fvs);
	}
	else if(pid[3] == 0)
	{
		//레이더의 세팅을 담당하는 프로세스. (터치 이벤트 처리)
		set_process();
	}
	else if(pid[4] == 0)
	{
		//초음파의 상, 하 동작을 담당하는 서보모터 용 pwm 프로세스.
		(void)signal(SIGINT, user_signal1);
		pwm_updown_process();
	}
	else if(pid[5] == 0)
	{
		//초음파의 좌, 우 동작을 담당하는 서보모터 용 pwm 프로세스.
		(void)signal(SIGINT, user_signal1);
		pwm_right_left_process();
	}
	else
	{
		//FPGA의 기능들을 대부분 담당하는 프로세스.
		(void)signal(SIGINT, user_signal1);
		fpga_process(pid, pfbdata,fvs,frame_fd);
		/*
		=============================================================
		= 가장 중심이 되는 프로세스로 선정하였음.						=
		= 깔끔하고 안전한 데모를 위해 이 프로세스에서 종료버튼을 누르면,	=
		= GAM-Z 레이더 클라이언트, 서버 모두 종료하게 됨.				=
		=============================================================
		*/
		printf("logout\n");
	}
}

void client_send_process(void)
{
	printf("I'm client send process\n");
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	int i;

	//프로그램 시작 전 현재시각이 제대로 설정되었는지 확인하기위해 serial창에 시간 정보를 출력.
	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	//전송하게 될 버퍼를 초기화.
    memset(data_buf,1,sizeof(*data_buf)*70);
	while(!quit)
	{
		//공유변수를 통해 전송 할 데이터가 있는 지 확인.
		if(*send_flag)
		{
			t = time(NULL);
			tm = *localtime(&t);
			*data_semaphore = 1;
			write(client_socket, "data_in",strlen("data_in")+1);
			usleep(10);
			//데이터를 보낼 당시의 시간값을 같이 전송, 스크린샷의 시간정보 역할을 함.
			*(data_buf + 61) = tm.tm_year;
			*(data_buf + 62) = tm.tm_mon + 1;
			*(data_buf + 63) = tm.tm_mday;
			*(data_buf + 64) = tm.tm_hour;
			*(data_buf + 65) = tm.tm_min;
			*(data_buf + 66) = tm.tm_sec;
			
			//현재 설정된 감지거리에 관한 정보도 같이 전송.
			*(data_buf + 67) = *warn_data;
			*(data_buf + 68) = 0;

			/*
			=============================================================
			= 70바이트를 한 프레임으로 설정,								=
			= 추후 여러개의 보드를 사용하여, 몇 번째 보드인지 명시해주기위해 =
			= 2바이트의 여유공간을 남겨 둠.								=
			=============================================================
			*/

			//데이터 전송.
			write(client_socket, data_buf, 70);
			*send_flag = 0;
			*data_semaphore = 0;
		}
	}
}

void client_receive_process()
{
	/*
	=====================================================
	=    추후에 양방향 통신을 위해 남겨둔 프로세스			=
	=====================================================
	*/
}

void radar_process(short *pfbdata,struct fb_var_screeninfo fvs)
{
	/*
	=====================================================================
	=	레이더를 측정하면서 프레임 버퍼에 반원을 그려주는 프로세스.			=
	=	알고리즘을 조금 더 개선하고 필요없는 변수를 삭제해주는것이 최종 목표.	=
	=====================================================================
	*/

	int tan_list[16] = {0, 524, 1051, 1583, 2125, 2679, 3249, 3838, 4452, 5095, 5773, 6494, 7265, 8097, 9004, 10000};
	int r[61] = {0}, i, repx, repy, x, y, offset, temp;
	int fd_radar, temp_warn,min, temp_dist, warn_val,str_size;
	char buf[10] = {0};
	int dev, dev_dot;
	unsigned char retval, data[4] = {0};
	short color;
	
	//==================== 필요한 문자 드라이버 노드 오픈. ====================
	dev = open("/dev/fpga_fnd", O_RDWR);
	if(dev <0){
		printf("FND OPEN ERROR\n");
	}

	write(dev, &data, 4);		//FND초기화해서 0000으로 값을 넣어줌.

	dev_dot = open(FPGA_DOT_DEVICE, O_WRONLY);
	if (dev_dot<0) {
		printf("Device open error : %s\n",FPGA_DOT_DEVICE);
		exit(1);
	}
	

	fd_radar = open("/dev/us",O_RDWR);
	if(fd_radar < 0){
		perror("/dev/us error");
	}else{
		printf("< us device has been detected >\n");
	}


	if((unsigned)pfbdata == (unsigned)-1) {
        	perror("Error Mapping!\n");
    }
	//=============================================================

	//흰색 반원 테두리 그려주는 반복문.
	for(repy = 599; repy >= 0; repy--)
	{
		offset = repy * fvs.xres;
		for(repx = 0; repx < 1024; repx++)
		{
			x = repx - 512;
			y = 599 - repy;
			if((x)*(x) + (y)*(y) <= 250000 + 10000 && (x*x)+(y*y) > 250000){
				while(*semaphore){}
				*(pfbdata+offset+repx) = 0xffff;
			}
		}
	}

	//이 프로세스가 radar 프로세스라는것을 명시
	printf("I'm radar process\n");

	//필요한 변수 초기화.
	str_size=sizeof(fpga_number[0]);
	*side_pwm = 500;
	sleep(2);

	//pwm값에 따라 서보모터가 제대로 초기화되도록 여유를 준 뒤, 레이더 구동 알고리즘 시작.
	while(!quit){
		*side_pwm = 480;
		usleep(500000);
		*buz_flag = 0;
		temp_warn = (*warn_line) * 2;
		warn_val = *warn_line;
		*warn_data = warn_val;
		for(repy = 599; repy >= 0; repy--)
		{
			offset = repy * fvs.xres;
			for(repx = 0; repx < 1024; repx++)
			{
				x = repx - 512;
				y = 599 - repy;
				if((x)*(x) + (y)*(y) <= 250000){
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn+ 10)  && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					}
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(y >=(-2 * x) - 1 &&  y <= (-2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= (-1 * x) - 2 &&  2 * y <=(-1 *  x) + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(repx == 512){
						*(pfbdata+offset+repx) = 0xffff;

					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else {
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0x0000;
					}
				}
			}	
		}

		/*
		==================== 레이더 감지 결과를 나타내는 알고리즘 전반적인 설명 =========================
		=	1. dot matrix 는 45도마다 변경됨.															=
		=	2. 7-segment 는 매번 측정된 거리를 나타냄.													=
		=	3. 초음파로 거리를 측정할때 값이 0 or 250이면 값이 튀었을 가능성이 높기때문에 재 측정.		=
		=	4. (3)의 경우가 아닌경우 한번 더 측정한 후 더 크게 측정된 값을 측정된 거리로 함.				=
		=		ㄴ> 평균값을 산출하는 것 보다 빠르고 좀 더 정확했음. 아마 서보모터의 진동때문이라고 생각.	=
		=	5. 전체 싸이클은 반원을 두 번 그리게되고 한 번 반원을 그릴때는 네번에 나눠서 그림.			=
		=	    ㄴ> 정리 해보면, 총 8번으로 나누게 됨.													=
		=	6. 아직 최적화가 완벽한건 아니기 때문에 코드도 길고 정돈이 안되있음.							=
		=	7. 측정된 거리가 *warn_line 이라는 설정된 감지거리보다 작은 값일 경우 알람 flag가 1이됨.		=
		=	8. 0~180도로 원을 그릴때는 이전 기록을 초기화시키고 180~0으로 갈떄는 덮어씌우면서 기록.		=
		=============================================================================================
		*/

		write(dev_dot,fpga_number[0],str_size);
		for(i = 0; i <= 15; i++){
			//dot matrix 관련 설정
			if(i == 12)
				write(dev_dot,fpga_number[1],str_size);
			
			//첫 번째 거리 측정.
			read(fd_radar,buf,2);

			//두 번째 거리 측정.
			if(buf[0] == 250 || buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}

			//측정 후 pwm값을 변경.
			*side_pwm +=30; 
			//pwm값 변경이 조금 더 잘 적용되도록 딜레이를 조금 넣어줌 (20ms)
			usleep(20000); 

			//감지거리에 따른 색 설정. 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000001111100000;
			}

			//변수 이름은 min이지만 사실 7-seg에 표시하던 코드를 그대로 가져와서 아직 정리를 못함.
			//7-seg에 값을 뿌려주는 과정.
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);

			//측정은 250cm이 max인데 반지름의 화소는 500개이기 때문에 스케일링을 맞춰주는 작업.
			r[i] = buf[0] * 2;

			//원 + 감지 선 들을 그려주는 과정.
			for(repy = 599; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 513; repx < 1013; repx++)
				{
					x = repx - 512;
					y = 599 - repy;
					temp = (int) (y*10000)/x;
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					}
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if(i == 0){
						if(temp <= tan_list[i]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[i]*r[i])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[i]*r[i])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
		}
		

		//이전과 비슷한 과정이 반복.
		for(i = 15; i > 0; i--){
			read(fd_radar,buf,2);
			if(i == 5)
				write(dev_dot,fpga_number[2],str_size);
			if(buf[0] == 250|| buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}
			*side_pwm +=30; 
			usleep(20000); 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000001111100000;
			}
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);
			r[15 + (16 - i)] = buf[0] * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 513; repx < 1013; repx++)
				{
					x = repx - 512;
					y = 599 - repy;
					temp = (int) (x*10000)/y;
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} 
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else {
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[15 + (16 - i)]*r[15 + (16 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
	
				}
			}
		}
		
		for(repy = 599 ; repy > 100; repy--)
		{
			offset = repy * fvs.xres;
			while(*semaphore){}
			*(pfbdata+offset+512) = 0xffff;
		}
		
		//leftcircle
		for(i = 1; i <= 15; i++){
			read(fd_radar,buf,2);
			if(i == 8)
				write(dev_dot,fpga_number[3],str_size);
			if(buf[0] == 250|| buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}
			*side_pwm +=30; 
			usleep(20000); 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000001111100000;
			}
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);
			r[30 +  i] = buf[0] * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 0; repx < 512; repx++)
				{
					x = repx - 512;
					x = -1 * x;
					y = 599 - repy;
					temp = (int) (x*10000)/y;
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} 				
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
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
			read(fd_radar,buf,2);
			if(i == 12)
				write(dev_dot,fpga_number[4],str_size);
			if(buf[0] == 250|| buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}
			*side_pwm +=30; 
			usleep(20000); 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000001111100000;
			}
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);
			r[46 +  (15 - i)] = buf[0] * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 0; repx < 512; repx++)
				{
					x = repx - 512;
					x = -1*x;
					y = 599 - repy;
					temp = (int) (y*10000)/x;				
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} 
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[46 + (15 - i)]*r[46 + (15 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
		}
		//경고음이 울리게되면 세팅된 거리 이내에 물체가 감지됐다는 뜻.
		//다라서 아래의 if문이 동작하며 send 담당 프로세스에게 보낼 데이터가 있다고 알림.
		if(*buz_flag)
		{
			for(i = 0; i < 61; i++)
				*(data_buf + i) = (r[i] / 2) + 1;
			*buz_flag = 0;
			*send_flag = 1;
		}
		//먼저 반바퀴 돌고 1초 대기 후 다시 반바퀴 측정.
		//이 루틴에서는 그 전의 측정기록들을 덮어쓰기 하면서 재 측정한다.
		sleep(1);
		//printf("side pwm = %d\n",*side_pwm);	
		
		//re left circle 
		for(i = 0; i <= 15; i++){
			read(fd_radar,buf,2);			
			if(i == 12)
				write(dev_dot,fpga_number[3],str_size);
			if(buf[0] == 250|| buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}
			*side_pwm -=30; 
			usleep(20000); 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000000000011111;
			}
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);
			r[44 +  (15 - i)] = buf[0] * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 0; repx < 512; repx++)
				{
					x = repx - 512;
					x = -1*x;
					y = 599 - repy;
					temp = (int) (y*10000)/x;
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					}
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if(i == 0){
						if(temp <= tan_list[i]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[44 + (15 - i)]*r[44 + (15 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
							}
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[44 + (15 - i)]*r[44 + (15 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
		}
	
		for(i = 15; i >= 1; i--){
			read(fd_radar,buf,2);
			if(i == 6)
				write(dev_dot,fpga_number[2],str_size);
			if(buf[0] == 250|| buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}
			*side_pwm -=30; 
			usleep(20000); 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000000000011111;
			}
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);
			r[44 -  (15 - i)] = buf[0] * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 0; repx < 512; repx++)
				{
					x = repx - 512;
					x = -1*x;
					y = 599 - repy;
					temp = (int) (x*10000)/y;
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} else if(i == 0){
						if(temp <= tan_list[i]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[44 - (15 - i)]*r[44 + (15 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
							}
					}
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[44 - (15 - i)]*r[44 - (15 - i)])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
		}
		for(repy = 599 ; repy > 100; repy--)
		{
			offset = repy * fvs.xres;
			while(*semaphore){}
			*(pfbdata+offset+512) = 0xffff;
		}
		//right circle
		for(i = 1; i<=  15; i++){
			read(fd_radar,buf,2);
			if(i == 8)
				write(dev_dot,fpga_number[1],str_size);
			if(buf[0] == 250 || buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}
			*side_pwm -=30; 
			usleep(20000); 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000000000011111;
			}
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);
			r[30 -   i] = buf[0] * 2;
			for(repy = 598; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 513; repx < 1013; repx++)
				{
					x = repx - 512;
					y = 599 - repy;
					temp = (int) (x*10000)/y;
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10)  && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					} 
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[30 -  i]*r[30 - i])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
	
				}
			}
		}
	
		for(i = 15; i > 0; i--){
			read(fd_radar,buf,2);
			if(i == 12)
				write(dev_dot,fpga_number[0],str_size);
			if(buf[0] == 250 || buf[0] == 0)
				read(fd_radar,buf,2);
			else{
				temp_dist = buf[0];
				read(fd_radar,buf,2);
				buf[0] = (buf[0] > temp_dist) ? buf[0] : temp_dist; 
			}
			*side_pwm -=30; 
			usleep(20000); 
			if(buf[0] < warn_val){
				color = 0b1111100000000000;
				*buz_flag = 1;
			} else {
				color = 0b0000000000011111;
			}
			min = buf[0];
			data[1] = min/100;
			data[2] = (min%100)/10;
			data[3] = min%10;
			write(dev,&data,4);
			r[i-1] = buf[0] * 2;

			for(repy = 599; repy >= 100 ; repy--)
			{
				offset = repy * fvs.xres;
				for(repx = 513; repx < 1013; repx++)
				{
					x = repx - 512;
					y = 599 - repy;
					temp = (int) (y*10000)/x;
					if((x)*(x) + (y)*(y) <= (temp_warn + 10)*(temp_warn + 10) && (x*x)+(y*y) > (temp_warn)*(temp_warn)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0b1111111111100000;
					}
					else if(y >=(2 * x) - 1 &&  y <= (2*x) + 1)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if(2 * y >= x - 2 &&  2 * y <= x + 2)
					{
						if((x)*(x) + (y)*(y) <= 500*500){
							while(*semaphore){}
							*(pfbdata+offset+repx) = 0xffff;
						}
					}
					else if((x)*(x) + (y)*(y) <= (100 + 3)*(100+ 3)  && (x*x)+(y*y) > (100)*(100)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (200 + 3)*(200+ 3)  && (x*x)+(y*y) > (200)*(200)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (300 + 3)*(300+ 3)  && (x*x)+(y*y) > (300)*(300)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if((x)*(x) + (y)*(y) <= (400 + 3)*(400+ 3)  && (x*x)+(y*y) > (400)*(400)){
						while(*semaphore){}
						*(pfbdata+offset+repx) = 0xffff;
					}
					else if(i == 0){
						if(temp <= tan_list[i]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[i-1]*r[i-1])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
					else{
						if(temp <= tan_list[i] && temp >= tan_list[i-1]){
							while(*semaphore){}
							if((x)*(x) + (y)*(y) <= r[i-1]*r[i-1])
								*(pfbdata+offset+repx) = color;
							else if((x)*(x) + (y)*(y) <= 500*500)
								*(pfbdata+offset+repx) = 0;
						}
					}
				}
			}
		}
		printf("side pwm = %d\n",*side_pwm);	
		if(*buz_flag)
		{
			for(i = 0; i < 61; i++)
				*(data_buf + i) = (r[i] / 2) + 1;
			*buz_flag = 0;
			*send_flag = 1;
		}
	}

	//KILL 과정에서 while반복문을 정상적으로 탈출하고 열려있던 문자 디바이스 노드 및 맵핑들을 처리해줌.
	munmap(pfbdata, fvs.xres*fvs.yres*(sizeof(short)));
	close(frame_fd);
	close(fd_radar);
	close(dev_dot);
	close(dev);
}

void set_process(void)
{
	int fd, ret;
	//터치 스크린 이벤트 경로를 입력.
	const char* evdev_path = "/dev/input/by-path/platform-imx-i2c.2-event";
    struct input_event iev[3];

    fd = open(evdev_path, O_RDONLY);
    if(fd < 0) {
        perror("error: could not open evdev\n");
    }
	printf("I'm set process\n");
	
	while(1)
    {
		//이벤트 발생에서 오는 값들을 디코딩해서 세가지 상태(1. 눌림, 2. 눌리는중, 3. 때어짐)를 구분
        ret = read(fd, iev, sizeof(struct input_event)*3);
        if(ret < 0) {
            perror("error: could not read input event");
            break;
        }

        if(iev[0].type == 1 && iev[1].type == 3 && iev[2].type == 3)
        {
			//1번 상태 감지. 값을 공유메모리에 상태 기록.
            *t_x = iev[1].value;
			*t_y = iev[2].value;
			//printf("touch!!!!\n");
            //printf("x = %d, y = %d \n",*t_x,*t_y);
        }
        else if(iev[0].type == 0 && iev[1].type == 1 && iev[2].type == 0)
        {
	        //3번상태 감지, 공유메모리에 상태 기록.
			*deep = 0;
        }
		else if(iev[0].type == 0 && iev[1].type == 3 && iev[2].type == 0 ||\
	        iev[0].type == 3 && iev[1].type == 3 && iev[2].type == 0)
        {
	        //2번 상태 감지, 공유메모리에 상태 기록.
			*deep = 1;
        }
    }
	close(fd);
	//touch screen
}

void fpga_process(pid_t *temp, short *pfbdata, struct fb_var_screeninfo fvs, int frame_fd)
{
	int dev, flag = 1,i, mute = 0,dev_text, dev_buzzer;
	int buff_size, bright = 100, str_size;
	int offset,x,y,repx,repy, buz;	
	char str[16]={0};
	unsigned char push_sw_buff[MAX_BUTTON];
	unsigned char string[32];
	printf("I'm fpga process\n");
	
	dev = open("/dev/fpga_push_switch", O_RDWR);
	if (dev<0){
		printf("Device Open Error\n");
		close(dev);
		flag = 0;
	}

	dev_buzzer = open("/dev/fpga_buzzer", O_RDWR);
    if (dev_buzzer < 0) {
	    printf("Device open error : %s\n","/dev/fpga_buzzer");
        exit(1);
    }

	dev_text = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
    if (dev_text < 0) {
        printf("Device open error : %s\n",FPGA_TEXT_LCD_DEVICE);
    }

	//버튼을 만들때 다른 프로세스에서 접근하지 못하도록 잠금 설정.
	*semaphore = 1;

	//버튼을 그려주는 함수 수행.
    make_button(705, pfbdata, fvs,puple,2);
	make_button(860, pfbdata, fvs,puple,3);
	make_button(10, pfbdata, fvs,puple,mute);
    make_button(280, pfbdata, fvs,puple,2);
	make_button(435, pfbdata, fvs, puple, 3);

	//버튼 다 그리고 잠금 해제.
	*semaphore = 0;
	
	//Text LCD에 초기설정상황 출력.
	buff_size=9;
	memset(string,0,sizeof(string));
	sprintf(str,"warn line: %dcm",*warn_line);
	str_size=strlen(str);
	strncat(string,str,str_size);
    memset(string+str_size,' ',LINE_BUFF-str_size);
	if(mute)
		strcpy(str,"mute mode: on");
	else
		strcpy(str,"mute mode: off");
	str_size=strlen(str);
	strncat(string,str,str_size);
    memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
	write(dev_text,string,MAX_BUFF);
	
	//up_down pwm 조정 시 쓰이는 flag를 확실하게 0으로 명시해줌.
	*up_down_flag = 0;
	while(1)
	{	
		//혹시모를 채터링이 무서워 10ms 딜레이를 줌.
		usleep(10000);

		//스위치에서 입력값을 스캔
		read(dev, &push_sw_buff,9);

		//레이더 프로세스에서 측정이 됐다면, 그리고 mute모드가 아니라면 알람을 울림.
		if(!mute && *buz_flag)
		{
			buz = 1;
			write(dev_buzzer,&buz,1);
			usleep(100000);
			buz = 0;
			write(dev_buzzer,&buz,1);
			usleep(100000);	
		} 

		//스위치 입력 종류별로 맵핑
		if(push_sw_buff[0] == 1)
		{
			if(*up_down_pwm < 2200)
				*up_down_pwm += 50;
			*up_down_flag = 1;
			usleep(50000);
			*up_down_flag = 0;
			printf("current pwm val = %d\n",*up_down_pwm);
		}
		else if(push_sw_buff[1] == 1)
		{
			if(*up_down_pwm > 1400)
				*up_down_pwm -= 50;
			*up_down_flag = 1;
			usleep(50000);
			*up_down_flag = 0;
			printf("current pwm val = %d\n",*up_down_pwm);			
		}
		else if(push_sw_buff[8] == 1)
			break;	

		//deep신호는 현재 버튼이 눌리고있음을뜻함.
		if(*deep)
		{
			*semaphore = 1;
			if((*t_x >= 705 && *t_x < 855) && (*t_y >= 0 && *t_y < 80))
				make_button(705, pfbdata, fvs,deep_puple,2);
			else if((*t_x >= 860 && *t_x < 1010) && (*t_y >= 0 && *t_y < 80))
				make_button(860, pfbdata, fvs,deep_puple,3);
			else if((*t_x >= 10 && *t_x < 160) && (*t_y >= 0 && *t_y < 80))
			{	
				make_button(10, pfbdata, fvs,deep_puple,mute);
				mute = (mute) ? 0:1;
			}
			else if((*t_x >= 280 && *t_x < 430) && (*t_y >= 0 && *t_y < 80))
				make_button(280, pfbdata, fvs,deep_puple,2);
			else if((*t_x >= 435 && *t_x < 585) && (*t_y >= 0 && *t_y < 80))
				make_button(435, pfbdata, fvs,deep_puple,3);
			*semaphore = 0;
		
			//버튼이 때어지길 기다리고있는 상태.
			while(*deep){}
			
			*semaphore = 1;
			if((*t_x >= 705 && *t_x < 855) && (*t_y >= 0 && *t_y < 80))
			{
				make_button(705, pfbdata, fvs,puple,2);
				bright = set_brightness(1,bright);
			}
			else if((*t_x >= 860 && *t_x < 1010) && (*t_y >= 0 && *t_y < 80))
			{
				make_button(860, pfbdata, fvs,puple,3);
				bright = set_brightness(0,bright);
			}
			else if((*t_x >= 10 && *t_x < 160) && (*t_y >= 0 && *t_y < 80))
				make_button(10, pfbdata, fvs,puple,mute);
			else if((*t_x >= 280 && *t_x < 430) && (*t_y >= 0 && *t_y < 80))
			{
				make_button(280, pfbdata, fvs,puple,2);
				if(*warn_line < 250)
					*warn_line += 10;
			}
			else if((*t_x >= 435 && *t_x < 585) && (*t_y >= 0 && *t_y < 80))
			{
				make_button(435, pfbdata, fvs,puple,3);
				if(*warn_line >= 20)
					*warn_line -= 10;
			}

			//실시간으로 Text LCD에 업데이트.
			*semaphore = 0;
			memset(string,0,sizeof(string));
			sprintf(str,"warn line: %dcm",*warn_line);
			str_size=strlen(str);
			strncat(string,str,str_size);
	        memset(string+str_size,' ',LINE_BUFF-str_size);
			if(mute)
				strcpy(str,"mute mode: on");
			else
				strcpy(str,"mute mode: off");
			str_size=strlen(str);
			strncat(string,str,str_size);
	        memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
			write(dev_text,string,MAX_BUFF);
		}

	}
	if(*connected_flag){
		*data_semaphore = 1;
		write(client_socket, "logout", strlen("logout") + 1);
		*data_semaphore = 0;
		close(client_socket);
	}
	sleep(1);
	for(i = 0; i < 6; i++)
		kill(temp[i],SIGINT);
	sleep(1);
	close(dev_buzzer);
	close(dev);
	close(dev_text);	
	system("echo 100 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
	//정상 종료시 모든 메모리 할당 해제.
	un_mmap();
}

//pwm담당하는 프로세스. 공유메모리에 설정된 값에 다라서 동작.
void pwm_updown_process(void)
{
	int fd, i;
	char buf[2] = {0};
	fd = open("/dev/up_down_pwm", O_RDWR);
	if(fd<0){
		perror("/dev/up_down_pwm error");
        exit(-1);
    }else{
        printf("< ext_sens device has been detected >\n");
    }

	(void)signal(SIGINT, user_signal1);
	printf("I'm updown process\n");
	while(!quit)
	{
		if(*up_down_flag){
			for(i = 0; i<200 ; i++){
				buf[0] = 1;
				read(fd,buf,sizeof(buf));
				usleep(*up_down_pwm);
				buf[0] = 0;
				read(fd,buf,sizeof(buf));
				usleep(20000 - *up_down_pwm);
			}
		}
	}
	close(fd);
	printf("up_pwm_good bye~\n");
}

//pwm담당하는 프로세스. 공유메모리에 설정된 값에 다라서 동작.
void pwm_right_left_process(void)
{
	int fd;
	char buf[2] = {0};
	fd = open("/dev/left_right_pwm", O_RDWR);
	if(fd<0){
		perror("/dev/left_right_pwm error");
        exit(-1);
    }else{
        printf("< ext_sens device has been detected >\n");
    }

	(void)signal(SIGINT, user_signal1);
	printf("I'm right_left process\n");
	while(!quit)
	{
			buf[0] = 1;
			read(fd,buf,sizeof(buf));
			usleep(*side_pwm);
			buf[0] = 0;
			read(fd,buf,sizeof(buf));
			usleep(20000 - *side_pwm);
	}
	close(fd);
	printf("LR_pwm_good bye~\n");
}

void retry()
{
	//만약 처음 코드를 실행할 때 한번에 연결됐다면 이 함수는 그냥 지나치게 된다.
	if(*connected_flag)
		client_send_process();
	else{
		//연결이 될때까지, 혹은 종료 시그널이 올때까지 소켓통신을 한다. (최종 테스트를 해보니 완벽하게 구성되진 않은 것 같음)
		while(!*connected_flag && !quit)
		{
			sleep(10);
			memset( &server_addr, 0, sizeof( server_addr));
			server_addr.sin_family     = AF_INET;
			server_addr.sin_port       = htons(4002);
			server_addr.sin_addr.s_addr= inet_addr( "127.0.0.1");
	
			if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) )
			{
	      		printf( "WiFi error!\n");
	   		}
			else
			{
				printf("connected!!!\n");
				*connected_flag = 1;
				break;
			}
		}
	}
}

//공유변수 구현을 위해 메모리를 맵핑해주는 부분.
void mmset()
{	
	warn_line = mmap(NULL, sizeof *warn_line, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	buz_flag = mmap(NULL, sizeof *buz_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	up_down_pwm = mmap(NULL, sizeof *up_down_pwm, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	side_pwm = mmap(NULL, sizeof *side_pwm, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	detect_dir = mmap(NULL, sizeof *detect_dir, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	detect_cm = mmap(NULL, sizeof *detect_cm, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	send_flag = mmap(NULL, sizeof *send_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	receive_flag = mmap(NULL, sizeof *receive_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	connected_flag = mmap(NULL, sizeof *connected_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	t_x = mmap(NULL, sizeof *t_x, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	t_y = mmap(NULL, sizeof *t_y, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	deep = mmap(NULL, sizeof *deep, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	semaphore = mmap(NULL, sizeof *semaphore, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
	up_down_flag = mmap(NULL, sizeof *up_down_flag, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	data_buf = mmap(NULL, sizeof (*data_buf)*70, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	data_semaphore = mmap(NULL, sizeof *data_semaphore, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
	warn_data = mmap(NULL, sizeof *warn_data, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
}

//메모리 맵핑을 해제해주는 부분.
void un_mmap()
{
	munmap(warn_line ,sizeof *warn_line);
	munmap(buz_flag  ,sizeof *buz_flag );
	munmap(up_down_pwm ,sizeof *up_down_pwm);
	munmap(side_pwm ,sizeof *side_pwm);
	munmap(detect_dir ,sizeof *detect_dir);
	munmap(detect_cm ,sizeof *detect_cm);
	munmap(send_flag ,sizeof *send_flag);
	munmap(receive_flag ,sizeof *receive_flag);
	munmap(connected_flag ,sizeof *connected_flag);
	munmap(t_x ,sizeof *t_x);
	munmap(t_y  ,sizeof *t_y );
	munmap(deep  ,sizeof *deep );
	munmap(semaphore  ,sizeof *semaphore );
	munmap(up_down_flag  ,sizeof *up_down_flag );
	munmap(data_buf  ,(sizeof *data_buf) * 70 );
	munmap(data_semaphore  ,sizeof *data_semaphore );
	munmap(warn_data  ,sizeof *warn_data);
}

//버튼을 만들어주는 함수, y = 0부터 시작하는것은 고정, x값, 색상, 버튼안에 들어가는 무늬를 선택할 수 있다.
void make_button(int position_x, short* pfbdata, struct fb_var_screeninfo fvs, short color,int mod)
{
	int repy, repx, offset, i =0, inc = 0 , j = 40;
	
	
	if((unsigned)pfbdata == (unsigned)-1) {
		perror("Error Mapping!\n");
	}
	for(repy = 0 ; repy < 80 ; repy++)
	{
		offset = repy * fvs.xres;
		for(repx = 0 + position_x; repx < 150 + position_x; repx++)
		{
			if((repx >= 15+position_x || repy >= 15) && (repx < 135 + position_x || repy >= 15)&&\
				(repx >= 15+position_x || repy < 65) && (repx < 135 + position_x || repy < 65)) 
			{
				if(mod == 0)
				{	
					if((repx - position_x - 75)*(repx - position_x - 75) + (repy-40)*(repy-40) <= 529 &&\
						(repx - position_x - 75)*(repx - position_x - 75) + (repy-40)*(repy-40) >= 324)
						*(pfbdata+offset+repx) = 0;
					else
						*(pfbdata+offset+repx) = color;
				}
				else if(mod == 1)
				{	
					if((repx - position_x - 75 >= -20 && repx - position_x - 75 < 20) &&\
						(repy - 40 >= -20 && repy - 40 < 20)) 	
					{
						if((repy - 40 >= repx -75 - position_x -1) && (repy - 40 <= repx - 75 - position_x + 1))
							*(pfbdata+offset+repx) = 0;
						else if((repy - 40 <= -1 * (repx -75 - position_x) +1) && (repy - 40 >= -1*( repx - 75 - position_x) - 1))
							*(pfbdata+offset+repx) = 0;
						else
							*(pfbdata+offset+repx) = color;
					}
					else
						*(pfbdata+offset+repx) = color;
				}
				else if(mod == 2)
				{
					if((repy >= 20 && repy <= 60) && ((repx - position_x - 75 >= -1 * i) &&( repx - position_x - 75 <= i)))
					{
						*(pfbdata+offset+repx) = 0;
						inc = 1;
					}
					else
						*(pfbdata+offset+repx) = color;
				}
				else if(mod == 3)
				{
					if((repy >= 20 && repy <= 60) && ((repx - position_x - 75 >= -1 * j) &&( repx - position_x - 75 <= j)))
					{
						*(pfbdata+offset+repx) = 0;
						inc = 1;
					}
					else
						*(pfbdata+offset+repx) = color;
				}
			}
			else
			{
				if((repx - 15 - position_x)*(repx - 15 - position_x) + (repy-15)*(repy-15) <= 225)
					*(pfbdata+offset+repx) = color;
				else if((repx - 135 - position_x)*(repx - 135 - position_x) + (repy-15)*(repy-15) <= 225)
					*(pfbdata+offset+repx) = color;
				else if((repx - 15 - position_x)*(repx - 15 - position_x) + (repy-65)*(repy-65) <= 225)
					*(pfbdata+offset+repx) = color;
				else if((repx - 135 - position_x)*(repx - 135 - position_x) + (repy-65)*(repy-65) <= 225)
					*(pfbdata+offset+repx) = color;
				else
					*(pfbdata+offset+repx) = 0;	
			}
		}
		if(inc)
		{
			i++;
			j--;
			inc = 0;
		}
	}

}


//터치스크린 밝기를 조절하는 함수.
int set_brightness(int mod, int bright)
{
	if(mod)
	{
		if(bright == 80)
		{
			bright = 100;
			system("echo 100 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 60)
		{
			bright = 80;
			system("echo 80 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 40)
		{
			bright = 60;
			system("echo 60 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 20)
		{
			bright = 40;
			system("echo 40 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 10)
		{
			bright = 20;
			system("echo 20 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
	}
	else
	{
		if(bright == 100)
		{
			bright = 80;
			system("echo 80 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 80)
		{
			bright = 60;
			system("echo 60 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 60)
		{
			bright = 40;
			system("echo 40 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 40)
		{
			bright = 20;
			system("echo 20 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
		else if(bright == 20)
		{
			bright = 10;
			system("echo 10 > /sys/devices/platform/pwm-backlight.2/backlight/pwm-backlight.2/brightness");
		}
	}
	return bright;
}

