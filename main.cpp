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
#include<iostream>
#include<fstream>
#include<ctime>
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

	time_t t;
	char* charTime;
	std::ofstream myfile;
	char s[INET6_ADDRSTRLEN];	
	int slotNum = 1;
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
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0){
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
		//printf("msg type %d \n", msg_type);
		//printf("msg num  %d \n", msg_num);
		std::string userName = get_user(decrypted);
		
		char *message;
		std::string msg1 = "";
		std::string msg_to_encrypt = "";
		std::string msgEncrypted = "";
		std::string buddyName = "";
		std::string buddyMsg = "";
	//	myfile.open("log.txt");
	//	myfile.close();

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
		switch(msg_type){
			case 1:{
				
				// write initial message from client to log
				myfile.open("log.txt",std::fstream::app);
				t = time(NULL);
				charTime = ctime(&t);			
				myfile << "\nReceived msg (" << numbytes << ") " << msg_num << ": " << decrypted << " from " << s << " at "<< charTime;	
				myfile << userName << " has just entered the group in slot " << slotNum++ << "\n";

				//std::cout<< userName << " \n";		


				// send ack to client along with list of active users
				msg1 = "ack;" + std::to_string(msg_num) + ";";
				msg_to_encrypt = "\nWelcome to the group " + userName +"\nMembers already logged in are\n";
				user = active_user(userName, their_addr);
				users.push_back(user);
				for(std::vector<active_user>::iterator it = users.begin();it != users.end();it++){
					msg_to_encrypt = msg_to_encrypt + it->user + "\n";
					}
				msgEncrypted = encryptMessage(msg_to_encrypt);
				msg1 = msg1 + msgEncrypted;
				message = new char[msg1.length()+1];
				strcpy(message, msg1.c_str());
				if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len))==-1){
					perror("\tUDP_Server: message1 sendto error");
					exit(1);
					}


				// send msg to all active users about new client joining group
				msg1 = "Users;1;";
				msg_to_encrypt = userName + " just joined the group\n";
				msgEncrypted = encryptMessage(msg_to_encrypt);
				msg1 = msg1 + msgEncrypted;
				message = new char[msg1.length() + 1];
				strcpy(message, msg1.c_str());
				for(std::vector<active_user>::iterator it = users.begin();it != users.end();it++){
					if(userName != it->user){
						if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&(it->addr), addr_len)) == -1){
							perror("\tUDP_Server: joined message to active users error");
							exit(1); 
							}
						myfile << "Sent ack for " << userName << " to " << it->user << "\n";
						}
					}
				myfile.flush();
				myfile.close();			
				break;
				}

			case 2:{
				// send back ack for msg 2
				myfile.open("log.txt",std::fstream::app);
				t = time(NULL);
				charTime = ctime(&t);			
				myfile << "\nRecieved (" << numbytes << " bytes) " << "msg " << msg_num << " " << decrypted << " from " << s << " at " << charTime;
				msg1 = "ack;" + std::to_string(msg_num) + ";";	
				message = new char[msg1.length() + 1];
				strcpy(message, msg1.c_str());	
				if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len))==-1){
					perror("\tUDP_Server: message1 sendto error");
					exit(1);
					}

				buddyName = getBuddyName(decrypted);
				buddyMsg = getMessage(decrypted);	
				//std::cout<< "message is " << buddyMsg << " \n";
				myfile << "Message is: " << buddyMsg;
				myfile << "Sent message to: " << buddyName << " from: " << userName << "\n";

				msg1 = "Message;2;";
				msg_to_encrypt = "\nFrom: " + userName + " To: " + buddyName + "\n" + buddyMsg + "\n";
				msgEncrypted = encryptMessage(msg_to_encrypt);
				msg1 += msgEncrypted;
				message = new char[msg1.length() +1];
				strcpy(message, msg1.c_str());
				std::vector<active_user>::iterator it = users.begin();
				while(it->user != buddyName && it != users.end())
					it++;
				if(it != users.end()){
					if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&(it->addr), addr_len)) == -1){
						perror("\tUDP_Server: message to buddy error");
						exit(1);
						}
					 }
				myfile.close();	
				break;					
				}
			case 3:{
				myfile.open("log.txt", std::fstream::app);
				t = time(NULL);
				charTime = ctime(&t);
				myfile << "\nRecieved (" << numbytes << " bytes) " << "msg " << msg_num << " " << decrypted << " from " << s << " at " << charTime;
				msg1 = "ack;"+ std::to_string(msg_num) + ";";
				msg_to_encrypt = "Goodbye, " + userName;
				msgEncrypted = encryptMessage(msg_to_encrypt);
				msg1 = msg1 + msgEncrypted;
				message = new char[msg1.length()+1];
				strcpy(message, msg1.c_str());
				if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
					perror("\tUDP_Server: message3 sendto error");
					exit(1);
					}
						
				// Loop to take user out of list of active users
				for(int i = 0; i <  (int)users.size();i++){
					if(users[i].user == userName){
						users.erase(users.begin()+i);
						break;
						}
					}

				msg1 = "Users;3;";
				msg_to_encrypt = userName + " is leaving the group";
				msgEncrypted = encryptMessage(msg_to_encrypt);
				msg1 = msg1 + msgEncrypted;
				message = new char[msg1.length()+1];
				strcpy(message, msg1.c_str());
				if(!users.empty()){
					for(std::vector<active_user>::iterator it = users.begin();it != users.end();it++){
						if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&(it->addr), addr_len)) == -1){
						perror("\tUDP_Server: message3 sendto error");
						exit(1);
							}					
						myfile << "Sent Goodbye for " << userName << " to " << it->user << "\n";
						}
					}
				myfile.close();
				break;
				}
			default:
				printf(" ");			
			}
		delete[] message;	
		}
	close(sockfd);
	return 0;
}


		
