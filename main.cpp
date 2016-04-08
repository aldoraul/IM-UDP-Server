/* CS 423 Project 2
 UDP_SERVER
 Aldo Anaya
*/

//#include<stdio.h>
#include<cstdlib>
#include<cstdio>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<vector>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include<iostream>
#include"cipher.h"
#include"functions.h"

// get sockaddr, IPv4 or IPv6

#define MYPORT "23456"
#define MAXBUFFLEN 100
#define ADDR_LEN 50

struct active_user{
        std::string user;
        struct sockaddr_storage addr;

        active_user(std::string user1, struct sockaddr_storage addr1):
         user(user1), addr(addr1) {}

};


int main(void){

	printf("cool");
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int numbytes;
	int rv;
	struct sockaddr_storage their_addr;
	struct active_user user = active_user("", their_addr);
	std::vector<active_user> users;
	char buf[MAXBUFFLEN];
	//char strptr[ADDR_LEN];
	socklen_t addr_len;
	int yes = 1;
	
	memset(&hints, 0, sizeof hints);
	printf("memset cleared\n");

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	//hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo("192.168.10.200", MYPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	for(p = servinfo;p != NULL; p=p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("UDP_Server: socket");
			continue;
		}
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("UDP_Server: bind");
			continue;
		}
		break;
	}
	printf("now listening\n");

	if(p == NULL){
		fprintf(stderr, "UDP_Server: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);
	
	while(1){
		addr_len = sizeof their_addr;
		if((numbytes = recvfrom(sockfd, buf, MAXBUFFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len))== -1){
			perror("\trecvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';
		std::string decrypted = decryptMessage(buf);
		int msg_type = getMsgType(decrypted);
		int msg_num = getMsgNum(decrypted);
		//if((numbytes = sendto(sockfd, "Server", 6, 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
		//	perror("\tUDP_Server: sendto error");
		//	exit(1);
		//	}
		printf("msg type %d \n", msg_type);
		printf("msg num  %d \n", msg_num);
		std::string userName = get_user(decrypted);
		
		switch(msg_type){
			case 1:{
				char *message;
				std::string msg1 = "ack;" + std::to_string(msg_num) + ";";
				std::string msg_to_encrypt = "\nWelcome to the group " + userName +"\nMembers already loggen in are\n";
				user = active_user(userName, their_addr);
				users.push_back(user);
				for(std::vector<active_user>::iterator it = users.begin();it != users.end();it++){
					msg_to_encrypt = msg_to_encrypt + it->user + "\n";
					}
				std::string msgEncrypted = encryptMessage(msg_to_encrypt);
				msg1 = msg1 + msgEncrypted;
				message = new char[msg1.length()+1];
				strcpy(message, msg1.c_str());
				if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len))==-1){
					perror("\tUDP_Server: message1 sendto error");
					exit(1);
					}				
				break;
				}

			case 2:
			case 3:
			default:
				printf("not 1 2 or 3");			
			}

		}

		//for(std::vector<active_user>::iterator it = users.begin();it != users.end();it++){
		//	std::cout<<"name " << it->user << std::endl;
		//	}
	close(sockfd);
	return 0;
}


		
