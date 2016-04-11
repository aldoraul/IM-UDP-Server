/* CS 423 Project 2
 UDP_SERVER
 Aldo Anaya
*/

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

// creates a struct of active_user.  each time a new client signs on
//	it is stored in a vector<active_users>
struct active_user{
        std::string user;
        struct sockaddr_storage addr;

        active_user(std::string user1, struct sockaddr_storage addr1):
         user(user1), addr(addr1) {}

};


int main(void){
	
	time_t t; 		// used to get current time and date for log.txt
	char* charTime;
	std::ofstream myfile;
	char s[INET6_ADDRSTRLEN];	// used to store client address in readable form
	std::string ERROR1 = "Message type is invalid";		//Error mesages to be sent to client
	std::string ERROR2 = "We got message request, but buddy isn't logged in";
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int numbytes;
	int rv;
	struct sockaddr_storage their_addr;
	struct active_user user = active_user("", their_addr);
	std::vector<active_user> users;		// stores all active clients
	char buf[MAXBUFFLEN];		// store received message
	socklen_t addr_len;
	int yes = 1;
	
	memset(&hints, 0, sizeof hints);
	printf("memset cleared\n");

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	
	// server side info
	if((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop until connection is made
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
	// free information, no longer needed, socket already created
	freeaddrinfo(servinfo);
	
	while(1){
		

		addr_len = sizeof their_addr;
		if((numbytes = recvfrom(sockfd, buf, MAXBUFFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len))== -1){
			perror("\trecvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';						// all functions defined in functions.cpp
		std::string decrypted = decryptMessage(buf);  // decrypts received msg after 2nd semi colon. returns entire msg
		int msg_type = getMsgType(decrypted);	// returns msg type number. 1-initial msg, 2.-sendto buddy, 3. sign-off
		int msg_num = getMsgNum(decrypted);	// returns msg num created by client.  reads 1st 5 chars from msg unless stay alive reply
		std::string userName = get_user(decrypted);	// returns user name of client who sent message	

		printf("user name is %s", userName);	// prints user name, msg type and msg num of each incoming message
		printf("msg type %d \n", msg_type);
		printf("msg num  %d \n", msg_num);

		char *message;		// used in sendto functions to deliver string message to client
		std::string msg1 = "";	// used to construt initial un-encrypted part of message
		std::string msg_to_encrypt = "";	// all info of string to be encrypted
		std::string msgEncrypted = "";		// info from msg_to_encrypt in encrypted form
		std::string buddyName = "";		// extracts user name of who the client wants to send a  message to
		std::string buddyMsg = "";		// extracts body of msg to be sent to other client
 		
			// switch statment based on which msg type is recevied
		switch(msg_type){
			case 1:{		// initial sign on msg
				
				
				myfile.open("log.txt", std::fstream::aos);	// open file to write to.  use aos to append not open new file
				t = time(NULL);		// get current date and time
				charTime = ctime(&t);	// convert time to char		
				inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s); // get client ip in readable form s
				myfile << "Received msg " << msg_num << ": " << decrypted << " from " << s << " at "<< charTime << "\n";	
				myfile.close();				// write info and close file of log
				


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
						}
					}			
				break;
				}

			case 2:{	// msg to send to buddy
				// send back ack for msg type 2
				msg1 = "ack;" + std::to_string(msg_num) + ";";	
				message = new char[msg1.length() + 1];
				strcpy(message, msg1.c_str());	
				if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len))==-1){
					perror("\tUDP_Server: message1 sendto error");
					exit(1);
					}

				buddyName = getBuddyName(decrypted);	// if msg type 2, extract buddy name
				buddyMsg = getMessage(decrypted);	// if msg type 2, extract buddy msg
				
				std::vector<active_user>::iterator it = users.begin();
				while(it->user != buddyName && it != users.end())	// search for buddy name in active user list
					it++;
				if(it != users.end()){	
					msg1 = "Message;2;";	// if buddy is found forward msg
					msg_to_encrypt = "\nFrom: " + userName + " To: " + buddyName + "\n" + buddyMsg + "\n";
					msgEncrypted = encryptMessage(msg_to_encrypt);
					msg1 += msgEncrypted;
					message = new char[msg1.length() +1];
					strcpy(message, msg1.c_str());
					if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&(it->addr), addr_len)) == -1){
						perror("\tUDP_Server: message to buddy error");
						exit(1);
						}
				}else{

					msg1 = "Error;" + std::to_string(msg_num) + ";";
					msgEncrypted = encryptMessage(ERROR2);			// if buddy is not found send error msg
					msg1 += msgEncrypted;
					message = new char[msg1.length() + 1];
					strcpy(message, msg1.c_str());
					if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
						perror("\tUDP_Server: sendto ERROR2");
						exit(1);
					
						}
					}
				break;					
				}
			case 3:{	// sign off msg
	
				msg1 = "ack;"+ std::to_string(msg_num) + ";";	// send ack for receving msg
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
			
				msg1 = "Users;3;";			// alert all other active users that the client has signed off
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
						}
					}

				break;
				}
			default:	// if msg type number is not [1,2,3] send back error
				
				msg1 = "Error;" + std::to_string(msg_num) + ";";
				msgEncrypted = encryptMessage(ERROR1);
				msg1 += msgEncrypted;
				message = new char[msg1.length()+1];
				strcpy(message, msg1.c_str());
				if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
					perror("\tUDP_Server: error sending error msg");
					exit(1);
					}			
			}
		delete[] message;	
		}

	
	close(sockfd);
	return 0;
}


		
