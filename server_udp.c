#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define PORT     8080 
long int BLOCK_SIZE = 0, file_size=0, no_of_blocks=0;

int cr_check(char* );
char *int_str(long int, int);
long int str_int(char *, int);
char* stringToBinary(char* );

int main() { 
    int sockfd; 
    //const char *hello = "Hello Client...How you doing?"; 
    struct sockaddr_in servaddr, cliaddr; 
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ 	// Creating socket file descriptor 
        perror("[SOCKET CREATION FAILED]"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr));    	// Filling server information 
    servaddr.sin_family    = AF_INET; 			// IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
	printf("\nWating for sender to bind and send file information...");
    if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)	//Bind the socket with the server address 
    { 
        perror("[BIND FAILED]"); 
        exit(EXIT_FAILURE); 
    } 

	unsigned char buffer[32]={0};
	int req_no = 0, len,n;
	recvfrom(sockfd, (char *)buffer, 32, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
	file_size = str_int(buffer, 32);
	printf("\nFile Size received...");
	recvfrom(sockfd, (char *)buffer, 32, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
	BLOCK_SIZE = str_int(buffer, 32);
	printf("\nBlock Size received...");		
	unsigned char receive_buffer[4096][BLOCK_SIZE];
	no_of_blocks = 1 + (file_size/BLOCK_SIZE);
	printf("\nNO. of blocks to be received: %ld \nBLOCK_SIZE = %ld \nfile_size = %ld", no_of_blocks, BLOCK_SIZE, file_size);
	char *arr= (char *)malloc(sizeof(char)*no_of_blocks) , last_block[BLOCK_SIZE];
	for(int j=0;j<no_of_blocks;j++){*(arr+j)='0';}
    while(1){  
 		unsigned char buffer[BLOCK_SIZE+32+8];
		n = recvfrom(sockfd, (unsigned char *)buffer, BLOCK_SIZE+32+8, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);		
		buffer[n] = '\0';

		unsigned char header[32]={0};
		strncpy(header, buffer, 32);
		long int block_no = str_int(header, 32);
		*(arr+block_no-1)='1';
		char  *temp=(char *)malloc(sizeof(char)*BLOCK_SIZE), *temp1=(char *)malloc(sizeof(char)*BLOCK_SIZE*8+8);

		strncpy(temp,buffer+32, BLOCK_SIZE);
		strcpy(temp1, stringToBinary(temp));
		strcat(temp1, buffer+32+BLOCK_SIZE);	
		//printf("\n\n\t\tOK--OK\n\n");
		//printf("\nCRCode: %s",buffer+BLOCK_SIZE+32); 	
		printf("\t crc %d",cr_check(buffer+32));   //this is crc check;
		strncpy(receive_buffer[block_no-1], buffer+32, BLOCK_SIZE);
		
		//if(block_no == no_of_blocks-1)
			//receive_buffer[block_no][file_size%BLOCK_SIZE]='\0';
			
		//printf("\n\n\n\nReceiveding...|||||%s|||||\n\n\n",receive_buffer[block_no-1]);		
		printf("\nBLOCK_NO.:%ld\tBLOCK_LEN: %d\treceived", block_no, n); 
		
		//sendto(sockfd, hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
		
		if ((long int)req_no == no_of_blocks){
			printf("\nLast Block Received...");    
			break; 
		}
		if((long int)block_no >= no_of_blocks && (long int)req_no != no_of_blocks){
			n = recvfrom(sockfd, (char *)last_block, BLOCK_SIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);		
			//last_block[n] = '\0';
			printf("\n(K+1)th block received... press 1 to exit loop");
			scanf("%d", &n);
			if(n==1) break;
		}
      }
	printf("\nRetriving erased block(if any)...");
	for(int j=0;j<no_of_blocks; j++){
		if (*(arr+j) == '1') {
			for(int k=0;k<BLOCK_SIZE;k++) 
					last_block[k] = last_block[k] ^ receive_buffer[j][k]; 
		}
	}
//printf("\n\n\n>>>\n");
	//for(int k=0;k<BLOCK_SIZE;k++) 
	//	printf("%c", (int)last_block[k]);	
//printf("\n<<<\n\n");
	printf("\nRecipt::\n");
	int k=0;
	for(int j=0;j<no_of_blocks; j++){
		printf("%c ",*(arr+j));
		if(*(arr+j)=='0'){
			strncpy(receive_buffer[j], last_block, BLOCK_SIZE);
			k=j+1;

		}
	}
	printf("\nErased and then retrived Block_No.: %d", k);
	//printf("\nNo. of Erased Blocks are: %d",k);
	char filename[100];
	printf("\nEnter the new file name(contain only [A-Z|a-z|0-9|.]): ");	
	scanf("%s",filename);
	FILE *fptr;
	if ((fptr = fopen(filename,"wb")) == NULL){
	   printf("\nERROR! OPENING FILE");	// Program exits if the file pointer returns NULL.
	   exit(1);
	}
	//fseek(fptr, block_no*BLOCK_SIZE, SEEK_SET);
	
	for(int i=0; i<no_of_blocks-1; i++)
		fwrite(receive_buffer[i], BLOCK_SIZE, 1, fptr);
	fwrite(receive_buffer[no_of_blocks-1], file_size%BLOCK_SIZE, 1, fptr);
	printf("\nClosing File: %d\n", fclose(fptr));
    return 0; 
} 
//////////////////////////////////////////////////////////////////////////////////////////////////////////


int cr_check(char *temp){
	char *crc = "101010101" ,encoded[(1024*8)+8];
	char *temp2 = (char*)malloc(sizeof(char)*BLOCK_SIZE);
	strncpy(temp2, temp, BLOCK_SIZE);
	strncpy(encoded, stringToBinary(temp2), BLOCK_SIZE*8);
	for(int i=0;i<8;i++)
		encoded[(BLOCK_SIZE*8)+i] = temp[BLOCK_SIZE+i];
	int m = strlen(encoded), n = strlen(crc), i, j;
	//printf("\nn=%d,\t m=%d", n,m);
	//printf("\nCRCODE: %s-------\n",encoded+BLOCK_SIZE*8);
	for(i=0; i<=BLOCK_SIZE*8;){
		for(j=0; j<8; j++)
			encoded[i+j]=encoded[i+j]==crc[j]?'0':'1';
		for(;i<strlen(encoded) && encoded[i]!='1';i++);
	}
	for (i=0 ;i<strlen(encoded)-8; i++)
		if(encoded[i]=='1')
			return 0;
	return 1;
}

char* stringToBinary(char* s) {
    if(s == NULL) return 0; /* no input string */
    size_t len = strlen(s);
    char *binary = malloc(len*8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
    binary[0] = '\0';
    for(size_t i = 0; i < len; ++i) {
        char ch = s[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat(binary,"1");
            } else {
                strcat(binary,"0");
            }
        }
    }
    return binary;
}

char * int_str(long int a, int len){
	char *str= (char *)malloc(sizeof(char)*len);	
	long int i=len-1;	
	while(i != 0){	char temp = (char)(48+a%10);
			*(str + i--) = temp;
			a = a / 10;
	}
	return str;
}

long int str_int(char *str, int len){
	long int i=0,j=0;
	for(j=0;j<len;j++)
		if((long int)str[j] >47 && (long int)str[j] < 58)
			i = i*10 + (long int)str[j] - 48;
	return i;
}
