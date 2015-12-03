#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "packet.c"

static const unsigned int TIMEOUT_SECS = 2; // Seconds between retransmits
static const unsigned int MAXTRIES = 5;     // Tries before giving up

unsigned int tries = 0; // Count of times sent - GLOBAL for signal-handler access

void CatchAlarm(int ignored); // Handler for SIGALRM
int max (int first, int second);
int min (int first, int second);
void DieWithMessage(char* message);

 int flag = 1;


int main(int argc, char *argv[])
{

    struct sockaddr_in serverAddress;
    
    int parameterCount;
    char *server;
    int port;
    int chunkSize;
    int windowSize;
    int numberOfPackets = 0;
    const int bufferSize = 8192;
    char buffer[8192] = "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";
    int packetACK = -1;
    int flag = 1;
    int packetSent = -1;
    int base = 0;
    int numBytes;
    
    if (argc !=5)
    {
        DieWithMessage("Enter correct parameters, <server_ip> <server_port> <chunk_size> <window_size>\n");
    }


    server = argv[1];
    port = atoi(argv[2]);
    chunkSize = atoi(argv[3]);
    windowSize = atoi(argv[4]);

    //Make sure chunk size is valid
    if (chunkSize > 512)
    {
        printf("Chunk Size should be less than or equal to 512. Current Value - %i\n", chunkSize);
        exit;
    }
    
    //Determine the number of packets that need to be sent
    numberOfPackets = bufferSize/chunkSize;
    //If it does not divide into an equal amount of packets we will add one.
    if (bufferSize%chunkSize)
    {
        numberOfPackets++;
    }


    int sock; //socket descriptor

    // Create a reliable, stream socket using UDP
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); // Socket descriptor for client
    
    if (sock < 0)
    {
        DieWithMessage("socket() failed");
    }
    else
    {
        printf("Created UDP Socket\n");
    }



    // Set signal handler for alarm signal
    struct sigaction handler; // Signal handler
    handler.sa_handler = CatchAlarm;
    if (sigfillset(&handler.sa_mask) < 0) // Block everything in handler
    {
        DieWithMessage("sigfillset() failed");
        handler.sa_flags = 0;
    }

    if (sigaction(SIGALRM, &handler, 0) < 0)
    {
        DieWithMessage("sigaction() failed for SIGALRM");
    }

    //Server address struct
    memset(&serverAddress, 0, sizeof(serverAddress)); //zero structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(server); //IP
    serverAddress.sin_port = htons(port); //port


    int counter = 0;
    while ( (packetACK < numberOfPackets-1) && (tries < MAXTRIES) )
    {
        if (flag > 0)
        {
            flag = 0;

            int i;
            for (i = 0; i < windowSize; i++)
            {
                //finding the highest packet that was sent
                packetSent = min(max(i + base, packetSent), numberOfPackets - 1);

                //create the packet we are currently going to send
                struct packet packetToSend;
                if ( (i+base) < numberOfPackets)
                {
                    memset(&packetToSend, 0, sizeof(packetToSend));
                    printf("Send Packet %i\n", (i+base) );

                    packetToSend.type = htonl(1);
                    packetToSend.seq_no = htonl(i+base);

                    int length;
                    if ((bufferSize - ((i+base)*(chunkSize))) >= chunkSize )
                    {
                        //set the packet size except for the last packet
                        length = chunkSize;
                    }
                    else
                    {
                        length = bufferSize % chunkSize;
                    }

                    packetToSend.length = htonl(length);
                    memcpy(packetToSend.data, buffer + ((i+base)*(chunkSize)), length);

                    if (sendto(sock, &packetToSend, (sizeof(int)*3) + length, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != ((sizeof(int)*3) + length) )
                    {
                        DieWithMessage("sendto() error - sent unexpected number of bytes");
                    }
                }
            }
        }
    }




    // Receive a response

    struct sockaddr_storage fromAddr; // Source address of server
    // Set length of from address structure (in-out parameter)
    socklen_t fromAddrLen = sizeof(fromAddr);

    alarm(TIMEOUT_SECS); // Set the timeout

   // char incomingBuffer[MAXSTRINGLENGTH + 1]; // I/O buffer

    struct packet incomingBuffer;

    while ((numBytes = recvfrom(sock, &incomingBuffer, sizeof(int)*3, 0, (struct sockaddr *) &fromAddr, &fromAddrLen)) < 0)
    {
        if (errno == EINTR) // Alarm went off
        {
            if (tries < MAXTRIES) // Incremented by signal handler
            {
                printf("Timed out, will attempt %i more times.\n", (MAXTRIES - tries) );
                break;
            }
            else
            {
                DieWithMessage("No Response - unable to communicate with server");
            }
        }
        else
        {
            DieWithMessage("recvfrom() failed");
        }
    }


    if (numBytes)
    {
        int type = ntohl(incomingBuffer.type);
        int ackNumber = ntohl(incomingBuffer.seq_no);

        if ((ackNumber > packetACK) && type == 2)
        {
            printf("RECEIVED ACK %i\n", ackNumber );
            packetACK++;

            if (packetACK == packetSent)
            {
                alarm(0);
                tries = 0;
                flag = 1;
            }
            else
            {
                tries = 0;
                flag = 0;
                alarm(TIMEOUT_SECS);
            }
        }
    }

    int j;
    for (j = 0; j < 10; j++)
    {
        struct packet timingOUT;
        timingOUT.type = htonl(4);
        timingOUT.seq_no = htonl(0);
        timingOUT.length = htonl(0);
        sendto(sock, &timingOUT, (sizeof(int)*3), 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    }

    close(sock);
    exit(0);

}


void DieWithMessage(char* message)
{
    perror(message);
    exit(1);
}


int min (int first, int second)
{
    if (second > first)
    {
        return first;
    }
    return second;
}

int max (int first, int second)
{
    if (second > first)
    {
        return second;
    }
    return first;
}

// Handler for SIGALRM
void CatchAlarm(int ignored)
{
    flag = 1;
    tries += 1;
}

