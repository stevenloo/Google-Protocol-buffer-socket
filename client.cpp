#include <unistd.h>
#include "http_packet.pb.h"
#include <iostream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "http_session_format_info.h"
using namespace google::protobuf::io;

using namespace std;

int protobuf_send(http_session_format_info* http_info)
{

	/* Coded output stram */

	http_packet a_http;
	a_http.set_url(http_info->url);
	a_http.set_cookie(http_info->cookie);
	a_http.set_src_ip(http_info->src_ip);
	a_http.set_dst_ip(http_info->dst_ip);
	a_http.set_cont_type(http_info->content_type);
	a_http.set_cont_length(http_info->content_length);
	a_http.set_location(http_info->location);
	a_http.set_referer(http_info->referer);
	a_http.set_pid(http_info->pid);
	cout<<"size after serilizing is "<<a_http.ByteSize()<<endl;
	int siz = a_http.ByteSize()+4;
	char *pkt = new char [siz];
	google::protobuf::io::ArrayOutputStream aos(pkt,siz);
	CodedOutputStream *coded_output = new CodedOutputStream(&aos);
	coded_output->WriteVarint32(a_http.ByteSize());
	a_http.SerializeToCodedStream(coded_output);

        int host_port= 1101;
        char* host_name="10.0.6.227";

        struct sockaddr_in my_addr;

        int bytecount;

        int hsock;
        int * p_int;
        int err;

        hsock = socket(AF_INET, SOCK_STREAM, 0);
        if(hsock == -1)
	{
                printf("Error initializing socket %d\n",errno);
                close(hsock);
		return 0;
        }
	//setting socket;
        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;
        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
                (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) )
	{
                printf("Error setting options %d\n",errno);
                free(p_int);
                close(hsock);
		return 0 ;
        }
        free(p_int);
	//setting ok;
        my_addr.sin_family = AF_INET ;
        my_addr.sin_port = htons(host_port);

        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(host_name);
        if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 )
	{
                if((err = errno) != EINPROGRESS)
		{
                        fprintf(stderr, "Error connecting socket %d\n", errno);
                        close(hsock);
			return 0;
                }
        }
	//send ();
        if( (bytecount=send(hsock, (void *) pkt,siz,0))== -1 ) 
	{
                fprintf(stderr, "Error sending data %d\n", errno);
        }
	else
	{
        	printf("Sent bytes %d\n", bytecount);
	}

	close(hsock);
        usleep(1);
        delete pkt;
	return 0;
}
