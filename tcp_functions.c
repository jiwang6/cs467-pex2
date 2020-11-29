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
#include <sys/time.h>
#include <sys/ioctl.h>
#include "tcp_functions.h"

#define FIN 1
#define SYN 2
#define ACK 16
#define MAXLINE 1024

int ParseTCPHeader(char *buffer, struct tcp_info *connection_info);

int BuildPacketHeader(char *buffer, struct tcp_info *connection_info, int flags, char *appdata) {
    // Put FLAGS, SEQ, ACK, and APPDATA strings into a buffer
    // Update the connection_info struct's data_sent and data_received fields
    if (strcmp(appdata, "ACK") == 0) {
        sprintf(buffer, "FLAGS\n%d\nSEQ\n%d\nACK\n%d\nAPPDATA\n", flags, connection_info->my_seq + connection_info->data_sent, connection_info->remote_seq + connection_info->data_received);
    } else if (strcmp(appdata, "SYN") == 0) {
        sprintf(buffer, "FLAGS\n%d\nSEQ\n%d\nACK\n%d\nAPPDATA\n", flags, connection_info->my_seq + connection_info->data_sent, connection_info->remote_seq + connection_info->data_received);
        connection_info->data_sent += 1;
    } else {
        // connection_info->data_sent += strlen(appdata);
        sprintf(buffer, "FLAGS\n%d\nSEQ\n%d\nACK\n%d\nAPPDATA\n%s", flags, connection_info->my_seq + connection_info->data_sent, connection_info->remote_seq + connection_info->data_received, appdata);
        // connection_info->data_sent += strlen(appdata);
    }


    // printf("%s\n\n",buffer); //FIXME - here for debugging !!!!!!!!!!!!!!!!

    return strlen(buffer);
}

// Called by TCPReceive (see below). This calls ParseTCPHeader (see below).
int TCPReceivePacket(int sockfd, char *appdata, int appdata_length, struct sockaddr_in *addr,
    struct tcp_info *connection_info) {
    //appdata is the receive buffer
    int n, len = sizeof( *addr);

    // call recvfrom, check for error
    if ((n = recvfrom(sockfd, appdata, appdata_length, 0, (struct sockaddr *) addr, &len)) < 0) {
        //print appropriate error message, then exit
        perror("ERROR receiving response from server\n");
        exit(EXIT_FAILURE);
    }
    appdata[n] = '\0';
    int header_length = ParseTCPHeader(appdata, connection_info); //call ParseTCPHeader, and save output in header_length
    int appdata_received = 0;

    //check header_length to determine if there was an error; if so, ignore packet according to Receiving Rule #2 from PEX2 CS467 instructions
    if (header_length == -1) {
        //print error message
        perror("ERROR malformed packet\n");
        return -2;
    }

    //check header_length to determine if wrong SEQ number received; if so, look at Receiving Rule #3 from PEX2 CS467 instructions
    else if (header_length == -2) {
        //immediately re-send ACK packet
        TCPSend(sockfd, "ACK", 1, addr, connection_info);
        return -2;
    }

    //check header_length to determine if wrong ACK number received; if so, ignore packet according to Receiving Rule #2 from PEX2 CS467 instructions
    else if (header_length == -3) {
        perror("ERROR incorrect ACK number\n");
        return -2;
    }

    //if we made it to here, then we successfully received good appdata and must track the additional bytes received in data_received
    else {
        //calculate how many data bytes were received (don't include the header)
        //add those number of bytes to data_received
        appdata_received = n - header_length;
        connection_info->data_received += appdata_received;
    }

    if(appdata_received > 0){
        //if data was received, send an ACK immediately according to Receiving Rule #1 from PEX2 CS467 instructions
        char *buffer = malloc(sizeof(char) *MAXLINE);
        int bufSize = BuildPacketHeader(buffer, connection_info, ACK, "ACK");
        sendto(sockfd, buffer, bufSize, 0, (const struct sockaddr *) addr, sizeof( *addr));
    }

    // return (appdata_received);
    return header_length;
}

// TCPReceive calls TCPReceivePacket to receive one packet and to track how many attempts the client will make to receive one packet
int TCPReceive(int sockfd, char *buffer, int appdata_length, struct sockaddr_in *addr,
    struct tcp_info *connection_info) {
    //initialize # of attempts and header_length
    int numAttempts = 1;
    int header_length = -1;

    //loop to call TCPReceivePacket()
    //  loop until you've reached a certain number of attempts (how many attempts are you supposed to try?)
    //  appdata_received saves the receive buffer returned from TCPReceivePacket()
    //  return appdata_received after successful TCPReceivePacket() execution
    while (numAttempts < 4 && header_length < 0) { // 3 attempts
        header_length = TCPReceivePacket(sockfd, buffer, appdata_length, addr, connection_info);
        numAttempts++;
    }
    if (numAttempts > 4) {
        return -1;
    }

    return header_length;
}

//parse out Flags, Seq#, and Ack#.  Validate they are correct.  Update Connection Info
int ParseTCPHeader(char *buffer, struct tcp_info *connection_info) {
    typedef unsigned long ulong;
    ulong header_length = (strstr(buffer, "APPDATA\n") - buffer) + strlen("APPDATA\n"); //header_length is the send/receive buffer minus 
    //  the TCP header strings and APPDATA string
    char *splitstring = strtok(buffer, "\n");
    int SYNbit = 0;
    int ACKbit;
    int SEQnum;
    int ACKnum;

    //parse splitstring looking for FLAGS. Look for SYN bit and ACK bit, save to SYNbit and ACKbit accordingly
    if (strcmp(splitstring, "FLAGS") == 0) {
        splitstring = strtok(NULL, "\n");
        if (atoi(splitstring) == 2) {
            SYNbit = 1;
        }
        if (atoi(splitstring) == 16) {
            ACKbit = 1;
        }
        if (atoi(splitstring) == 18) {
            SYNbit = 1;
            ACKbit = 1;
        }
    } else { //print appropriate error message, return -1
        perror("ERROR no FLAGS header in packet\n");
        return -1; //invalid packet
    }

    //parse splitstring looking for SEQ. Set SEQnum accordingly
    splitstring = strtok(NULL, "\n");
    if (strcmp(splitstring, "SEQ") == 0) {
        splitstring = strtok(NULL, "\n");
        SEQnum = atoi(splitstring);
    } else { //print appropriate error message, return -1
        perror("ERROR no SEQ header in packet\n");
        return -1; //invalid packet
    }

    //parse splitstring looking for ACK. Set ACKnum accordingly
    splitstring = strtok(NULL, "\n");
    if (strcmp(splitstring, "ACK") == 0) {
        splitstring = strtok(NULL, "\n");
        ACKnum = atoi(splitstring);
    } else { //print appropriate error message, return -1
        perror("ERROR no ACK header in packet\n");
        return -1; //invalid packet
    }

    //parse splitstring looking for the presence of APPDATA string. If not found, return error.
    splitstring = strtok(NULL, "\n");
    if (strcmp(splitstring, "APPDATA") != 0) { //print appropriate error message, return -1
        perror("ERROR no APPDATA header in packet\n");
        return -1; //invalid packet
    }

    //All headers were found, update packet info based on values
    if (SYNbit) {
    //     //update connection_info remote_seq and data_received fields
        connection_info->remote_seq = SEQnum;
    //     connection_info->data_received = 1; //FIXME
    } else if (SEQnum != (connection_info->remote_seq + connection_info->data_received)) {
    //     //else if {the sequence number you received does not match what you were expecting}, print error message and return -2
        perror("ERROR improper sequence number received\n");
        return -2;
    }

    if (ACKbit) {
        // //if ACKnum is not what it should be (based on my_seq and data_sent), then print error message and return -3
        if (ACKnum != (connection_info->my_seq + connection_info->data_sent)) {
            printf("ERROR ACK number does not match expected value\n");
            return -3;
        } else if (ACKnum != connection_info->remote_data_acknowledged) {
            //else if there is data I've sent that has not been acknowledged, then update remote_data_acknowledged with this ACKnum
            connection_info->remote_data_acknowledged = ACKnum;
        }
    }

    return header_length; //return the number of bytes of header
}

// this function implements Sending Rule #3 from PEX2 CS467 instructions
int WaitForACK(int sockfd, char *packet_sent, int packet_length, struct sockaddr_in *addr,
    struct tcp_info *connection_info, int num_attempts) {

    // this implements Sending Rule #3.a from PEX2 CS467 instructions
    char *buffer = malloc(sizeof(char) *MAXLINE);
    struct timeval timeout;
    timeout.tv_sec = 1; //sets timeout in seconds
    timeout.tv_usec = 0; //0 milliseconds
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        return -1;
    }

    //implement Sending Rule 3.b. and 3.c from PEX2 CS467 instructions
    if (num_attempts > 3) {
        perror("ERROR receiving 3 ACKs from server\n");
        exit(EXIT_FAILURE);
    }

    int n, len = sizeof( *addr);

    if ((n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *) addr, &len)) < 0) {
        //Resend packet
        sendto(sockfd, packet_sent, packet_length, 0, (const struct sockaddr *) addr, sizeof( *addr));
        WaitForACK(sockfd, packet_sent, packet_length, addr, connection_info, num_attempts++);
    }

    int header_length = ParseTCPHeader(buffer, connection_info); //call ParseTCPHeader, and save output in header_length
    //check header_length to determine if wrong ACK number received; if so, resend packet
    if (header_length == -3) {
        sendto(sockfd, packet_sent, packet_length, 0, (const struct sockaddr *) addr, sizeof( *addr));
        WaitForACK(sockfd, packet_sent, packet_length, addr, connection_info, num_attempts++);
    }

    return 0;
}

int TCPSend(int sockfd, char *appdata, int appdata_length, struct sockaddr_in *addr, struct tcp_info *connection_info) {
    //new buffer is required to add TCP header
    char *buffer = malloc(sizeof(char) *MAXLINE);
    int flags = ACK; // initialize to ACK
    if (strcmp(appdata, "SYN") == 0) {
        flags = SYN;
    } else if (strcmp(appdata, "FIN") == 0) {
        flags = FIN;
    }
    //build a buffer containing the TCP header
    int bufSize = BuildPacketHeader(buffer, connection_info, flags, appdata);
    //send packet with sendto()
    int bSent;
    //if sendto() was successful, then update data_sent with how many bytes were just sent
    if ((bSent = sendto(sockfd, buffer, bufSize, 0, (const struct sockaddr *) addr, sizeof( *addr))) != -1) {
        connection_info->data_sent += appdata_length;
        // connection_info->data_sent = connection_info->my_seq + appdata_length; // FIXME - is this bSent or appdata_length or header_length - can't decide  
    }
    
    // bSent = sendto(sockfd, buffer, bufSize, 0, (const struct sockaddr *) addr, sizeof( *addr));

    //immediately get ACK by calling WaitForAck function (see above) according to Sending Rule #3 from PEX2 CS467 instructions
    WaitForACK(sockfd, buffer, bufSize, addr, connection_info, 1);

    return 0;
}

struct tcp_info *TCPConnect(int sockfd, struct sockaddr_in *servaddr) {
    //instantiate new connection_info struct
    struct tcp_info *connection_info = malloc(sizeof(struct tcp_info));;

    //initialize my_seq, remote_seq, data_sent, data_received, remote_data_acknowledged
    connection_info->my_seq = rand() % (10000 + 1 - 1) + 1; // random intial sequence number
    connection_info->remote_seq = 0; // server's sequence number is currently unknown
    connection_info->data_sent = 0;
    connection_info->data_received = 0;
    connection_info->remote_data_acknowledged = 0;

    //do the 3-way handshake using TCPSend, sendto, TCPReceivePacket, recvfrom, or any other combo of sending/receving

    char *buffer = malloc(sizeof(char) *MAXLINE);
    int len = sizeof( *servaddr);

    int bufSize = BuildPacketHeader(buffer, connection_info, SYN, "SYN");
    sendto(sockfd, buffer, bufSize, 0, (const struct sockaddr *) servaddr, len);

    recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *) servaddr, &len);

    ParseTCPHeader(buffer, connection_info);
    connection_info->data_received += 1;
    
    bufSize = BuildPacketHeader(buffer, connection_info, ACK, "ACK");
    sendto(sockfd, buffer, bufSize, 0, (const struct sockaddr *) servaddr, len);

    printf("Connected!\n");
    return connection_info;
}