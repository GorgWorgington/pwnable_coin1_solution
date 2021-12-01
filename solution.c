// This problem requires a combination of establishing a TCP connection with a server,
// string parsing, and binary search algorithm.
// Note that due to network delay the 60 second time limit is not enough time unless
// you run the solution directly on their SSH server and have it connect to localhost.
// This doesn't have DNS resolution implemented so it only takes raw ip addresses.
// run `./solution 128.61.240.205 9007` after compiling with `gcc -o solution solution.c`
/*
	---------------------------------------------------
	-              Shall we play a game?              -
	---------------------------------------------------
	
	You have given some gold coins in your hand
	however, there is one counterfeit coin among them
	counterfeit coin looks exactly same as real coin
	however, its weight is different from real one
	real coin weighs 10, counterfeit coin weighes 9
	help me to find the counterfeit coin with a scale
	if you find 100 counterfeit coins, you will get reward :)
	FYI, you have 60 seconds.
	
	- How to play - 
	1. you get a number of coins (N) and number of chances (C)
	2. then you specify a set of index numbers of coins to be weighed
	3. you get the weight information
	4. 2~3 repeats C time, then you give the answer
	
	- Example -
	[Server] N=4 C=2 	# find counterfeit among 4 coins with 2 trial
	[Client] 0 1 		# weigh first and second coin
	[Server] 20			# scale result : 20
	[Client] 3			# weigh fourth coin
	[Server] 10			# scale result : 10
	[Client] 2 			# counterfeit coin is third!
	[Server] Correct!

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define READ_BYTES 2048

char* find_char(char* str1, const char* str2, int n); //find the first occurrence in first n bytes of str1 of any character in str2.
int check(int range_beg, int range_end, char* read_buf, int fd); //writes a guess to socket and returns 1 if the counterfeit is inside. returns -1 otherwise. returns 0 if counterfeit was found.
int get_total_digits(int range_beg, int range_end); //find the sum of the digits in a range (inclusive).
int int_pow(int base, int exponent);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("usage: solution ip port\n");
        return 1;
    }

    char* serv_ip = argv[1];
    int serv_port = atoi(argv[2]);

    int client_socket_fd;
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = inet_addr(serv_ip);

    if (connect(client_socket_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Failed to connect");
        return 1;
    }

    int num_bytes;
    char read_buf[READ_BYTES];
    memset(read_buf, 0x00, sizeof(read_buf)); //not strictly necessary
    num_bytes = read(client_socket_fd, read_buf, READ_BYTES); //Read the first message. It's just the introduction for the game.

    while((num_bytes = read(client_socket_fd, read_buf, READ_BYTES)) > 0)
    {
        write(0, read_buf, num_bytes);

        char* num1 = find_char(read_buf, "123456789", num_bytes);
        char* space = find_char(read_buf, " ", num_bytes);
        char* num2 = find_char(space, "123456789", num_bytes - (space - read_buf));
        *space = '\0'; //add a null terminator so atoi() will work.
        *(read_buf + num_bytes) = '\0'; //ditto

        int N = 0;
        int C = 0; //This literally never gets used because the number of tries is always enough for binary search.
        if (num1 != NULL) {N = atoi(num1);}
        if (num2 != NULL) {C = atoi(num2);}

        int range_beg = 0;
        int range_end = N - 1;
        
        int range_mid = (range_end + range_beg) / 2;
        
        int checkval;
        while((checkval = check(range_beg, range_mid, read_buf, client_socket_fd)))
        {
            if (checkval > 0)
            {
                //counterfeit is inside [range_beg, range_mid]
                range_end = range_mid;
            } else
            {
                range_beg = range_mid+1;
            }
            range_mid = (range_end + range_beg) / 2;
        }
    

    }

    return 0;
}

int check(int range_beg, int range_end, char* read_buf, int fd)
{

    int num_char = get_total_digits(range_beg, range_end) + (range_end - range_beg) + 1; //total digits + spaces and newline
    char write_buf[num_char];
    char* p = write_buf;

    for (int i = range_beg; i <= range_end; ++i)
    {
        int n;
        sprintf(p, "%d %n", i, &n); //never thought I would ever use %n in my entire life.
        p += n;
    }
    *(p-1) = '\n'; //replace last space with \n
    write(0, write_buf, num_char);
    //printf("|%d|\n", num_char - (p - write_buf));
    write(fd, write_buf, num_char);

    int num_read = read(fd, read_buf, READ_BYTES);
    write(0, read_buf, num_read);

    read_buf[num_read-1] = '\0'; //change the newline to null terminator

    if (read_buf[0] == 'C' || read_buf[0] == 'f')
    {
        return 0;
    }

    int weight = atoi(read_buf);

    if (weight % 10)
    {
        return 1;
    }else
    {
        return -1;
    }

}

int get_total_digits(int range_beg, int range_end)
{
    int sum = 0;

    ++range_end; //accounts for the last number not being counted

    while (range_beg < range_end)
    {
        int n;
        if (range_beg == 0)
        {
            n = 1;
        }
        else
        {
            n = (int)(log10((double)range_beg)) + 1; //int cast rounding down is necessary
        }
        int next = int_pow(10, n);

        if (next < range_end)
        {
            sum += (next - range_beg) * n;
            range_beg = next;
        }
        else
        {
            sum += (range_end - range_beg) * n;
            break;
        }
    }
    return sum;
}

int int_pow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp % 2) //exp & 1;
           result *= base;
        exp /= 2; //exp >>= 1;
        base *= base;
    }
    return result;
}

char* find_char(char* str1, const char* str2, int n)
{
    for (int i = 0; i < n; ++i)
    {
        for (char* p = (char*)str2; *p != '\0'; ++p)
        {
            if (str1[i] == *p)
            {
                return str1 + i;
            }
        }
    }
    return NULL;
}