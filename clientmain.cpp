#include <netinet/in.h>
#include <ostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
// #include <string>
#include <string>
#include <sys/_endian.h>
#include <sys/_types/_socklen_t.h>
#include <sys/_types/_ssize_t.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <unistd.h>


// Enable if you want debugging to be printed, see examble below.
// Alternative, pass CFLAGS=-DDEBUG to make, make CFLAGS=-DDEBUG
#define DEBUG


// Included to get the support library
#include "calcLib.h"

const int BUFFER_SIZE = 1024;

void error(const std::string& message) {
  std::cerr << "Error: " << message << std::endl;
  exit(1);
}

std::string receive_message(int socket) {
  char buffer[BUFFER_SIZE];
  std::string message = "";
  ssize_t bytesRead;
  while ((bytesRead = read(socket, buffer, BUFFER_SIZE)) > 0) {
    message += std::string(buffer, bytesRead);
    if (message.find('\n') != std::string::npos) {
      break;
    }
  }
  if (bytesRead == -1) {
    error("Failed to receive message");
  }
  return message;
}

void send_message(int socket, const std::string& message) {
  if (write(socket, message.c_str(), message.size()) == -1) {
    error("Failed to send message");
  }
}

int main(int argc, char *argv[]){

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter. 

  /* Do magic */
  int port=atoi(Destport);
#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    error("Failed to create socket");
  }

  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr(Desthost);
  serverAddr.sin_port = htons(port);

  if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
    error("Failed to connect to server");
  }

#ifdef DEBUG
  sockaddr_in cli_addr;
  socklen_t len = sizeof(cli_addr);
  if (getsockname(clientSocket, (struct sockaddr *)&cli_addr, &len) < 0) {
    error("Failed to get local socket name");
  }
  std::cout << "Connected to " << Desthost << ":" << port 
  << " local " << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << std::endl;
#endif

  std::string serverResponse = receive_message(clientSocket);
  std::cout << serverResponse << std::endl;

  send_message(clientSocket, "OK\n");

  std::string message = receive_message(clientSocket);
  std::istringstream iss(message);
  std::string operation, values1, values2;
  iss >> operation >> values1 >> values2;

  double fresult = 0.0;
  int iresult = 0;

  std::string result = "";
  
  if(operation[0]=='f'){
    double f1 = std::stod(values1);
    double f2 = std::stod(values2);
   
    if(std::strcmp(operation.c_str(),"fadd")==0){
      fresult=f1+f2;
    } else if (std::strcmp(operation.c_str(), "fsub")==0){
      fresult=f1-f2;
    } else if (std::strcmp(operation.c_str(), "fmul")==0){
      fresult=f1*f2;
    } else if (std::strcmp(operation.c_str(), "fdiv")==0){
      fresult=f1/f2;
    }
    printf("%s %8.8g %8.8g = %8.8g\n",operation.c_str(),f1,f2,fresult);
  } else {
    int i1 = std::stoi(values1);
    int i2 = std::stoi(values2);

    if(std::strcmp(operation.c_str(),"add")==0){
      iresult=i1+i2;
    } else if (std::strcmp(operation.c_str(), "sub")==0){
      iresult=i1-i2;
    } else if (std::strcmp(operation.c_str(), "mul")==0){
      iresult=i1*i2;
    } else if (std::strcmp(operation.c_str(), "div")==0){
      iresult=i1/i2;
    }

    printf("%s %d %d = %d \n",operation.c_str(),i1,i2,iresult);
  }

}
