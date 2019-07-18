#include <stdio.h>	// Client side implementation of UDP client-server model  
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>
  
#define PORT     8080 
#define BLOCK_SIZE 1024 


unsigned char * int_str(long int, int);
long int str_int(char *, int);
long int init();
int load_buffer(unsigned char [], long int, long int);
void probab(char *, int);
char* stringToBinary(char*);
char* crc(char*);
char filename[100];

int main() { 
    int sockfd; 
    struct sockaddr_in     servaddr; 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 		// Creating socket file descriptor 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 			    // Filling server information 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 

	long int n = init(),block_no = 0;
	long int no_of_blocks = ((n/BLOCK_SIZE) + 1);
	char *drop= (char*)malloc(sizeof(char)*no_of_blocks);
	int len;
	char  *meta = int_str(n, 32);
	meta[32]='\0';
	probab(drop, no_of_blocks);
	long int x = sendto(sockfd, (const unsigned char *)meta, strlen(meta), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
		printf("\nSent1");		
	meta = int_str(BLOCK_SIZE, 32);
	sendto(sockfd, (const unsigned char *)meta, strlen(meta), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
		printf("\nSent2");
	char last_block[BLOCK_SIZE]; //= (char *)malloc(sizeof(char)*BLOCK_SIZE);		
	while(block_no < no_of_blocks){
		
		unsigned char buffer[BLOCK_SIZE+32+8]= {0}, file_buffer[BLOCK_SIZE]={0};
		strncpy(buffer, int_str(block_no+1, 32),32); 
		load_buffer(file_buffer, n, block_no);
		char *crcode = crc(stringToBinary(file_buffer));
		//printf("\nCRCode :%s", crcode);	
		
		if(block_no == 0) strncpy(last_block, file_buffer, BLOCK_SIZE);
		else
			for(int k=0;k<BLOCK_SIZE;k++) 
				last_block[k]=last_block[k] ^ file_buffer[k];

		if (drop[block_no] == 1){++block_no;continue;}
		strncpy(buffer+32, file_buffer, BLOCK_SIZE);
		strcat(buffer+32+BLOCK_SIZE, crcode);
		//printf("\n\n\nSending...%s",buffer);		
	   	sendto(sockfd, (const unsigned char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
		printf("\nBlockNo.%ld\tBlock_Len:%ld\t Sent",++block_no, strlen(buffer));
	}   
	sendto(sockfd, (const char *)last_block, BLOCK_SIZE, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	printf("\nAll Sent...>>\n");
	//for(int k=0;k<BLOCK_SIZE;k++) 
		//printf("%c", last_block[k]);
    //n = recvfrom(sockfd, buffer, BLOCK_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
    //buffer[n] = '\0'; 
    //printf("Server : %s\n", buffer); 
 	printf("\nClosing Socket::%d",close(sockfd)); 
    return 0; 
} 
///////////////////////////////////////////////////////////////////////////////////////////////////////


void probab(char *arr, int NOPK){
	int p=0, i=0, j=0;
	printf("NO. of packets to be dropped: ");
	scanf("%d", &p);
	//p = (p*NOPK)/100;
	printf("No. corrupt pkts are: %d\n", p);
	for(i=0;i<NOPK;i++)
		arr[i] = 0;
	i=0;
	while(i<p){
		srand(time(NULL));
		j = random()%NOPK;
		if(arr[j] == 0){
			arr[j] = 1;
			i++;		
		}
	} 
	for(i=0;i<NOPK;i++){printf("%d.",arr[i]);}
	printf("\n");
}


long int init(){
		printf("\nEnter file name to send in current dir(contain only [A-Z|a-z|0-9|.]): ");	
		scanf("%s",filename);

		FILE *fptr;
		if ((fptr = fopen(filename, "rb")) == NULL){
		    printf("\n[ERROR! OPENING FILE]");	        // Program exits if the file pointer returns NULL.
		    exit(1);
	   	}
		fseek(fptr, 0L, SEEK_END);
		long int size = ftell(fptr);
		fclose(fptr);
		printf("\nSize of file:: %ld",size );	
		return size;
	}



int load_buffer(unsigned char file_buffer[BLOCK_SIZE], long int n, long int block_no){

	long int n1 = 1 + (n/BLOCK_SIZE);
	FILE *fptr;
		if ((fptr = fopen(filename, "rb")) == NULL){
		    printf("[ERROR! OPENING FILE]");	        // Program exits if the file pointer returns NULL.
		    exit(1);
	   	}	
		fseek(fptr, block_no*BLOCK_SIZE, SEEK_SET);
		if(block_no==n1-1){printf("\nLast Block...offset= %ld", n%BLOCK_SIZE);
		fread(file_buffer, n%BLOCK_SIZE, 1, fptr);}
		else {fread(file_buffer, BLOCK_SIZE, 1, fptr);}
		//printf("\n\t\t---OK---\n");
		int j=0;		
		for(int i=0;file_buffer[i]!='\0';i++){j++;}
		//printf("strlen = %ld, j=%d", strlen(file_buffer), j);
		return fclose(fptr);
}

unsigned char * int_str(long int a, int len){
	unsigned char *str= (unsigned char *)malloc(sizeof(char)*len);	
	int i=len-1;	
	while(i != -1){	unsigned char temp = (unsigned char)(48+a%10);
			str[i--] = temp;
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

char* crc(char *msg){
	int m = strlen(msg);
	char *encoded = (char *)malloc(sizeof(char)*(m+1)*8), *crc = "101010101";	
	int  n = strlen(crc), i, j;
	strcpy(encoded, msg);
	for(i=1; i<n; i++)
		strcat(encoded,"0");

	for(i=0; i<=strlen(encoded)-n;){
		for(j=0; j<n; j++)
			encoded[i+j]=encoded[i+j]==crc[j]?'0':'1';
		for(;i<strlen(encoded) && encoded[i]!='1';i++);
	}
	return (encoded+strlen(encoded)-n+1);
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
