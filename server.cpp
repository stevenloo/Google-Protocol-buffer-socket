//author:LUO Pan 
//date:2015/3/27
//version:0

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "http_packet.pb.h"
#include <iostream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "server.h"

using namespace std;
using namespace google::protobuf::io;


void* SocketHandler(void*);
void* recv_thread(void*);
int main(int argv, char** argc){

        int host_port= 1101;

        struct sockaddr_in my_addr;

        int hsock;

        socklen_t addr_size = 0;
        
        pthread_t thread_id=0;
	sockaddr_in  sadr;
        hsock = socket(AF_INET, SOCK_STREAM, 0);
        if(hsock == -1)
	{
                printf("Error initializing socket %d\n", errno);
		return 0;
        }

	//set socket;
	int*  p_int;
        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;
        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
                (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
                printf("Error setting options %d\n", errno);
                free(p_int);
		return 0;
        }
        free(p_int);
	//setting ok;
	
        my_addr.sin_family = AF_INET ;
        my_addr.sin_port = htons(host_port);

        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = INADDR_ANY ;

        if( bind(hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 )
	{
                fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
                return 0;
        }
        if(listen(hsock, 10) == -1 )
	{
                fprintf(stderr, "Error listening %d\n",errno);
                return 0;
        }
	acsock_argv arg;
	acsock_argv* p_arg=&arg;
	p_arg->hsock=hsock;
	p_arg->sadr=sadr;
	p_arg->addr_size=addr_size;
        //Now lets do the server stuff
        addr_size = sizeof(sockaddr_in);
	pthread_create(&thread_id,0,&recv_thread,(void*)p_arg);
	pthread_join(thread_id,NULL);
	/*
 	while(true)
	{
                printf("waiting for a connection\n");
                csock = (int*)malloc(sizeof(int));
                if((*csock = accept(hsock, (sockaddr*)&sadr, &addr_size))!= -1)
		{
                        printf("---------------------\nReceived connection from %s\n",inet_ntoa(sadr.sin_addr));
                        pthread_create(&thread_id,0,&SocketHandler, (void*)csock );
                        pthread_detach(thread_id);
                }
                else{
                        fprintf(stderr, "Error accepting %d\n", errno);
                }
	}        
	*/
	return 1 ;
       	//oops

}

void* recv_thread(void* p_argv)
{
 	acsock_argv* p_arg=(acsock_argv*) (p_argv);
    int hsock=p_arg->hsock;
	sockaddr_in  sadr=p_arg->sadr;
	int* csock;
	socklen_t addr_size=p_arg->addr_size;
	pthread_t thread_id=0;

	while(true)
	{
        printf("waiting for a connection\n");
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept(hsock, (sockaddr*)&sadr, &addr_size))!= -1)
		{
				printf("---------------------\nReceived connection from %s\n",inet_ntoa(sadr.sin_addr));
				pthread_create(&thread_id,0,&SocketHandler, (void*)csock );
				pthread_detach(thread_id);
        }
                else
				{
                        fprintf(stderr, "Error accepting %d\n", errno);
                }
    }
	return NULL;
}

google::protobuf::uint32 readHdr(char *buf)
{
  google::protobuf::uint32 size;
  google::protobuf::io::ArrayInputStream ais(buf,4);
  CodedInputStream coded_input(&ais);
  coded_input.ReadVarint32(&size);//Decode the HDR and get the size
  cout<<"size of payload is "<<size<<endl;
  return size;
}

void readBody(int csock,google::protobuf::uint32 siz)
{
    int bytecount;
    http_packet a_http;
    char buffer [siz+4];//size of the payload and hdr
    //Read the entire buffer including the hdr
    if((bytecount = recv(csock, (void *)buffer, 4+siz, MSG_WAITALL))== -1)
    {
	fprintf(stderr, "Error receiving data %d\n", errno);
    }
    cout<<"Second read byte count is "<<bytecount<<endl;
    //Assign ArrayInputStream with enough memory 
    google::protobuf::io::ArrayInputStream ais(buffer,siz+4);
    CodedInputStream coded_input(&ais);
    //Read an unsigned integer with Varint encoding, truncating to 32 bits.
    coded_input.ReadVarint32(&siz);
    //After the message's length is read, PushLimit() is used to prevent the CodedInputStream 
    //from reading beyond that length.Limits are used when parsing length-delimited 
    //embedded messages
    google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(siz);
    //De-Serialize
    a_http.ParseFromCodedStream(&coded_input);
    //Once the embedded message has been parsed, PopLimit() is called to undo the limit
    coded_input.PopLimit(msgLimit);
    //Print the message
    cout<<"Message is "<<a_http.DebugString();
    //cout<< "url"<<a_http.;
}

void* SocketHandler(void* lp)
{
    int *csock = (int*)lp;

        char buffer[4];
        int bytecount=0;
        memset(buffer, '\0', 4);

        while (1) 
		{
        	//Peek into the socket and get the packet size
       		if((bytecount = recv(*csock, buffer,4, MSG_PEEK))== -1)
				{
                	fprintf(stderr, "Error receiving data %d\n", errno);
				}
		else if (bytecount == 0)
                	break;
				
        	cout<<"First read byte count is "<<bytecount<<endl;
        	readBody(*csock,readHdr(buffer));
		}

    return 0;
}
