struct packet
{
	int type;
	int seq_no;
	int length;
	char data[512];
};