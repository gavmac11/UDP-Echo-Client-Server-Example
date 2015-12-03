Gavin MacKenzie

Contents of Folder:
README.txt - this file
packet.c - c file containing the structure of a UDP packet
UDPEchoClient-Timeout.c - this is the source for the sender
UDPEchoServer.c - this is the source for the reciever

How to compile:

To compile the sender, type "gcc -o sender UDPEchoClient-Timeout.c"

To compiile the reciever, type "gcc -o reciever UDPEchoServer.c"

How to run:

In the terminal, type "./sender server_ip server_port chunk_size window_size".
In the terminal, type "./reciever port_number chunk_size loss_rate_OPTIONAL". 
Substitute values for each of the parameters. Note, loss rate is optional. 

Example sender: ./sender 192.168.1.1 7575 505 3
Example reciever: ./reciever 7575 505 .5