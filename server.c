#include "unpifiplus.h"
#include "myunprtt.h"
#include <setjmp.h>
#define RTT_DEBUG

extern struct ifi_info *Get_ifi_info_plus(int family, int doaliases);
extern void free_ifi_info_plus(struct ifi_info *ifihead);
void sig_chld(int);


struct hdr {
	uint32_t seq;
	uint32_t ts;
	int slwnd;
	int ack;
	int islast;
};
struct hdr sendhdr, recvhdr;

struct congestion
{
	int ssthreshold;
	float cwnd;

}cong;

struct sock {
	int sfd;
	struct sockaddr_in *ipaddr;
	struct sockaddr_in *netmask;

	unsigned long subaddr;
	struct sock *next;
}*so;

struct client {
	struct in_addr *caddr;
	int port;
	pid_t tid;
	struct client *next;
};

struct slwndow {
	struct iovec iovs[2];
	struct slwndow *next;
};

struct server_in_args {
	int port;
	int window_size;
};
struct server_in_args ser;

static struct rtt_info rttinfo;
struct itimerval tout_val,zero_val;
static int rttinit = 0;
//static struct msghdr msgsend,msgrecv;
static void sig_alrm(int signo);
static sigjmp_buf jmpbuf;
fd_set rfds,rfds1,rfds2;
struct timeval tv,tv1;
int retval,ret,retval2;
int maxrfd;

struct iovec gp[2],gp1[2],gp2[2],gh[2];
void *inbuff, *outbuff;
size_t inbytes = 512 - sizeof(recvhdr);
ssize_t outbytes = 512 - sizeof(sendhdr);
ssize_t rec, recd;
struct iovec iovsend[2], iovrecv[2];     //iovrecv should be 1
struct hdr *heads;

//ssize_t dg_send_recv(int,const void *,int,void *,int,const SA *,socklen_t);
void addnode(struct sock **, int, struct sockaddr_in *, struct sockaddr_in *);//,struct sockaddr_in  );
int verifyclient(struct client **, struct sockaddr_in *);
void addclient(struct client **,struct sockaddr_in *,pid_t);
void deleteclient(struct client **,pid_t);
struct sockaddr_in *getaddr(struct sock **, int, struct sockaddr_in **);
void addpacket(struct slwndow **, struct iovec *);
void freepacket(struct slwndow **, struct iovec *);
void display(struct slwndow **);
//struct hdr *gethead(struct slwndow **);
void getpacket(struct slwndow **,int ,struct iovec *);
void gethead(struct slwndow **,struct iovec *);
int length(struct slwndow **);
int maxi(int,int);

struct sock *head;
struct slwndow *h =NULL;
struct slwndow *temp1;

FILE *fp;
char buffer[512],buffer1[512];
size_t result;
char *loc = "127.0.0.1";
struct client *headptr = NULL;
//struct client *hptr=NULL;
struct sockaddr_in servchadd;
struct sockaddr_in *p, *clieaddr, *serad;
int f, x, t, wr,w, rd, l, b, sf, d, csf,recval1,recval2, flag = 0,flag1=0,inc,
		maxwnd,counter,akno,wndsize,lastseq,sendag,rto,sec,msec,ackno,exack,last,prwnd,tempwnd,tempwnd1,rc=0,rc1=0,temwnd;
int sockfd[10];
long lsize;
uint32_t timerv,timest;

char buf[MAXLINE], buff2[MAXLINE], buff4[MAXLINE], buff5[MAXLINE], strg[16],
		mesg[MAXLINE], recvc[MAXLINE], st[16],recvline[MAXLINE + 1]; //rnd[MAXLINE] = "hello",

socklen_t leng;
const int optval1 = 1, optval2 = 1;


int main(int argc, char **argv) {
	int maxfd, nready, is_connected;
	fd_set rset, allset;
	FD_ZERO(&allset);
	head = NULL;
	FILE *fp1;
	char s[80];

	const int on = 1;

	pid_t pid;
	char str1[16], str2[16], buff[MAXLINE];
	char ipstr[INET6_ADDRSTRLEN];
	/*gp[0]=(struct iovec *)malloc(sizeof(struct iovec));
	gp[1]=(struct iovec *)malloc(sizeof(struct iovec));
	gp1[0]=(struct iovec *)malloc(sizeof(struct iovec));
	gp1[1]=(struct iovec *)malloc(sizeof(struct iovec));
	gp2[0]=(struct iovec *)malloc(sizeof(struct iovec));
	gp2[1]=(struct iovec *)malloc(sizeof(struct iovec));
	gh[0]=(struct iovec *)malloc(sizeof(struct iovec));
	gh[1]=(struct iovec *)malloc(sizeof(struct iovec));*/


	fp1 = fopen("server.in", "r");
	if (fp1 == NULL ) {
		puts("The server.in file cannot open file\n");
		exit(1);
	}

	while (fscanf(fp1, "%d%d", &ser.port, &ser.window_size) != EOF)
		printf("\n%d %d\n", ser.port, ser.window_size);
	/*while(fgets(s,50,fp)!=NULL)
	 printf("%s",s);*/
	fclose(fp1);

	struct ifi_info *ifi, *ifihead;
	struct sockaddr_in *sa, *nm;			//*clieaddr,*serad;
	struct sockaddr cliaddr;
	struct sockaddr * temp;
	temp = (struct sockaddr *) malloc(sizeof(struct sockaddr));
	memset(temp, 0, sizeof(struct sockaddr));
	struct sockaddr_in sna;

	//struct in_addr *p;
	//struct in_addr *serad;
	p = NULL;
	u_char *ptr;
	int m, j, k, i, n, family, largest;

	x = 0;
	socklen_t len;
	signal(SIGCHLD, sig_chld);
	signal(SIGALRM,sig_alrm);

	for (ifihead = ifi = Get_ifi_info_plus(AF_INET, 1); ifi != NULL ;
			ifi = ifi->ifi_next) {
		//printf("after ifiinfo\n");
		sockfd[x] = socket(AF_INET, SOCK_DGRAM, 0);
		setsockopt(sockfd[x], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		bzero(&sa, sizeof(sa));
		bzero(&nm, sizeof(nm));
		sa = (struct sockaddr_in *) ifi->ifi_addr;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(ser.port);
		bind(sockfd[x], (SA *) sa, sizeof(*sa));
		//printf("bound %s\n", sock_ntop((SA *) sa, sizeof(*sa)));

		nm = (struct sockaddr_in *) ifi->ifi_ntmaddr;
		//nm=(struct in_addr *)ifi->ifi_ntmaddr;
		//printf(" -------------------------- network mask: %s\n",
				//sock_ntop_host((SA *) nm, sizeof(*nm)));
printf("-------------------------------------------------------------------------\n");
		//sna.sin_addr.s_addr=(sa->sin_addr.s_addr & nm->sin_addr.s_addr);
//pri

		addnode(&head, sockfd[x], sa, nm);		//,sna);
		x++;
		//printf("no of sfds is %d\n", x);
	}

	for (;;) {
		//printf("after for\n");
		FD_ZERO(&allset);
		for (j = 0; j < x; j++)
			FD_SET(sockfd[j], &allset);
		largest = sockfd[0];
		for (k = 0; k < x; k++) {
			if (sockfd[k] > largest)
				largest = sockfd[k];
		}

		maxfd = largest + 1;
		//rset = allset;

		if ((nready = select(maxfd, &allset, NULL, NULL, NULL )) < 0) {
			if (errno == EINTR) {
				printf("Interrupted by a signal\n");
				continue;
			} else {
				printf("select error\n");
				sleep(5);
			}
		}
		printf("before isset\n");
		for (l = 0; l < x; l++) {
			if (FD_ISSET(sockfd[l],&allset)) {
				//printf("Inside isset\n");

				sf = sockfd[l];
				printf("%d\n", l);
				serad = getaddr(&head, sockfd[l], &p);
				bzero(&cliaddr, sizeof(cliaddr));

				//so->sfd=sockfd[l];
				//printf(so->sfd);
				len = sizeof(cliaddr);
				n = recvfrom(sf, buf, MAXLINE, 0, &cliaddr, &len);

				//sendto(sf,rnd,MAXLINE,0,&cliaddr,len);
				printf("Received file name is %s\n", buf);
				//printf("sfd is %d\n", sf);
				//printf("Inside isset2\n");

				//getpeername(sf,(SA *)&cliaddr,&len);
				struct sockaddr_in *s = (struct sockaddr_in *) &cliaddr;
				int ports = ntohs(s->sin_port);
				inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
				printf("Peer IP address: %s\n", ipstr);
				printf("Peer port      : %d\n", ports);
				//printf("length of cliaddr is %d\n", len);

				clieaddr = (struct sockaddr_in *) &cliaddr;
				//printf("The client is connected to the server address %s and port %d\n",inet_ntop(AF_INET,&(serad->sin_addr.s_addr),buff2,sizeof(buff2)),ntohs(serad->sin_port));
				//printf("  network mask: %s\n",
				//									sock_ntop_host((struct in_addr *)p, sizeof(*p)));
				printf(
						"The client is connected to the server address %s and port %d\n",
						inet_ntop(AF_INET, &(serad->sin_addr.s_addr), buff2,
								sizeof(buff2)), ntohs(serad->sin_port));
				printf("  network mask: %s\n",
						sock_ntop_host((SA *) p, sizeof(*p)));

				//printf("The client connected to the server address %s",inet_ntop(AF_INET,sa->sin_addr.s_addr,buff2,sizeof(buff2)));

				printf("connection from %s, port %d\n",
						inet_ntop(AF_INET, &(clieaddr->sin_addr.s_addr), buff,
								sizeof(buff)), ntohs(clieaddr->sin_port));
				//inet_ntop(AF_INET,&cliaddr->sin_addr.s_addr,buff,sizeof(buff));

				is_connected = verifyclient(&headptr, clieaddr);
				//if (memcmp(temp, &cliaddr, sizeof(struct sockaddr)) == 0)
					//printf("hgkjhgk\n");
				//printf("Inside isset3\n");

				if (is_connected == 0)

					printf("You are already connected to the server\n");

				else
				{
					printf("before frk\n");
					pid = fork();
				if (pid == 0) {
					printf("inside fork\n");




					//printf("inside child\n");
					unsigned long cli_subnet = (serad->sin_addr.s_addr
							& p->sin_addr.s_addr);
					unsigned long ser_subnet = (clieaddr->sin_addr.s_addr
							& p->sin_addr.s_addr);
					//unsigned long cli_subnet=(serad->sin_addr.s_addr & p->sin_addr.s_addr);
					//unsigned long ser_subnet=(clieaddr->sin_addr.s_addr & p->sin_addr.s_addr);

					for (l = 0; l < x; l++) {
						printf("Inside isset4\n");
						if (sockfd[l] != sf) {
							//printf("the closed socket fd is %d\n", sockfd[l]);
							close(sockfd[l]);
						}
						//printf("sfd is %d\n", sf);
					}

					//printf(so->sfd);
					if (strcmp(buff2, "127.0.0.1")) {
						flag = 1;
						printf("The client host is local to the server\n");
						/*inet_pton(AF_INET,loc,&serad->sin_addr.s_addr);

						 inet_pton(AF_INET,loc,&clieaddr->sin_addr.s_addr);
						 printf("The client is connected to the server address %s and port %d\n"
						 ,inet_ntop(AF_INET,&(serad->sin_addr.s_addr),buff4,sizeof(buff4)),ntohs(serad->sin_port));
						 printf("connection from %s, port %d\n",
						 inet_ntop(AF_INET,&(clieaddr->sin_addr.s_addr),buff5,sizeof(buff5)),ntohs(clieaddr->sin_port));*/
						setsockopt(sf, SOL_SOCKET, SO_DONTROUTE, &optval1,
								sizeof(optval1));

					}

					else if (cli_subnet == ser_subnet) {
						flag = 1;
						printf("The client host is local to the server\n");
						/*
						 inet_pton(AF_INET,loc,&serad->sin_addr.s_addr);

						 inet_pton(AF_INET,loc,&clieaddr->sin_addr.s_addr);
						 printf("The client is connected to the server address %s and port %d\n"
						 ,inet_ntop(AF_INET,&(serad->sin_addr.s_addr),buff4,sizeof(buff4)),ntohs(serad->sin_port));
						 printf("connection from %s, port %d\n",
						 inet_ntop(AF_INET,&(clieaddr->sin_addr.s_addr),buff5,sizeof(buff5)),ntohs(clieaddr->sin_port));
						 */setsockopt(sf, SOL_SOCKET, SO_DONTROUTE, &optval1,
								sizeof(optval1));

					} else
						printf("The client host is not local to the server\n");
					csf = socket(AF_INET, SOCK_DGRAM, 0);
					bzero(&servchadd, sizeof(servchadd));
					servchadd.sin_family = AF_INET;
					servchadd.sin_addr.s_addr = serad->sin_addr.s_addr;
					servchadd.sin_port = htons(0);
					if (bind(csf, (SA *) &servchadd, sizeof(servchadd)) == -1) {
						printf("The server cannot bind\n");
						return 1;
					}
					leng = sizeof(servchadd);
					if (getsockname(csf, (SA *) &servchadd, &leng) < 0)
						return -1;
					printf(
							"The IP address of the server is %s and the port number assigned to the socket created in the child is %d\n",
							inet_ntop(AF_INET, &servchadd.sin_addr, strg,
									sizeof(strg)), ntohs(servchadd.sin_port));
					clieaddr->sin_family = AF_INET;
					if (connect(csf, (SA *) &cliaddr, sizeof(cliaddr)) < 0)
						printf("connect error");
					if (getpeername(csf, (SA *) &cliaddr, &len) < 0)
						return -1;
					printf("client ip addr is %s\n",
							inet_ntop(AF_INET, &clieaddr->sin_addr, st,
									sizeof(st)));
					unsigned long po = ntohs(servchadd.sin_port);
					printf("%ld", po);
					char a[200];
					sprintf(a, "%lu", po);
					strcpy(mesg, a);
					//printf("after bind\n");
					f = sendto(sf, mesg, MAXLINE, 0, &cliaddr, sizeof(cliaddr));
					printf("no of bytes sent is %d\n", f);
					printf("port number sent is %s\n", mesg);
					selectwait:
					FD_ZERO(&rfds);
					FD_SET(csf, &rfds);
					rc1++;
					if(rc1>12)
					{
					printf("The file transmission has been stopped as the client did not respond to 12 repeated requests to update its sliding window size\n");
					exit(1);
					}
					tv.tv_sec = 5;
					tv.tv_usec = 0;
					if ((retval = select(csf + 1, &rfds, NULL, NULL, &tv))
							< 0) {
						if (errno == EINTR) {
							printf("Interrupted by a signal\n");
							continue;
						} else {
							printf("select error\n");
							sleep(5);
						}

					}
					if (retval == 0) {
						sendto(sf, mesg, MAXLINE, 0, &cliaddr, sizeof(cliaddr));
						send(csf, mesg, MAXLINE, 0);
						goto selectwait;
					} else {
						if (FD_ISSET(csf,&rfds)) {
							/*if (d = recvfrom(csf, recvc, MAXLINE, 0, &cliaddr,
									&leng) > 0) {*/
							if (d = recv(csf, recvc, MAXLINE, 0) > 0){

								close(sf);
							}
							printf("The client sent its window size as %s\n",
									recvc);
						}

					}

					send(csf, mesg, MAXLINE, 0);
					sendhdr.slwnd=ser.window_size;
					if (ser.window_size > atoi(recvc)) {
						maxwnd = atoi(recvc);
						printf("port is %d\n", maxwnd);
					} else
						maxwnd = ser.window_size;
					tempwnd=maxwnd;
					cong.cwnd=1.0;
					cong.ssthreshold=maxwnd;
					fp = fopen(buf, "r");
					if (fp == NULL ) {
						/*char msg2[200]={"The specified file cannot be read as it is not present on the disk----please check the filename"};
						 send(csf,msg2,MAXLINE,0);*/
						printf(
								"The specified file cannot be read as it is not present on the disk----please check the filename\n");
						exit(1);
					}
					/*fseek (fp, 0, SEEK_END);
					 lsize = ftell (fp);
					 buffer = (char*) malloc (sizeof(char)*lsize);
					 if (buffer == NULL)
					 {
					 fputs ("Memory error",stderr);
					 exit (2);
					 }
					 result=fread (buffer,1,lsize,fp);

					 if (result != lsize) {
					 fputs ("Reading error",stderr);
					 exit (3);
					 }
					 strcpy(outbuff,buffer);
					 */
					// rec=dg_send_recv(csf,buffer,492,recvline,492,cliaddr,sizeof(cliaddr));
					//b=write(csf,buffer,MAXLINE);
					//int d=ntohs(mesg);
					//printf("%d\n",d);
					//sendto(sf,)
					memset((void *)&sendhdr, 0, sizeof(sendhdr));
					memset((void *)&recvhdr, 0, sizeof(recvhdr));
					printf("The initial value of congestion window is %d\n",(int)cong.cwnd);
					printf("The initial value of the ssthreshold is %d\n ",cong.ssthreshold);
					//for(t=0;t<maxwnd;t++)

				again:
				/*temp1=h;

				if(getlength(&h)> && maxwnd>0)
				{
					tempwnd1=cwnd;
					printf("Packets are already in the sending window\n");
					while(tempwnd1>0)
					{
						rtt_newpack(&rttinfo);
						gethead(temp1,gh);
						printf("sending the packet number with sequence number %d\n",((struct hdr *)h->iovs[0].iov_base)->seq);
						writev(csf,gh,2);
						temp=temp->next;
						tempwnd1--;

					}
				}*/



					temwnd=floor(cong.cwnd);



					//while (floor(cong.cwnd)>0 && maxwnd > 0 && floor(cong.cwnd)<=maxwnd) {
						while(temwnd>0 && maxwnd>0 && temwnd<=maxwnd){
						//temwnd=floor(cong.cwnd);
						printf("cwnd initially is %d\n",(int)floor(cong.cwnd));
						printf("maxwnd initially is %d\n",maxwnd);

						//printf("inside t=0\n");
						if (rttinit == 0) {
							rtt_init(&rttinfo);
							rttinit = 1;
							rtt_d_flag = 1;
						}
						//printf("after rttinit=0\n");
						sendhdr.seq++;
						sendhdr.islast=0;
						bzero(&buffer, sizeof(buffer));
						rd = fread(buffer, sizeof(char), outbytes, fp);
						//printf("file read is %s \n", buffer);
						if (feof(fp)) {
							sendhdr.islast = 1;
							outbytes = rd;
							//maxwnd = 0;
						}
						//sendhdr.slwnd = maxwnd;
						//msgsend.msg_name=(struct sockaddr_in *)cliaddr;
						//msgsend.msg_namelen=sizeof(cliaddr);
						//msgsend.msg_iov=iovsend;
						//msgsend.msg_iovlen=2;
						iovsend[0].iov_base = &sendhdr;
						iovsend[0].iov_len = sizeof(struct hdr);

						iovsend[1].iov_base = (char *) buffer;
						iovsend[1].iov_len = outbytes;

						//msgrecv.msg_name=NULL;
						//msgrecv.msg_namelen=0;
						//msgrecv.msg_iov=iovrecv;
						//msgrecv.msg_iovlen=2;
						/*iovrecv[0].iov_base = &recvhdr;
						iovrecv[0].iov_len = sizeof(struct hdr);
						iovrecv[1].iov_base = inbuff;
						iovrecv[1].iov_len = inbytes;
																*/
						//signal(SIGALRM,sig_alrm);
						rtt_newpack(&rttinfo);

						//  	sendagain:
						sendhdr.ts=rtt_ts(&rttinfo);
						printf("The time stamp sent is %u\n",sendhdr.ts);
						//sendhdr.ts=rtt_ts(&rttinfo);
						//sendmsg(csf,&msgsend,0);
						wr = writev(csf, iovsend, 2);
						addpacket(&h,iovsend);
						//display(&h);


						//if((maxwnd==1) || (sendhdr.islast==1))
						//if((floor(cong.cwnd)==1) || (sendhdr.islast==1))
						if((temwnd==1) || (sendhdr.islast==1))
						{
							printf("cwnd is %d\n",(int)floor(cong.cwnd));
							printf("maxwnd is %d\n",maxwnd);
						lastseq=sendhdr.seq;
						printf("last seq is %d\n",lastseq);
						}
						printf("Sequence number sent is %d and data is %s\n", ((struct hdr *)iovsend[0].iov_base)->seq,(char *) iovsend[1].iov_base);
						printf("Sending sequence number %d \n", ((struct hdr *)iovsend[0].iov_base)->seq);
						printf("Sending sequence number %d \n",sendhdr.seq);
						if(sendhdr.islast==1)
						{
						last=sendhdr.seq;
						break;
						}
						temwnd--;
						//cong.cwnd--;
						//maxwnd--;
					}

					//display(&h);
					/*debug*/
					//freepacket(&h,iovsend);
					//display(&h);
					/*debug*/
					sendag=0;
					printf("The window sent is between sequence numbers %d and %d\n",((struct hdr *)h->iovs[0].iov_base)->seq,lastseq);
					/*getpacket(&h,2,gp1);
					printf("after get packet sequence number is %d\n",((struct hdr *)gp1[0]->iov_base)->seq);*/
					timer:
					exack=(((struct hdr *)h->iovs[0].iov_base)->seq);
					if(sendag==1)
					{
						printf("A timeout has occurred and hence the value of cwnd is made %d and value of ssthreshold is made %d\n",(int)floor(cong.cwnd),cong.ssthreshold);
					getpacket(&h,exack,gp1);
					sendhdr.ts=rtt_ts(&rttinfo);

					printf("The time stamp sent is %u\n",sendhdr.ts);
					printf("Resending the packet with sequence number %d due to time out\n",exack);
					w=writev(csf,gp1,2);

					}
					//put the writev function
					rto=rtt_start(&rttinfo);

					printf("retransmission time is %d\n",rto);
					//convert into seconds and milliseconds
					if(rto>=1000)
					{
						//sec=rto/1000;
						rto=rto/1000;
					}
					msec=((rto)%1000)*1000;

					zero_val.it_interval.tv_sec = 0;
					zero_val.it_interval.tv_usec = 0;
															//  timerv=rtt_start(&rttifo)*1000;
					zero_val.it_value.tv_sec = 0;
					zero_val.it_value.tv_usec = 0;

					tout_val.it_interval.tv_sec = 0;
					  tout_val.it_interval.tv_usec = 0;
					  //timerv=rtt_start(&rttinfo)*1000;
					  tout_val.it_value.tv_sec = rto;
					  tout_val.it_value.tv_usec = msec;

					  setitimer(ITIMER_REAL, &zero_val,0);

					  setitimer(ITIMER_REAL, &tout_val,0);



					  if(sigsetjmp(jmpbuf,1)!=0)
					  {
						  cong.ssthreshold=maxi((cong.ssthreshold/2),2);
						  cong.cwnd=1;


						  printf("inside sigsetjump\n");
						  if(rtt_timeout(&rttinfo)<0)
						  {
							 err_msg("The client is not responding, so transimission is being stopped\n");
							  					 rttinit=0;
							  					 return -1;
						  }

						  sendag=1;
						  goto timer;
					  }



					//if(sendag==1)
					/*if(sigsetjmp(jmpbuf,1)!=0)
					{
						if(rtt_timeout(&rttinfo)<0)
						{
							err_msg("The client is not responding, so transimission is being stopped\n");
							rttinit=0;
							return -1;
						}
						wr=writev(csf,iovsend,2);
					}*/

				//	for(;;)
					//{

			waitread:
					FD_ZERO(&rfds1);
					FD_SET(csf, &rfds1);


					if((ret=select(csf+1,&rfds1,NULL,NULL,NULL))<0)
					{
						if (errno == EINTR)
						{
						printf("Interrupted by a signal\n");
						continue;
						}
						else
						{
						printf("select error\n");
						sleep(5);
						}
					}
					else
					{

					if(FD_ISSET(csf,&rfds1))
					{

						iovrecv[0].iov_base = &recvhdr;
						iovrecv[0].iov_len = sizeof(struct hdr);
						//iovrecv[1].iov_base = inbuff;
						//iovrecv[1].iov_len = inbytes;

					recval1=readv(csf,iovrecv,2);


					if (recval1 < 0)
						perror("readv1 :");
						else
						{
							/*wndsize=recvhdr.slwnd;
							timest=recvhdr.ts;
							ackno=recvhdr.ack;*/
							wndsize=((struct hdr *)iovrecv[0].iov_base)->slwnd;
							timest=((struct hdr *)iovrecv[0].iov_base)->ts;
							ackno=((struct hdr *)iovrecv[0].iov_base)->ack;

							printf("Received the ackowledgement number %d and window size %d\n",((struct hdr *)iovrecv[0].iov_base)->ack,wndsize);
							if(wndsize==0)
								flag1=1;
							/*if(sigsetjmp(jmpbuf,1)!=0)
							{
								if(rtt_timeout(&rttinfo)<0)
								{
									 err_msg("The client is not responding, so transimission is being stopped\n");
									 rttinit=0;
									 return -1;

								}
								sendag=1;
								goto timer;
							}*/

							//heads=gethead(&h);
							//if(((struct hdr *)iovrecv[0].iov_base)->ack>=heads)


							if(((struct hdr *)iovrecv[0].iov_base)->ack>=((struct hdr *)h->iovs[0].iov_base)->seq)
							{
								counter=0;
								printf("Received time stamp is %u\n",recvhdr.ts);
								if(recvhdr.ts==((struct hdr *)h->iovs[0].iov_base)->ts)
								{
									rtt_stop(&rttinfo,rtt_ts(&rttinfo)-((struct hdr *)h->iovs[0].iov_base)->ts);  //y not recv.hdr;
									printf("The new RTO  value is %d\n",rttinfo.rtt_rto);
									if(sendhdr.islast==1 && recvhdr.islast==1)
									{
									printf("File transfer completed successfully and all acknowledgements received,thus exiting\n");
									freepacket(&h,iovrecv);
									fclose(fp);
									exit(0);
									}
									//rtt_newpack(&rttinfo);
								}
								else
								{
									if(sendhdr.islast==1 && recvhdr.islast==1)
									{
									printf("File transfer completed successfully and all acknowledgements received,thus exiting\n");
									freepacket(&h,iovrecv);
									fclose(fp);
									exit(0);
									}
								}
								if(cong.cwnd<cong.ssthreshold)
								{
									printf("As the cwnd is smaller than the ssthreshold,cwnd increases multiplicatively\n");
								inc=(((struct hdr *)iovrecv[0].iov_base)->ack)-(((struct hdr *)h->iovs[0].iov_base)->seq)+1;
								cong.cwnd=cong.cwnd+1;
								printf("The new congestion window is %d\n",(int)floor(cong.cwnd));
								}
								else if(cong.cwnd>=cong.ssthreshold)
								{
									printf("As the cwnd is greater than the ssthreshold,cwnd starts increasing linearly\n");
								cong.cwnd=cong.cwnd+1/cong.cwnd;
								printf("The new congestion window size is %d\n",(int)floor(cong.cwnd));
								}

								freepacket(&h,iovrecv);
								//counter=0;
								//display(&h);
								printf("after freeing the packet\n");
								if(((struct hdr *)iovrecv[0].iov_base)->ack==lastseq)
													{

														printf("inside last seq\n");
														setitimer(ITIMER_REAL, &zero_val,0);
														if(sendhdr.islast!=1)
														{
															printf("checking if entering islast loop\n");
															//if(wndsize<maxwnd)
															if(wndsize<=tempwnd)
															{
																maxwnd=wndsize;
																//tempwnd=maxwnd;
																printf("New maxwnd is %d\n",maxwnd);
															}

															printf("after setting maxwnd %d\n",maxwnd);
														/*	else
															{
																if(wndsize>ser.window_size)
																	maxwnd=ser.window_size;
																else
																	maxwnd=wndsize;
															}*/
															if(maxwnd>0)
															{
																printf("after setting maxwnd %d\n",maxwnd);
																goto again;
															}
															else if(maxwnd==0)
															{
																rc=0;
																printf("after setting maxwnd %d\n",maxwnd);
																goto wndprb;
															}
														}
														else
														{
															printf("File transfer completed successfully and all acknowledgements received,thus exiting\n");
															freepacket(&h,iovrecv);
															fclose(fp);
															exit(0);
														}
													}


								if(sendhdr.islast==1)
								{
									//printf("File transfer completed successfully\n");
									//fclose(fp);
									goto waitread;

									//exit(0);
								}

								rtt_newpack(&rttinfo);
							}

							//else if(((struct hdr *)iovrecv[0].iov_base)->ack<heads)
							else if(((struct hdr *)iovrecv[0].iov_base)->ack<((struct hdr *)h->iovs[0].iov_base)->seq)
							{

								counter++;
								printf("count is %d\n",counter);
								if(counter==3)
								{
									cong.ssthreshold=maxi(cong.cwnd/2,2);
									cong.cwnd=maxi(cong.cwnd/2,1);

									akno=((struct hdr *)iovrecv[0].iov_base)->ack;
									//akno=((struct hdr *)iovrecv[0].iov_base)->ack;
									//akno=(((struct hdr *)h->iovs[0].iov_base)->seq);
									//((struct hdr *)iovsend[0].iov_base)->seq=akno+1;
									printf("Received an acknowledment %d three times  \n",((struct hdr *)iovrecv[0].iov_base)->ack);
									printf("The new values for cwnd and ssthrshold are %d and %d\n",(int)floor(cong.cwnd),cong.ssthreshold);
									getpacket(&h,akno+1,gp);
									printf("Resending the packet with sequence number %d\n",(((struct hdr *)iovrecv[0].iov_base)->ack)+1);
									sendhdr.ts=rtt_ts(&rttinfo);
									printf("The time stamp sent is %u\n",sendhdr.ts);
									w=writev(csf,gp,2);
									counter=0;
									rtt_newpack(&rttinfo);
								}


								}
						}

					/*if(((struct hdr *)iovrecv[0].iov_base)->ack==lastseq)		//or if islast==1
					{
						setitimer(ITIMER_REAL, &zero_val,0);
							if(wndsize<maxwnd)
							{
								maxwnd=wndsize;
							}
							if(maxwnd>0)
							goto again;
					}*/

					}//close isset
					else
						goto waitread;
					} //close select else
					goto waitread;


							//goto again;
wndprb:

if(maxwnd==0)
{

	FD_ZERO(&rfds2);
						FD_SET(csf, &rfds2);
						rc++;
						if(rc>12)
						{
							printf("The file transmission has been stopped as the client did not respond to 12 repeated requests to update its sliding window size\n");
							exit(1);
						}
						tv1.tv_sec = 5;
						tv1.tv_usec = 0;
						//sendhdr.seq++;
						sendhdr.islast=2;
						sendhdr.ack=0;
						sendhdr.ts=sendhdr.ts=rtt_ts(&rttinfo);
						sendhdr.slwnd=ser.window_size;
						memset(buffer1,0,outbytes);
						iovsend[0].iov_base = &sendhdr;
						iovsend[0].iov_len = sizeof(struct hdr);

						iovsend[1].iov_base = (char *) buffer1;
						iovsend[1].iov_len = outbytes;
						w=writev(csf,iovsend,2);
						printf("Sending probe packet to find out client's updated sliding window size to start re-transmission of the file\n");
						if ((retval2 = select(csf + 1, &rfds2, NULL, NULL, &tv1))
								< 0) {
							if (errno == EINTR) {
								printf("Interrupted by a signal\n");
								continue;
							} else {
								printf("select error\n");
								sleep(5);
							}

						}
						else
						{
							if(FD_ISSET(csf,&rfds2))
							{
								printf("insisde isset for probe\n");
								iovrecv[0].iov_base = &recvhdr;
								iovrecv[0].iov_len = sizeof(struct hdr);
								recval2=readv(csf,iovrecv,2);
								if(recvhdr.islast==2)
								{
									prwnd=recvhdr.slwnd;
									printf("%d\n",prwnd);
									if(prwnd>0)
									{
										if(prwnd<=tempwnd)
										{
											printf("inside prwnd<=tempwnd\n");
											maxwnd=prwnd;
										}
										goto again;
									}
									else
									goto wndprb;
								}
								else
									goto wndprb;

							}
							else goto wndprb;
						}
}



					exit(0);



				}


				else
				{
					printf("The process id of the child is %d\n",pid);
					addclient(&headptr,clieaddr,pid);
					break;
				}
				}//close else


			}		//close isset

		}			//close for -sfds

	}	//close infinite for


}	//close main



void addnode(struct sock **q, int sfd, struct sockaddr_in *ipaddr,
		struct sockaddr_in *ntmask)				//,struct sockaddr_in subaddr)

{
	struct sock *temp, *r;
	char buff[MAXLINE], buff2[MAXLINE];

	temp = *q;
	r = NULL;
	if (temp == NULL ) {
		temp = (struct sock *) malloc(sizeof(struct sock));
		temp->next = NULL;
		temp->sfd = sfd;
		temp->ipaddr = ipaddr;
		temp->netmask = ntmask;
		//temp->subaddr.sin_addr.s_addr=(ntmask->sin_addr.s_addr & ipaddr->sin_addr.s_addr);
		//temp->subaddr=subaddr;
		//temp->subaddr=(ntmask->s_addr & ipaddr->s_addr);
		//printf("ip-address:%s\n",sock_ntop_host((struct in_addr *)ipaddr, sizeof(*ipaddr)));

		//printf("network mask:%s\n",sock_ntop_host((struct in_addr *)ntmask,sizeof(*ntmask)));

		temp->subaddr = (ntmask->sin_addr.s_addr & ipaddr->sin_addr.s_addr);

		printf("\nip-address: %s\n",
				sock_ntop_host((SA *) ipaddr, sizeof(*ipaddr)));

		printf("network mask: %s\n",
				sock_ntop_host((SA *) ntmask, sizeof(*ntmask)));

		printf("subnet: %s\n",
				inet_ntop(AF_INET, &(temp->subaddr), buff, sizeof(buff)));
		/*printf("  IP addr: %s\n Network mask: %s \nSubnet address :%s\n",
		 sock_ntop_host((SA *)ipaddr, sizeof(*ipaddr)),
		 sock_ntop_host((SA *)ntmask,sizeof(*ntmask)),
		 inet_ntop(AF_INET,&(temp->subaddr),buff,sizeof(buff)));//sock_ntop(&temp->subaddr,sizeof(temp->subaddr)));*/
		*q = temp;

	} else {
		while ((temp->next)!= NULL )
			temp = temp->next;
		r = (struct sock *) malloc(sizeof(struct sock));
		temp->next = r;
		r->sfd = sfd;
		r->ipaddr = ipaddr;
		r->netmask = ntmask;
//r->subaddr=(ntmask->s_addr & ipaddr->s_addr);
//printf("ip-address:%s\n",sock_ntop_host((struct in_addr *)ipaddr, sizeof(*ipaddr)));
//printf("network mask:%s\n",sock_ntop_host((struct in_addr *)ntmask,sizeof(*ntmask)));
//printf("subnet:%s\n",inet_ntop(AF_INET,&(r->subaddr),buff2,sizeof(buff2)));

		r->subaddr = (ntmask->sin_addr.s_addr & ipaddr->sin_addr.s_addr);
		printf("\nip-address :%s\n",
				sock_ntop_host((SA *) ipaddr, sizeof(*ipaddr)));
		printf("network mask :%s\n",
				sock_ntop_host((SA *) ntmask, sizeof(*ntmask)));
		printf("subnet: %s\n",
				inet_ntop(AF_INET, &(r->subaddr), buff2, sizeof(buff2)));

//r->subaddr.sin_addr.s_addr=(ntmask->sin_addr.s_addr & ipaddr->sin_addr.s_addr);
		//printf("  IP addr: %s Network mask: %s Subnet address :%s\n",
		 //sock_ntop_host((SA *)ipaddr, sizeof(*ipaddr)),sock_ntop_host((SA *)ntmask,sizeof(*ntmask)),inet_ntop(AF_INET,&(r->subaddr),buff2,sizeof(buff2)));//sock_ntop(&r->subaddr,sizeof(r->subaddr)));
//r->subaddr=subaddr;
		r->next = NULL;
	}
}

//struct sockaddr_in *getaddr(struct sock **q,int sfd,struct in_addr **r)
struct sockaddr_in *getaddr(struct sock **q, int sfd, struct sockaddr_in **r) {
	struct sock *temp;
	temp = *q;
	while (temp != NULL ) {
		if (temp->sfd == sfd) {
			//*r=(struct sockaddr_in *)temp->netmask;
			*r = temp->netmask;

			printf("  network mask: %s\n",
					sock_ntop_host((SA *) (temp->netmask),
							sizeof(*(temp->netmask))));
			return (struct sockaddr_in *) temp->ipaddr;
			//return temp->ipaddr;
		}
		temp = temp->next;
	}

}

int verifyclient(struct client **q, struct sockaddr_in *cliaddr) {
	struct client *temp, *r, *k;
	temp = *q;
	k = *q;
	r = NULL;
	if (temp == NULL ) {
		/*temp = (struct client *) malloc(sizeof(struct client));
		temp->next = NULL;
		temp->caddr = cliaddr->sin_addr.s_addr;
		temp->port = cliaddr->sin_port;
		*q = temp;*/
		return 1;

	} else {
		while (k != NULL ) {
			if (temp->caddr == cliaddr->sin_addr.s_addr
					&& temp->port == cliaddr->sin_port)
				return 0;

			k = k->next;
		}
		/*while (temp->next != NULL )
			temp = temp->next;
		r = (struct client *) malloc(sizeof(struct client));
		temp->next = r;
		r->caddr = cliaddr->sin_addr.s_addr;
		r->port = cliaddr->sin_port;
		r->next = NULL;*/
		return 1;

	}

}


void addclient(struct client **q,struct sockaddr_in *cliaddr,pid_t pid)

{
	struct client *temp, *r;// *k;
		temp = *q;
		//k = *q;
		r = NULL;
		if (temp == NULL ) {
			temp = (struct client *) malloc(sizeof(struct client));
			temp->next = NULL;
			temp->caddr = cliaddr->sin_addr.s_addr;
			temp->port = cliaddr->sin_port;
			temp->tid=pid;

			*q = temp;
			//return 1;

		} else {
			/*while (k != NULL ) {
				if (temp->caddr == cliaddr->sin_addr.s_addr
						&& temp->port == cliaddr->sin_port)
					return 0;

				k = k->next;
			}*/
			while (temp->next != NULL )
				temp = temp->next;
			r = (struct client *) malloc(sizeof(struct client));
			temp->next = r;
			r->caddr = cliaddr->sin_addr.s_addr;
			r->port = cliaddr->sin_port;
			r->tid=pid;
			r->next = NULL;
			//return 1;

		}

	}

void addpacket(struct slwndow **q, struct iovec *p) {
	struct slwndow *temp, *r;
		struct hdr *temphdr;//*temphdr1;
		char *bu;
		temphdr=NULL;
		//temphdr1=NULL;
		temp = NULL;
		temp = *q;
		/*bzero(temphdr,sizeof(struct hdr));
		temp->iovs[0].iov_base=temphdr;
		memcpy(temp->iovs[0].iov_base,(*p).iov_base,sizeof(struct hdr));
		temphdr1=(struct hdr *)malloc(sizeof(struct hdr));
		bzero(temphdr1,sizeof(struct hdr));
		temp->iovs[1].iov_base=temphdr1;
		memcpy(temp->iovs[1].iov_base,(*(p+1)).iov_base,sizeof(struct hdr));*/
		if (temp == NULL ) {
			temp = (struct slwndow *) malloc(sizeof(struct slwndow));
			temphdr=(struct hdr *)malloc(sizeof(struct hdr));
			bzero(temphdr,sizeof(struct hdr));
			temp->iovs[0].iov_base=temphdr;
			memcpy(temp->iovs[0].iov_base,p->iov_base,sizeof(struct hdr));
			(temp->iovs[0]).iov_len = (*p).iov_len;
			bu=(char *)malloc((p+1)->iov_len);

			printf("length is %d\n",(p+1)->iov_len);
			//bzero(bu,sizeof(buffer));
			bzero(bu,(p+1)->iov_len);
			temp->iovs[1].iov_base=bu;
			//printf("r malloc %d\n",(p+1)->iov_len);//,((struct hdr *)temp->iovs[0].iov_base)->seq);
			memcpy(temp->iovs[1].iov_base,(p+1)->iov_base,((p+1)->iov_len));
			(temp->iovs[1]).iov_len = (*(p + 1)).iov_len;
			/*//printf("temp seq no is %d\n",((struct hdr *)temp->iovs[0].iov_base)->seq);
			//printf("temp daat is %s\n",(char *)temp->iovs[1].iov_base);
			printf("temp length in add packet is %d\n",(temp->iovs[0]).iov_len);
			printf("temp1 length in add packet is %d\n",(temp->iovs[1]).iov_len);*/
			//printf("after malloc %d\n",((struct hdr *)temp->iovs[0].iov_base)->seq);
			/*(temp->iovs[0]).iov_base = (*p).iov_base;
			(temp->iovs[0]).iov_len = (*p).iov_len;
			(temp->iovs[1]).iov_base = (*(p + 1)).iov_base;
			(temp->iovs[1]).iov_len = (*(p + 1)).iov_len;*/
			temp->next = NULL;
			*q = temp;



		} else {
			while ((temp->next) != NULL )
				temp = temp->next;


			r = (struct slwndow *) malloc(sizeof(struct slwndow));

			temphdr=(struct hdr *)malloc(sizeof(struct hdr));
			bzero(temphdr,sizeof(struct hdr));
			r->iovs[0].iov_base=temphdr;
			memcpy(r->iovs[0].iov_base,p->iov_base,sizeof(struct hdr));
			(r->iovs[0]).iov_len = (*p).iov_len;
			bu=(char *)malloc((p+1)->iov_len);
			bzero(bu,(p+1)->iov_len);
			r->iovs[1].iov_base=bu;
			memcpy(r->iovs[1].iov_base,(p+1)->iov_base,((p+1)->iov_len));
			(temp->iovs[1]).iov_len = (*(p + 1)).iov_len;
			/*(r->iovs[0]).iov_base = (*p).iov_base;
			(r->iovs[0]).iov_len = (*p).iov_len;
			(r->iovs[1]).iov_base = (*(p + 1)).iov_base;
			(r->iovs[1]).iov_len = (*(p + 1)).iov_len;*/
			temp->next = r;
			r->next = NULL;

		}
}
void display(struct slwndow **q)
{
	struct slwndow *temp;
	temp=NULL;
	temp=*q;
	while(temp!=NULL)
	{
		printf("%d\n",((struct hdr *)temp->iovs[0].iov_base)->seq);
		temp=temp->next;
	}
}

void freepacket(struct slwndow **q, struct iovec *p) {
	struct slwndow *temp, *r,*k,*t;
	r = NULL;
	temp = *q;
	k=*q;
	while (temp != NULL ) {
		if (((struct hdr *) temp->iovs[0].iov_base)->seq
				== ((struct hdr *) (*p).iov_base)->ack)
			/*if (((struct hdr *) temp->iovs[0].iov_base)->seq
							==1)*/
		{
			if(temp==(*q))
			{
			r = temp->next;
			free(temp);
			(*q) = r;
			break;
		} else {
			r = temp->next;
			(*q)=r;
			while((k->next)!=temp)
			{
				t=k;
				k=k->next;
				free(t);

			}
			free(temp);

			break;
		}



	}
temp=temp->next;
}
}


void getpacket(struct slwndow **q,int ackno,struct iovec *g)
{
	struct slwndow *temp;
		temp = *q;


		(g)->iov_base=NULL;
		(g)->iov_len=0;
		((g+1))->iov_base=NULL;
		((g+1))->iov_len=0;
		while (temp != NULL )
		{
			if(((struct hdr *)temp->iovs[0].iov_base)->seq==ackno)
		{
			(*g).iov_base=temp->iovs[0].iov_base;
			(*g).iov_len=temp->iovs[0].iov_len;
			(*(g+1)).iov_base=temp->iovs[1].iov_base;
			(*(g+1)).iov_len=temp->iovs[1].iov_len;
			break;
		}
temp=temp->next;
		}

}

void gethead(struct slwndow **q,struct iovec *l)
{
	struct slwndow *temp;
	temp=*q;
	(l)->iov_base=NULL;
			(l)->iov_len=0;
			((l+1))->iov_base=NULL;
			((l+1))->iov_len=0;
			(*l).iov_base=temp->iovs[0].iov_base;
			(*l).iov_len=temp->iovs[0].iov_len;
			(*(l+1)).iov_base=temp->iovs[1].iov_base;
			(*(l+1)).iov_len=temp->iovs[1].iov_len;

}



int length(struct slwndow **q)
	{
		struct slwndow *temp;
				temp=*q;

				int a=0;
				while(temp!=NULL)
				{
					a++;
					temp=temp->next;
				}

				return a;

	}
/*struct hdr * gethead(struct slwndow **q)
{
	struct slwndow *temp;
	temp=*q;
	if(temp!=NULL)
		return ((struct hdr *)temp->iovs[0].iov_base)->seq;
}*/

void deleteclient(struct client **q,pid_t pid)
{
	struct client *temp, *r,*k;
		r = NULL;
		temp = *q;
		k=*q;
		while (temp != NULL ) {
			if(temp->tid==pid) {
				if(temp==(*q))
				{
					*q=temp->next;
					free(temp);
					break;
				}
				else
				{
					while(k->next!=temp)
					{
						k=k->next;
					}
					r=temp->next;
					k->next=r;
					free(temp);
					break;
				}
			}
			temp=temp->next;
		}
}

int maxi(int a,int b)
{
	if(a>b)
		return a;
	else return b;
}



static void sig_alrm(int signo) {
	printf("inside sig_alrm\n");
	siglongjmp(jmpbuf, 1);
}

void sig_chld(int signo) {
	struct client *hptr=NULL;
	pid_t pid;
	int stat;
	pid = waitpid(-1, &stat, WNOHANG);
	deleteclient(&headptr,pid);
	printf("child %d terminated\n", pid);

	return;
}



