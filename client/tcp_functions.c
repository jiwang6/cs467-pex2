//
// Created by Parks Masters; TEMPLATE edited by David Merritt.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>

#define FIN 1
#define SYN 2
#define ACK 16
#define MAXLINE 1024


struct tcp_info{
    int my_seq; //INITIAL sequence number of my connection.  Added to 'data_sent' returns the next sequence number i should use.
    int remote_seq; //INITIAL sequence number of remote side.  Added to 'data_received' returns next expected sequence number.
    int data_sent;  //used to determine SEQ #
    int data_received; //used to determine what MY ack should be
    int remote_data_acknowledged; // what the other side has received based on ACK #
};


int ParseTCPHeader(char *buffer, struct tcp_info *connection_info); //prototype for ParseTCP

int TCPSend(int sockfd, char* appdata, int appdata_length, struct sockaddr_in * addr, struct tcp_info *connection_info);

int BuildPacketHeader(char *buffer, struct tcp_info *connection_info, int flags){
    // Put FLAGS, SEQ, ACK, and APPDATA strings into a buffer

    sprintf(buffer, "FLAGS\n%d\nSEQ\n%d\nACK\n%d\nAPPDATA\n%s",flags, connection_info->my_seq, connection_info->remote_seq, "LIST_REQUEST"); // change LIST_REQUEST to specific request
    // Update the connection_info struct's data_sent and data_received fields
    return strlen(buffer);
}

// Called by TCPReceive (see below). This calls ParseTCPHeader (see below).
int TCPReceivePacket(int sockfd, char *appdata, int appdata_length, struct sockaddr_in *addr,
                     struct tcp_info *connection_info){
    //appdata is the receive buffer
    int n, len = sizeof(*addr);
    
    // call recvfrom, check for error
    if((n = recvfrom(sockfd, appdata, appdata_length, 0, ( struct sockaddr *) addr, &len)) < 0)
    {
        //print appropriate error message, then exit
        exit(EXIT_FAILURE);
    }
    appdata[n]= '\0';
    int header_length = ParseTCPHeader(appdata, connection_info); //call ParseTCPHeader, and save output in header_length
    int appdata_received=0;
    
    //check header_length to determine if there was an error; if so, ignore packet according to Receiving Rule #2 from PEX2 CS467 instructions
    if(header_length == -1){
        //print error message
        return -2;
    }

    //check header_length to determine if wrong SEQ number received; if so, look at Receiving Rule #3 from PEX2 CS467 instructions
    else if(header_length == -2){
        //immediately re-send ACK packet
        return -2;
    }

    //check header_length to determine if wrong ACK number received; if so, ignore packet according to Receiving Rule #2 from PEX2 CS467 instructions
    else if(header_length==-3){
        return -2;
    }
    
    //if we made it to here, then we successfully received good appdata and must track the additional bytes received in data_received
    else{ 
        //calculate how many data bytes were received (don't include the header)
        //add those number of bytes to data_received
    }

    //if data was received, send an ACK immediately according to Receiving Rule #1 from PEX2 CS467 instructions

    return (appdata_received);
}

// TCPReceive calls TCPReceivePacket to receive one packet and to track how many attempts the client will make to receive one packet
int TCPReceive(int sockfd, char *buffer, int appdata_length, struct sockaddr_in *addr,
               struct tcp_info *connection_info){
    //initialize # of attempts and appdata_received
    //loop to call TCPReceivePacket()
    //  loop until you've reached a certain number of attempts (how many attempts are you supposed to try?)
    //  appdata_received saves the receive buffer returned from TCPReceivePacket()
    //  return appdata_received after successful TCPReceivePacket() execution
    return -1;
}

//parse out Flags, Seq#, and Ack#.  Validate they are correct.  Update Connection Info
int ParseTCPHeader(char *buffer, struct tcp_info *connection_info){
    typedef unsigned long ulong;
    ulong header_length = (strstr(buffer, "APPDATA\n")-buffer)+strlen("APPDATA\n"); //header_length is the send/receive buffer minus 
                                                                                    //  the TCP header strings and APPDATA string
    char* splitstring = strtok(buffer, "\n");
    int SYNbit=0;
    int ACKbit;
    int SEQnum;
    int ACKnum;

    //parse splitstring looking for FLAGS. Look for SYN bit and ACK bit, save to SYNbit and ACKbit accordingly
    if(strcmp(splitstring, "FLAGS")==0){
    }
    else{ //print appropriate error message, return -1
        return -1; //invalid packet
    }

    //parse splitstring looking for SEQ. Set SEQnum accordingly
    splitstring = strtok(NULL, "\n");
    if(strcmp(splitstring, "SEQ")==0){
    }
    else{ //print appropriate error message, return -1
        return -1; //invalid packet
    }

    //parse splitstring looking for ACK. Set ACKnum accordingly
    splitstring = strtok(NULL, "\n");
    if(strcmp(splitstring, "ACK")==0){
    }
    else{ //print appropriate error message, return -1
        return -1; //invalid packet
    }

    //parse splitstring looking for the presence of APPDATA string. If not found, return error.
    splitstring = strtok(NULL, "\n");
    if(strcmp(splitstring, "APPDATA")!=0){ //print appropriate error message, return -1
        return -1; //invalid packet
    }

    //All headers were found, update packet info based on values
    if(SYNbit){
        //update connection_info remote_seq and data_received fields
    }
    //else if {the sequence number you received does not match what you were expecting}, print error message and return -2

    if(ACKbit){
        //if ACKnum is not what it should be (based on my_seq and data_sent), then print error message and return -3
        //else if there is data I've sent that has not been acknowledged, then update remote_data_acknowledged with this ACKnum
    }

    return header_length; //return the number of bytes of header
}


// this function implements Sending Rule #3 from PEX2 CS467 instructions
int WaitForACK(int sockfd, char * packet_sent, int packet_length, struct sockaddr_in * addr, struct tcp_info* connection_info, int num_attempts){
    
    /* this implements Sending Rule #3.a from PEX2 CS467 instructions
    char * buffer = malloc(sizeof(char)*MAXLINE);
    struct timeval timeout;
    timeout.tv_sec = 1; //sets timeout in seconds
    timeout.tv_usec = 0; //0 milliseconds
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof                                                                                                                                                                                                                                                                                                                               (timeout)) < 0){
        perror("setsockopt failed");
        return -1;
    }
    */

    //implement Sending Rule 3.b. and 3.c from PEX2 CS467 instructions

    return 0;
}

int TCPSend(int sockfd, char* appdata, int appdata_length, struct sockaddr_in * addr, struct tcp_info *connection_info, int flags){
    //new buffer is required to add TCP header
    char* buffer = malloc(sizeof(char)*MAXLINE);
    // int flags = 0; //actually, set this to some integer representing which flags you want to set for this packet
    //build a buffer containing the TCP header
    int header_length = BuildPacketHeader(buffer, connection_info, flags);

    //send packet with sendto()

    //if sendto() was successful, then update data_sent with how many bytes were just sent
    
    //immediately get ACK by calling WaitForAck function (see above) according to Sending Rule #3 from PEX2 CS467 instructions

    return 0;
}

struct tcp_info* TCPConnect(int sockfd,  struct sockaddr_in * servaddr){
    //instantiate new connection_info struct
    struct tcp_info * connection_info = malloc(sizeof(struct tcp_info));

    //initialize my_seq, remote_seq, data_sent, data_received, remote_data_acknowledged

    // connection_info->my_seq += connection_info->data_sent;
    // connection_info->remote_seq += connection_info->data_received;

    connection_info->my_seq = rand() % (10000 + 1 - 1) + 1; // don't worry about it
    
    connection_info->remote_seq = 0; // server's sequence number

    //do the 3-way handshake using TCPSend, sendto, TCPReceivePacket, recvfrom, or any other combo of sending/receving

    TCPSend();
    TCPReceivePacket();

    printf("Connected!\n");
    return connection_info;
}