#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<dirent.h>
#include<sys/mman.h>//공유메모리 사용을 위해 추가한 헤더
#define INIT_MSG "==================\nHello! I'm an Embedded Os Server..\nPlease, select number\n1. regist\n2. login\n =================\n"
#define SERV_IP "220.149.128.100"
#define SERV_PORT 4370
#define BACKLOG 10
static int *Semaphore;
int main(void)
{
    int sockfd, new_fd;
    struct sockaddr_in their_addr;
    struct sockaddr_in my_addr;
    unsigned int sin_size;
    int count = 0;
    int rcv_byte;
    char buf[512];
    char id[20];
    char pw[20];
    char msg[512];
	char ip[20];
	char port[5];
    int val = 1;
    char msgg[40];
	char msgg2[40];
	FILE *listf, *datalist,*f,*upload,*f_dir;
	char new_id[40];
	char new_pw[40];
	char comp_id[40];
	char comp_pw[40];
	char id_file_name[40];
	int id_accept_flag = 0;
	int check;
	int i,temp_row;
	unsigned int row;
	DIR *dir_info;
	struct dirent *dir_entry;
	Semaphore = mmap(NULL, sizeof *Semaphore, PROT_READ | PROT_WRITE, 
        MAP_SHARED | MAP_ANONYMOUS, -1, 0); 

    *Semaphore = 1;

    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if(sockfd == -1){
        perror("Server-scoket() error lol!");
        exit(1);
    }
    else printf("Server-spcket() sockfd is OK..\n");
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERV_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero),0,8);

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char*)&val, sizeof(val)) <0)
	{
		perror("setsockopt");
		close(sockfd);
		return -1;
	}

	
    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Server-bind() error lol!");
        exit(1);
    }
    else printf("Server-bind() is OK...\n");

    if(listen(sockfd, BACKLOG) == -1)
    {
        perror("listen() error lol");
        exit(1);
    }
    else printf("listen() is OK...\n\n");
    
    
   while( 1){
        sin_size = sizeof(struct sockaddr_in);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        pid_t pid;
        pid = fork();
        if(pid > 0){            
			close(new_fd);
        }
        else if(pid == 0){
            printf("accept() is OK...\n\n");    
            while(1)
			{
				send(new_fd, INIT_MSG,strlen(INIT_MSG) + 1, 0);//1
            	rcv_byte = recv(new_fd,msgg,sizeof(buf),0);//먼저 로그인 할지 회원가입할지 고름4
				if(strcmp(msgg,"1")==0)
				{
					listf = fopen("ID_LIST.txt","r+");
					send(new_fd,"New ID = ",sizeof(msgg),0);
					rcv_byte = recv(new_fd,new_id,sizeof(new_id),0);
					while(1)
					{
						fgets(comp_id,sizeof(comp_id),listf);
						if(strcmp(comp_id,"\0") == 0)
						{
							id_accept_flag = 1;
							send(new_fd,"New PW = ",sizeof(msgg),0);
							rcv_byte = recv(new_fd,new_pw,sizeof(new_pw),0);
							break;
						}
						comp_id[strlen(comp_id) - 1] = '\0';
						if(strcmp(new_id,comp_id) == 0)
						{
							printf("Someone is trying to regist with an ID already in use.\n");
							send(new_fd,"already use ID",sizeof(msgg),0);
							break;
						}
					}
					if(id_accept_flag == 1)
					{
						id_accept_flag = 0;
						sprintf(id_file_name,"./userdata/%s.txt",new_id);
						datalist = fopen(id_file_name,"w");
						fprintf(datalist,"%s\n",new_id);
						fprintf(datalist,"%s\n",new_pw);
						fprintf(listf,"%s\n",new_id);
						printf("New ID is registed\n");
						fclose(datalist);
					}
					fclose(listf);
				}
				else if(strcmp(msgg,"2") ==0)
				{
					rcv_byte = recv(new_fd,id,sizeof(id),0);
					rcv_byte = recv(new_fd,pw,sizeof(pw),0);	
					listf = fopen("ID_LIST.txt","r");
					while(1)
					{
						if(fgets(comp_id,sizeof(comp_id),listf)==NULL)
						{
							printf("id error\n");
							send(new_fd,"login fail...plz check your ID" ,sizeof(msgg),0);
							break;
						}
						else
						{
							comp_id[strlen(comp_id) - 1] = '\0';
							if(strcmp(id,comp_id) == 0)
							{
								sprintf(id_file_name,"./userdata/%s.txt",id);
								f = fopen(id_file_name,"r");
								fgets(comp_id,sizeof(comp_id),f);
								fgets(comp_pw,sizeof(comp_pw),f);
								comp_id[strlen(comp_id)-1] = '\0';
								comp_pw[strlen(comp_pw)-1] = '\0';	
				
								if(strcmp(pw,comp_pw) != 0)
								{
									printf("PW error\n");
									send(new_fd,"login fail...plz check your PW" ,sizeof(msgg),0);
								}
								else
								{
									printf("%s login!\n",id);
									sprintf(msg,"- - - - - - - - - - - - - - -\n%s hi! select number!\n1. Upload your list (you must have upload dir)\n2. take a look at list(server)\n3. take a look at list(yours)\n4. logout\n5. Server setting\n- - - - - - - - - - - - - - -",id);
									send(new_fd,msg,sizeof(msg),0);
									memset(msgg,0,40);
									while(1)
									{
										memset(msg,0,512);
										rcv_byte = recv(new_fd,msgg,sizeof(msgg),0);
										if(strcmp(msgg,"1") == 0)
										{
											while(*Semaphore <= 0);
											*Semaphore = 0;
											sprintf(msgg,"./userslist/%s_list.txt",id);
											upload = fopen(msgg,"w");
											memset(buf,0,512);
											memset(msg,0,512);
											i = 1;
											while(1)
											{
												rcv_byte = recv(new_fd,buf,sizeof(buf),0);
												if(strcmp(buf,"END")==0)
												{
													break;
												}
												sprintf(msg,"%s",buf);
												fprintf(upload,"%s\n",msg);
												printf("%s upload file %d\n",id,i);
												sleep(1);
												i++;
												memset(msg,0,512);
											}
											fclose(upload);
											dir_info = opendir("./userslist");
											upload = fopen("Total_list.txt","w");
											i = 1;
											if(NULL != dir_info)
											{
												while(dir_entry = readdir(dir_info))
												{
													if(strstr(dir_entry->d_name,".txt") != NULL)
													{
														sprintf(msgg,"./userslist/%s",dir_entry->d_name);
														f_dir  = fopen(msgg,"r");
														memset(msgg,0,40);
														memset(msg,0,512);
														memset(buf,0,512);
														while(1)
														{
															if(fgets(msg,sizeof(msg),f_dir)==NULL)
															{
																memset(buf,0,512);
																memset(msg,0,512);
																break;
															}
															else
															{
																sprintf(buf,"%d. %s",i,msg);
																fprintf(upload,"%s",buf);
																memset(buf,0,512);
																memset(msg,0,512);
																i++;
															}
														}
													}
												}
											}
											closedir(dir_info);
											fclose(upload);
											fclose(f_dir);
											sprintf(msgg,"%s's upload success",id);
											printf("%s\n",msgg);
											memset(msgg,0,40);
											*Semaphore = 1;
										}
										else if(strcmp(msgg,"2") == 0)
										{
											upload = fopen("Total_list.txt","r");
											memset(msgg,0,40);
											while(1)
											{
												if(fgets(msgg,sizeof(msgg),upload)==NULL)
												{													
													send(new_fd,"END" ,sizeof(msgg),0);
													memset(msgg,0,40);
													break;
												}
												else
												{
													send(new_fd,msgg,sizeof(msgg),0);
													memset(msgg,0,40);
												}
											}
											fclose(upload);
											printf("%s take a look server list\n",id);
											rcv_byte = recv(new_fd,msgg,sizeof(msgg),0);
											if(strcmp(msgg,"2") == 0)
											{
												memset(msgg,0,40);
												//do nothing
											}
											else if(strcmp(msgg,"1") == 0)
											{
												memset(msgg,0,40);
												printf("%s try to download file\n",id);
												rcv_byte = recv(new_fd,msgg,sizeof(msgg),0);
												row = atoi(msgg); //문자열을 인트로 변경
												memset(msgg,0,40);
												upload = fopen("Total_list.txt","r");
												temp_row = 1;
												memset(msg,0,512);
												while(1)
												{
													fgets(msg,sizeof(msg),upload);
													if(temp_row == row)
													{
														if(strcmp(msg,"\0") == 0)
														{
															send(new_fd,"please check file number\n",sizeof(msg),0);
															break;
														}
														else
														{
															send(new_fd,"Uploader Information\n",sizeof(msg),0);
															send(new_fd,msg,sizeof(msg),0);
															memset(msg,0,512);// 다운로드 정보전송
															break;
														}
													}
													else
													{
														memset(msg,0,512);
														temp_row++;
													}												
												}
												memset(msgg,0,40);
												rcv_byte = recv(new_fd,msgg,sizeof(msgg)+1,0);
												printf("%s\n",msgg);
												fclose(upload);
											}
										}
										else if(strcmp(msgg,"3") == 0)
										{
											memset(msgg,0,40);
											sprintf(msgg,"%s take a look %s's list",id,id);
											printf("%s\n",msgg);
											memset(msgg,0,40);
										}
										else if(strcmp(msgg,"4") == 0)
										{
											memset(msgg,0,40);
											printf("%s logout\n",id);
											break;//elseif "4" == logout
										}
										else if(strcmp(msgg,"5") == 0)
										{
											memset(msgg,0,40);
											sprintf(msgg,"%s sets the download option.\n",id);
											memset(msgg,0,40);
											//do client code
										}
										memset(msgg,0,40);
										sprintf(msg,"- - - - - - - - - - - - - - -\n%s hi! select number!\n1. Upload your list (you must have upload dir)\n2. take a look at list(server)\n3. take a look at list(yours)\n4. logout\n5. Server setting\n- - - - - - - - - - - - - - -\n",id);
										sleep(0.1);										
										send(new_fd,msg,sizeof(msg),0);
									}
								}
								break;
							}
						}
					}
					fclose(f);
					fclose(listf);
					break;
				}
			}
            close(new_fd);
            exit(0);
        }
		else
		{
			printf("can't fork, error\n");
			exit(1);
		}
	}
	return 0;
}

