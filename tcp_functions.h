//
// Author: Capt G. Parks Masters
// Date: 6 Oct 2019
//
// Jim Wang and Zach Rotzal
// PEX2
// 30 November 2020
// Please reference TCP_Client.c for documentation statement

#ifndef TCP_MP3SERVER_TCP_FUNCTIONS_H
#define TCP_MP3SERVER_TCP_FUNCTIONS_H

// Structure is used to track the details of the current connection.
// This data is initialized during TCPConnect (3-way handshake)
// Must be provided to TCPSend/Receive so that subsequent packets have
//   the correct header information, and errors can be handled appropriately.
struct tcp_info{
    int my_seq;         //stores my INITIAL SEQ number established during TCPComnect.  This never changes during a connection
    int remote_seq;     //stores server's INITIAL SEQ number established during TCP Connect.  This never changes during a connection
    int data_sent;      //used to determine what my SEQ # and server's ACK # should be
    int data_received;  //used to track what my ACK# and server's SEQ should be
    int remote_data_acknowledged;  //number of bytes server has received.  Updated
};

// Function to conduct 3-way handshake with server.  Establishes initial
//   SEQ and ACK numbers.  Returns a tcp_info structure to the MP3Client
//   that can later be passed to TCPSend/Receive.
struct tcp_info* TCPConnect(int sockfd,  struct sockaddr_in * servaddr);

// Replaces all instances of "recvfrom" in your MP3Client.
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer where the application data in the packet
//           will be stored (just like recvfrom()).  TCP header info is stripped
//           from the packet before it is returned.
//  connection_info: structure that contains info about current connection
int TCPReceive(int sockfd, char *appdata, int appdata_length, struct sockaddr_in *addr,
                     struct tcp_info *connection_info);

// Replaces all instances of "sendto" in your MP3Client.
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer that contains the application data the
//           MP3Client is trying to send.  TCP header information is added to
//           the beginning of this data before it is sent.
//  connection_info: structure that contains info about current connection
int TCPSend(int sockfd, char* appdata, int appdata_length, struct sockaddr_in * addr, struct tcp_info *connection_info);

#endif //TCP_MP3SERVER_TCP_FUNCTIONS_H