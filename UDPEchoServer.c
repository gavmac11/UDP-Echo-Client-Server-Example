#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <signal.h>
#include "packet.c"

void timeoutAlarm(int number);
void DieWithMessage(char* message);

int main(int argc, char *argv[]) 
{
    unsigned int serverPort;
    int chunkSize;
    double loss_rate;
    struct sigaction action;
    char buffer[8193];
    bzero(buffer, 8192);
    int highestPacket = -1;


    if (argc < 3 || argc > 4)
    {
        DieWithMessage("Parameters error, <Port Number> <Chunk Size> <Loss Rate (Optional)> \n");
    }

    serverPort = atoi(argv[1]);
    chunkSize = atoi(argv[2]);

    if (argc == 4)
    {
        loss_rate = atof(argv[3]);
    }
    else
    {
        loss_rate = 0;
    }

    int sock; //socket descriptor
    // Create socket for incoming connections
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        DieWithMessage("socket() failed");
    }


    struct sockaddr_in localAddress;
    memset(&localAddress, 0, sizeof(localAddress));//zero out
    localAddress.sin_family = AF_INET;//internet addressing family
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);//any interface
    localAddress.sin_port = htons(serverPort);//port

    // Bind to the local address
    if (bind(sock, (struct sockaddr*) &localAddress, sizeof(localAddress)) < 0)
    {
        DieWithMessage("bind() failed");
    }

    //set timer for teardown
    action.sa_handler = timeoutAlarm;
    if (sigfillset(&action.sa_mask) < 0)
    {
        DieWithMessage("sigfillset failed");
    }

    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        DieWithMessage("SIGALRM failed");
    }

    for (;;) 
    { // Run forever
        struct sockaddr_in clntAddr; // Client address
        // Set Length of client address structure (in-out parameter)
        unsigned int clntAddrLen = sizeof(clntAddr);

        struct packet currentPacket;

        memset(&currentPacket, 0, sizeof(currentPacket));

        // Block until receive message from a client
        // Size of received message
        ssize_t numBytesRcvd = recvfrom(sock, &currentPacket, sizeof(currentPacket), 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
        currentPacket.type = ntohl(currentPacket.type);
        currentPacket.length = ntohl(currentPacket.length);
        currentPacket.seq_no = ntohl(currentPacket.seq_no);

        //teardown
        if (currentPacket.type == 4)
        {
            printf("%s\n", buffer);
            struct packet ack;
            ack.type = htonl(8);
            ack.seq_no = htonl(0);
            ack.length = htonl(0);

            if (sendto(sock, &ack, sizeof(ack), 0, (struct sockaddr *) &clntAddr, clntAddrLen) != sizeof(ack))
            { 
                DieWithMessage("error teardown ack");
            }

            alarm(7);

            while(1)
            {
                while(recvfrom(sock, &currentPacket, sizeof(int)*3 + chunkSize, 0, (struct sockaddr *) &clntAddr, &clntAddrLen) < 0)
                {
                    if (errno == EINTR)
                    {
                        exit(0);
                    }
                    else
                    {
                        //nada
                    }
                }


                if (ntohl(currentPacket.type == 4))
                {
                    ack.type = htonl(8);
                    ack.length = htonl(0);
                    ack.seq_no = htonl(0);;
                    //Send teardown ack
                    if (sendto(sock, &ack, sizeof(ack), 0, (struct sockaddr *) &clntAddr, clntAddrLen) != sizeof(ack))
                    {
                        DieWithMessage("error teardown ack");
                    }
                }
            }
            DieWithMessage("recvfrom has failed");
        }
        else
        {
            if (loss_rate > drand48())
            {
                continue;
                printf ("RECEIVE PACKET %d length %d\n", currentPacket.seq_no, currentPacket.length);
            }

            //store in buffer and send ack
            if (currentPacket.seq_no == highestPacket+1 )
            {
                highestPacket++;
                int offset = chunkSize*currentPacket.seq_no;
                memcpy(&buffer[offset], currentPacket.data, currentPacket.length);
            }

            printf("SEND ACK %d\n", highestPacket);
            struct packet currentACK; //ack
            currentACK.type = htonl(2);
            currentACK.seq_no = htonl (highestPacket);
            currentACK.length = htonl(0);

            if (sendto(sock, &currentACK, sizeof(currentACK), 0, (struct sockaddr *) &clntAddr, clntAddrLen) != sizeof(currentACK))
            {
                DieWithMessage("sent wrong amount of bytes");
            }
        }
    }
// NOT REACHED
}



void timeoutAlarm(int number)
{
    perror("timeout");
    exit(1);
}

void DieWithMessage(char* message)
{
    perror(message);
    exit(1);
}