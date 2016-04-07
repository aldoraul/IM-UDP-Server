/* Functions.cpp
   contain function calls used
   in main.cpp
*/

#include<stdio.h>
//#include<netinet/in.h>
#include"functions.h"

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

std::string decryptMessage(std::string buf){	
	int check_msg = 0;
	int length = buf.length();
	int next = 0;
	for (int i = 0; i < length; i++) {
		if (buf[i] == ';')
			check_msg += 1;
		if (check_msg == 2) {
			next = i + 1;
			break;
		}
	}
	for (int j = next; j < length; j++)
		buf[j] = decrypt(buf[j]);
	return buf;
}

std::string get_user(std::string message){
	int check_msg = 0;
	int length = message.length();
	int next = 0;
	std::string name = "";
	for(int i = 0; i < length; i++) {
		if(message[i] == ';')
			check_msg +=1;
		if(check_msg == 2) {
			next = i + 1;
			break;
		}
	}
	return message.substr(next, -1);
}
