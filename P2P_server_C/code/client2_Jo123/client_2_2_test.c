#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<dirent.h>
#include<signal.h> //자식 프로세스 강제 종료를 위해 추가한 헤더
#include<sys/mman.h> //공유메모리 사용을 위해 추가한 헤더
#define SERV_IP "220.149.128.100"
#define USER_IP "220.149.128.102"
#define SERV_PORT 4370
#define USER_PORT 4372
#define BACKLOG 10
static int *static_signal;
int main()
{
    int sockfd,p2p_fd,new_fd;
	unsigned int sin_size;
    int check;
    struct sockaddr_in dest_addr,my_addr;
	struct sockaddr_in their_addr,dest_addr_new;
	char *data;
	char list_data[5][20];
    int rcv_byte;
    char buf[512];
    char id[20];
    char pw[20];
    char msg[40];
	char temp[40];
	char down_data[512];
	FILE *upload,*f;
	DIR *dir_info;
	struct dirent *dir_entry;
	int index = 0;
	int result = 1;
	int val = 1;
	int error_flag = 0;
	static_signal = mmap(NULL, sizeof *static_signal, PROT_READ | PROT_WRITE, 
        MAP_SHARED | MAP_ANONYMOUS, -1, 0); 

    *static_signal = 0; 
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(sockfd ==-1)
	{
		perror("Client-socket() error LoL!");
		close(p2p_fd);
		exit(1);
	}
	else printf("Cliet-socket() sockfd is OK..\n");

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(SERV_PORT);
	dest_addr.sin_addr.s_addr = inet_addr(SERV_IP);

	memset(&(dest_addr.sin_zero), 0, 8);
	if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr))== -1)
	{
		perror("CLient-connect() error lol");
		close(p2p_fd);
		exit(1);
	}
	else printf("Clinet-conneft() is OK..\n\n");

	

	p2p_fd = socket(AF_INET, SOCK_STREAM,0);
    if(p2p_fd == -1){
        perror("Server-scoket() error lol!");
        exit(1);
    }
    else printf("Server-spcket() p2p_fd is OK..\n");
    
	my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(USER_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero),0,8);

	if(setsockopt(p2p_fd, SOL_SOCKET, SO_REUSEADDR,(char*)&val, sizeof(val)) <0)
	{
		perror("setsockopt");
		close(p2p_fd);
		return -1;
	}

    if(bind(p2p_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Server-bind() error lol!");
        exit(1);
    }
    else printf("Server-bind() is OK...\n");

    if(listen(p2p_fd, BACKLOG) == -1)
    {
        perror("listen() error lol");
        exit(1);
    }
    else printf("listen() is OK...\n\n");


	pid_t pid;
	pid = fork();
	if (pid > 0)
	{
		while(1)
		{
			rcv_byte = recv(sockfd,buf,sizeof(buf),0);//2
			printf("%s\n",buf);
			scanf("%s",msg);//로그인, 회원가입 선택
			send(sockfd,msg,strlen(msg)+1,0);//3
			if(strcmp(msg,"1") == 0)
			{
				memset(msg,0,40);
				rcv_byte = recv(sockfd,msg,sizeof(buf),0);
				printf("%s",msg);
				memset(msg,0,40);
				scanf("%s",id);
				send(sockfd,id,strlen(id)+1,0);
				//새로운 아이디 전송, 중복여부 판단하고 알려줄예정
				rcv_byte = recv(sockfd,msg,sizeof(buf),0);
				printf("%s",msg);
				if(strcmp(msg,"New PW = ") == 0 )
				{
					memset(msg,0,40);
					scanf("%s",pw);
					send(sockfd,pw,strlen(pw)+1,0);	
				}
				else
				{
					printf("\nerror\n");
					memset(msg,0,40);
				}
			}
			else if(strcmp(msg,"2") == 0)
			{
				printf("ID = ");
				memset(id,0,20);
				scanf("%s",id);
				printf("PW = ");
				memset(pw,0,20);
				scanf("%s",pw);
				send(sockfd,id,sizeof(id),0);
				send(sockfd,pw,sizeof(pw),0);
				memset(buf,0,512);
				rcv_byte = recv(sockfd,buf,sizeof(buf),0);
				printf("%s\n",buf);
				if(strstr(buf,"hi!") == NULL)
				{
					break;
				}
				else
				{
					*static_signal = 1;
					memset(msg,0,40);
					memset(buf,0,512);
					while(1)
					{
						scanf("%s",msg);
						send(sockfd,msg,sizeof(msg),0);
						if(strcmp(msg,"1") == 0)
						{
							dir_info = opendir("./upload");
							if(NULL != dir_info)//잘못된 dir이 아니라면!
							{
								while(dir_entry = readdir(dir_info))//파일을 하나씩 읽어줌 
								{
									if(strstr(dir_entry->d_name,".txt") != NULL)
									{
										sprintf(buf,"%s %s %s %d",dir_entry->d_name,id,USER_IP,USER_PORT);
										send(sockfd,buf,sizeof(buf),0);
										memset(buf,0,512);
									}
								}
								closedir(dir_info);
								send(sockfd,"END",sizeof(msg),0);
								printf("Upload success\n\n");
							}
						}
						if(strcmp(msg,"2") == 0)
						{
							memset(msg,0,40);
							printf("========list========\n");
							while(1)
							{
								rcv_byte = recv(sockfd,msg,sizeof(msg),0);
								if(strcmp(msg,"END") == 0)
								{
									memset(msg,0,40);
									printf("====================\n");
									break;
								}
								else
								{
									printf("%s",msg);
									memset(msg,0,40);
								}
							}
							printf("====================\n");
							printf("select number\n1. down load file\n2. quit\n");
							printf("====================\n");
							scanf("%s",msg);
							send(sockfd,msg,sizeof(msg),0);
							if(strcmp(msg,"2") == 0)
							{
								memset(msg,0,40);
								//do nothing
							}
							else if(strcmp(msg,"1") == 0)
							{
								memset(msg,0,40);
								printf("====================\n");
								printf("select file\n");
								printf("====================\n");
								scanf("%s",msg);
								send(sockfd,msg,sizeof(msg),0);
								memset(msg,0,40);
								memset(buf,0,512);
								rcv_byte = recv(sockfd,buf,sizeof(buf),0);
								if(strstr(buf,"Information") == NULL) //잘못된 번호를 누르면 유저 정보가 안옴
								{
									printf("%s\n",buf);
									memset(buf,0,512);
									sprintf(msg,"%s fail to download fail\n",id);
									send(sockfd,msg,sizeof(msg),0);
									memset(msg,0,40);
									//do nothing
								}
								else
								{
									memset(buf,0,512);
									rcv_byte = recv(sockfd,down_data,sizeof(down_data),0);
									data = strtok(down_data, " ");
									while (data != NULL)               // 자른 문자열이 나오지 않을 때까지 반복
									{
										sprintf(list_data[index],"%s",data);
										index++;
										data = strtok(NULL, " ");      // 다음 문자열을 잘라서 포인터를 반환
									}
									index = 0;
									if(strstr(id,list_data[2]) != NULL)
									{
										printf("you can't download yourfile\n");
										memset(msg,0,40);
										sprintf(msg,"%s fail to download fail\n",id);
										send(sockfd,msg,sizeof(msg),0);
										memset(msg,0,40);
									}
									else
									{
										printf("====================\n");
										printf("file : %s\n",list_data[1]);
										printf("user : %s\n",list_data[2]);
										printf("IP : %s\n",list_data[3]);
										printf("PORT : %s\n",list_data[4]);
										printf("====================\n");
										new_fd = socket(AF_INET, SOCK_STREAM,0);
										memset(msg,0,40);										
										if(new_fd ==-1)
										{
											perror("Client-socket() error LoL!");
											error_flag = 1;
										}
										else printf("Client-socket() new_fd is OK..\n");
											dest_addr_new.sin_family = AF_INET;
											dest_addr_new.sin_port = htons(atoi(list_data[4]));
											dest_addr_new.sin_addr.s_addr = inet_addr(list_data[3]);
										memset(&(dest_addr_new.sin_zero), 0, 8);
										if(connect(new_fd, (struct sockaddr *)&dest_addr_new, sizeof(struct sockaddr))== -1)
										{
											perror("Client-connect() error lol");
											error_flag = 1;
										}
										else printf("Client-connect() is OK..\n\n");
										if(error_flag == 1)//서버가 닫혀있으면 에러!
										{
											printf("%s's p2p server is closed\n",list_data[2]);
											memset(msg,0,40);
											sprintf(msg,"%s's download fail\n",id);
											send(sockfd,msg,sizeof(msg)+1,0);
											error_flag = 0;
										}
										else
										{
											memset(msg,0,40);
											rcv_byte = recv(new_fd,msg,sizeof(buf),0);
											if(strstr(msg,"Start") == NULL)
											{
												printf("%s\n",msg);
												memset(msg,0,40);
											}
											else if(strstr(msg,"Start") != NULL)
											{
												int temp_i = 1;
												printf("%s\n",msg);
												send(new_fd,list_data[1],sizeof(buf),0);
												sprintf(temp,"./download/%s",list_data[1]);
												f = fopen(temp,"w");
												memset(buf,0,512);
												while(1)
												{
													rcv_byte = recv(new_fd,buf,sizeof(buf),0);
													if(strcmp(buf,"END") == 0)
													{
														memset(buf,0,512);
														break;
													}
													else
													{
														fprintf(f,"%s",buf);
														memset(buf,0,512);
														printf("you download %d line\n",temp_i);
														temp_i++;
													}
												}
												printf("Download complete!\n");
												fclose(f);
											}
											memset(msg,0,40);
											send(new_fd,"end",sizeof(msg),0);//끝내는 시그널
											close(new_fd);
											sprintf(msg,"%s's download success\n",id);
											send(sockfd,msg,sizeof(msg),0);
										}
									}
								}
							}
						}
						else if(strcmp(msg,"3") == 0)
						{
							memset(msg,0,40);
							dir_info = opendir("./upload");
							if(NULL != dir_info)//잘못된 dir이 아니라면!
							{
								printf("========list========\n");
								while(dir_entry = readdir(dir_info))//파일을 하나씩 읽어줌 
								{
									if(strstr(dir_entry->d_name,".txt") != NULL)
									{
										printf("%s\n",dir_entry->d_name);
									}
								}
								printf("====================\n");
								closedir(dir_info);
							}
						}
						else if(strcmp(msg,"4") == 0)
						{
							memset(msg,0,40);
							break;//elseif "4" ==  logout
						}
						else if(strcmp(msg,"5") == 0)
						{
							memset(msg,0,40);
							printf("====================\n");
							printf("select number\n1. Download block\n2. Download permissinon\n");
							printf("====================\n");
							scanf("%s",msg);
							if(strcmp(msg,"1") == 0)
							{
								*static_signal = 0;
							}
							else if(strcmp(msg,"2") == 0)
							{
								*static_signal = 1;
							}
							else
							{
								printf("Plz check option number\n");
							}
						}
						memset(buf,0,512);
						rcv_byte = recv(sockfd,buf,sizeof(buf),0);
						printf("%s\n",buf);
						memset(buf,0,512);
					}
					break;
				}
			}
		}
		close(p2p_fd);
		close(sockfd);
		kill(pid,SIGINT);
		return 0;
	}
	else if (pid == 0)
	{
		while(1)
		{
			sin_size = sizeof(struct sockaddr_in);
			new_fd = accept(p2p_fd, (struct sockaddr *)&their_addr, &sin_size);
			pid_t pid_1;
			pid_1 = fork();
			int down_load_flag = 0;
			if (pid_1 > 0)
			{
				close(new_fd);
			}
			else if(pid_1 == 0)
			{
				if(*static_signal == 0)
				{
					send(new_fd,"Server is closed..",sizeof(msg),0);
				}
				else
				{
					send(new_fd,"Download Start!",sizeof(msg),0);
					printf("accept() is OK...\n\n");
					memset(msg,0,40);
					rcv_byte = recv(new_fd,msg,sizeof(msg),0);//파일명
					printf("%s\n",msg);
					dir_info = opendir("./upload");
					if(NULL != dir_info)//잘못된 dir이 아니라면! // upload 폴더에 파일이 있는지 다시한번 확인!
					{
						while(dir_entry = readdir(dir_info))//파일을 하나씩 읽어줌 
						{
							if(strcmp(dir_entry->d_name,msg) ==0)
							{
								down_load_flag = 1;
							}
						}
						closedir(dir_info);
					}
					if(down_load_flag == 1)
					{
						int temp_i = 1;
						printf("Download Start!\n");
						sprintf(temp,"./upload/%s",msg);
						f = fopen(temp,"r");
						memset(msg,0,40);
						memset(buf,0,512);
						while(1)
						{
							fgets(buf,sizeof(buf),f);
							if(strcmp(buf,"\0") == 0)
							{
								send(new_fd,"END",sizeof(msg),0);
								memset(buf,0,512);
								break;
							}
							else
							{
								send(new_fd,buf,sizeof(buf),0);
								memset(buf,0,512);
								printf("Your file downloaded %d line\n",temp_i);
								temp_i++;
								sleep(1);
							}
						}
						printf("Download End!\n");
						fclose(f);
					}
					else
					{
						send(new_fd,"Download Fail!",sizeof(msg),0);
						printf("fail!\n");
					}
					rcv_byte = recv(new_fd,msg,sizeof(msg),0);
				}
				close(new_fd);
				exit(0);
			}
		}
	}
	else
	{
		printf("can't fork, error\n");
		exit(1);
	}
}