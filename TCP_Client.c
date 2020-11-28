//Code obtained and modified from https://www.geeksforgeeks.org/udp-server-client-implementation-c/
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
#include "tcp_functions.h"

#define PORT     4240
#define MAXLINE 1024


// Driver code
int main() {
    int sockfd; //Socket descriptor, like a file-handle
    char buffer[MAXLINE]; //buffer to store message from server
    char *listRequest = "LIST_REQUEST"; //message to send to server
    char streamRequest[100] = "START_STREAM\n";
    char songName[50];
    struct sockaddr_in     servaddr;  // we don't bind to a socket to send UDP traffic, so we only need to configure server address

    char kidsChoice[50]; // what the user wants, don't worry about it
    int totalFrames = 1;
    int totalBytes = 0;
    int errr = 0;
    char* listP;
    char* bufferP;
    FILE *fp;

    struct tcp_info* connection_info;



    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Filling server information
    servaddr.sin_family = AF_INET; //IPv4
    servaddr.sin_port = htons(PORT); // port, converted to network byte order (prevents little/big endian confusion between hosts)
    servaddr.sin_addr.s_addr = INADDR_ANY; //localhost
    
    int n, len = sizeof(servaddr);


    struct timeval timeout;      
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;   
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char * ) &timeout, sizeof(timeout)) < 0) {
                perror("setsockopt failed");
                exit(EXIT_FAILURE);
            }

    for (;;) {
        strcpy(kidsChoice, "\0");
        strcpy(buffer, "\0");
        errr = 0;
        totalFrames = 1;
        totalBytes = 0;

        connection_info = TCPConnect(sockfd, &servaddr); 

        printf("Enter one of the following commands:\n\"1\" = List Songs\n\"2\" = Stream a Song\n\"3\" = exit\n"); // prompt
        fgets(kidsChoice,10,stdin);
        strtok(kidsChoice, "\n");

        while ( (strcmp(kidsChoice, "1") != 0) && (strcmp(kidsChoice, "2") != 0) && (strcmp(kidsChoice, "3") != 0) ) { // reprompt
            printf("You did not enter a valid selection. Please enter the whole number 1, 2, or 3.\n");
            printf("Enter one of the following commands:\n\"1\" = List Songs\n\"2\" = Stream a Song\n\"3\" = exit\n");
            strcpy(kidsChoice, "\0");
            fgets(kidsChoice,10,stdin);
            strtok(kidsChoice, "\n");
        }

        if (strcmp(kidsChoice, "3") == 0) // exit program
            break;

        // Sending message to server


        if (strcmp(kidsChoice, "1") == 0) { // list request to be replaced
            printf("Requesting a list of songs\n");
            TCPSend(sockfd, listRequest, strlen(listRequest), &servaddr, connection_info);
            // sendto(sockfd, (const char *)listRequest, strlen(listRequest), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

            if(( n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) {  // TODO: replace this
                perror("ERROR receiving response from server");
                printf("Errno: %d. ",errno);
            } else {
                buffer[n] = '\0'; //terminate message
                listP = buffer + 11; 
                printf("Songs Available:\n%s\n", listP);
            }
        }

        if (strcmp(kidsChoice, "2") == 0) { // song request to be replaced
            printf("Please enter a song name: ");
            
            fgets(songName,49,stdin);
            strtok(songName, "\n");
            strcat(streamRequest, songName);
            
            sendto(sockfd, (const char *)streamRequest, strlen(streamRequest), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr)); // TODO: replace this
            printf("Sending start stream\nWaiting for response\n");
            strcpy(streamRequest, "START_STREAM\n"); //reset

            // PRINT EACH FRAME IT RECEIVES            
            do {
                if(( n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) { // TODO: replace this
                    printf("Socket timed out.");
                    errr = 1;
                    break;
                }

                buffer[n] = '\0'; //terminate message

                if (!strcmp(buffer,"COMMAND_ERROR")) { // when requesting a song not listed
                    printf("Command error received from server. Cleaning up...\nDone!\n");
                    errr = 1;
                    break;
                } else if (totalFrames == 1) {
                    fp = fopen(songName, "w+");
                }

                if (errr != 1) {
                    bufferP = buffer;
                    fwrite(bufferP + 12 , 1 , n-12 , fp);
                }

                if (!strcmp(buffer, "STREAM_DONE"))
                    break;

                n -= 12;
                printf("Frame # %d Received with %d Bytes\n", totalFrames, n);
                totalBytes += n;
                ++totalFrames;
            } while (strcmp(buffer, "STREAM_DONE"));

            totalFrames -= 1;
            if(errr != 1) {
                printf("Stream done. Total Frames: %d Total Size : %d bytes\nDone!\n", totalFrames, totalBytes);
                fclose(fp);
            }
        } // end of stream funct


    }

    close(sockfd);
    return 0;
}