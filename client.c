#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<sys/types.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/select.h>
#include "unpifiplus.h"
#include<pthread.h>
#include<math.h>
//extern struct ifi_info *Get_ifi_info_plus(int family, int doaliases);
//extern        void      free_ifi_info_plus(struct ifi_info *ifihead);




//int acknwld=0;
//int countnode=0;

pthread_mutex_t countnode_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t acknwld_mutex=PTHREAD_MUTEX_INITIALIZER;

struct node
	{
		struct sockaddr_in *ip,*nm;
	//	long clsub,srsub,lpsub;
		struct node *next;
	};

	struct node *head;



	struct client_params {

		char serv_addr[100];
		int portno;
		char file_name[20];
		int window_size;
		int rand_num;
		float prob;
		int mean;

	} cpr;

	void delete();

//void addnode(struct node **q,struct sockaddr_in *ip,struct sockaddr_in *nm);
	static struct hdr
	{
		uint32_t seq;
		uint32_t ts;
		int slwnd;
		int isack;
		int islast;
	} sheader,rheader;

	struct slwnd
	{
		struct iovec iov[2];
		struct slwnd *next;
	};

	struct iovec iov[2];

	struct slwnd *h;
	//h=NULL; // find out why?
	int a=0;

	char *buff;

int recentack=0;
int track=0;
	void addhead()
	{

		struct slwnd *temp,*r=NULL,*fr=NULL,*dup=NULL;
		struct hdr *temphdr;
		char *buffr;
		buffr=NULL;
		temphdr=NULL;
	//	pthread_mutex_lock(&countnode_mutex);
		dup=h;
		temp=h;
	//	pthread_mutex_unlock(&countnode_mutex);
		//if(temp!=NULL)


		if(temp==NULL)
		{
		


//		printf("this is in the add function %d \n",((struct hdr *)(iov[0].iov_base))->seq);

		//printf("this sucks \n");
			//pthread_mutex_lock(&countnode_mutex);
			temp=(struct slwnd *)malloc(sizeof(struct slwnd));
			bzero(temp, sizeof(struct slwnd));
			//printf("this sucks \n");
			temphdr = (struct hdr *)malloc(sizeof(struct hdr));
			bzero(temphdr, sizeof(struct hdr));
			//printf("this sucks \n");
			temp->iov[0].iov_base = temphdr;
			//printf("this sucks \n");

	//		printf("after malloc %d \n",((struct hdr *)(((temp)->iov[0]).iov_base))->seq);
			//printf("this sucks \n");
			//(temp->iov[0]).iov_base=iov[0].iov_base;
			memcpy(temp->iov[0].iov_base, iov[0].iov_base, sizeof(struct hdr));
			//((struct hdr *)(((temp)->iov[0]).iov_base))->seq =5;//((struct hdr *)(iov[0].iov_base))->seq;
			(temp->iov[0]).iov_len=iov[0].iov_len;
			//pthread_mutex_unlock(&countnode_mutex);
			//printf("this sucks 4\n");
			//(temp->iov[1]).iov_base=iov[1].iov_base;
			//pthread_mutex_lock(&countnode_mutex);
			buffr = (char *)malloc(iov[1].iov_len);
	//		printf("this sucks 5\n");
			bzero(buffr,iov[1].iov_len);
			temp->iov[1].iov_base=buffr;
			printf("iovlen size %d \n",iov[1].iov_len);
			//printf("this sucks 5\n");
		//	printf("this sucks 5\n");
			memcpy(temp->iov[1].iov_base, iov[1].iov_base, iov[1].iov_len);
			//printf("this sucks 5\n");
			(temp->iov[1]).iov_len=iov[1].iov_len;




			if(track>=((struct hdr *)(((temp)->iov[0]).iov_base))->seq)
			{
			return;
			}
					

					temp->next=NULL;
					pthread_mutex_lock(&countnode_mutex);
					h=temp;
					pthread_mutex_unlock(&countnode_mutex);


			//		printf("buffer is %s\n",(char *)((temp)->iov[1]).iov_base);
				//pthread_mutex_unlock(&countnode_mutex);
				//countnode++;
				//printf("countnode %d \n",countnode);
				//printf("adding head %d /n",((struct hdr *)(((temp)->iov[0]).iov_base))->seq);
				//pthread_mutex_unlock(&countnode_mutex);
		}

		else
		{
			//printf("head not null, 2nd node %d \n",((struct hdr *)((h)->iov[0].iov_base))->seq);

			//pthread_mutex_lock(&countnode_mutex);
			r=(struct slwnd *)malloc(sizeof(struct slwnd));
			bzero(r,sizeof(struct slwnd));
			temphdr = (struct hdr *)malloc(sizeof(struct hdr));
			bzero(temphdr, sizeof(struct hdr));
			r->iov[0].iov_base = temphdr;
			memcpy(r->iov[0].iov_base, iov[0].iov_base, sizeof(struct hdr));
			(r->iov[0]).iov_len=iov[0].iov_len;
			buffr = (char *)malloc(iov[1].iov_len);
			bzero(buffr,iov[1].iov_len);
			r->iov[1].iov_base=buffr;
			memcpy(r->iov[1].iov_base, iov[1].iov_base, iov[1].iov_len);
			(r->iov[1]).iov_len=iov[1].iov_len;

			//(r->iov[0]).iov_base=iov[0].iov_base;
			//(r->iov[1]).iov_base=iov[1].iov_base;
			//pthread_mutex_unlock(&countnode_mutex);


			//					pthread_mutex_lock(&countnode_mutex);
						//countnode++;
						//printf("countnode 1 %d \n",countnode);
		//				pthread_mutex_unlock(&countnode_mutex);




						while(dup!=NULL)
						{
//							printf(" dup : %d\n", ((struct hdr *)(((h)->iov[0]).iov_base))->seq);
	//						printf(" r : %d\n", ((struct hdr *)(((r)->iov[0]).iov_base))->seq);
							if(((struct hdr *)(((dup)->iov[0]).iov_base))->seq==((struct hdr *)(((r)->iov[0]).iov_base))->seq || track>=((struct hdr *)(((r)->iov[0]).iov_base))->seq)
								{
								//free(r);
			//					pthread_mutex_lock(&countnode_mutex);
							//	countnode--;
							//	printf("countnode 2 %d \n",countnode);
							//	printf("duplicate found\n");
				//				pthread_mutex_unlock(&countnode_mutex);
								return;// Point to remember : should add countnode-- as both duplicates are added.
								}
								dup=dup->next;
						}

							if(h!=NULL)
							{
//								pthread_mutex_lock(&countnode_mutex);
								if(((struct hdr *)(((temp)->iov[0]).iov_base))->seq>((struct hdr *)(((r)->iov[0]).iov_base))->seq)
								{
		//							printf("it is here 1\n");
									r->next=temp;
									pthread_mutex_lock(&countnode_mutex);
									h=r;
									pthread_mutex_unlock(&countnode_mutex);
						//			printf("buffer is %s\n",(char *)((temp)->iov[1]).iov_base);
						//			printf("should not come here \n");
									//printf("buffer is %s\n",(char *)((r)->iov[1]).iov_base);
					//				pthread_mutex_unlock(&countnode_mutex);
									return;
							//		countnode++;
								//	printf("count node two %d /n",countnode);
								}
							}

			while(temp!=NULL)
			{
				//pthread_mutex_lock(&countnode_mutex);
			if(((struct hdr *)(((temp)->iov[0]).iov_base))->seq<((struct hdr *)(((r)->iov[0]).iov_base))->seq)
			{
			//	printf("it is here 2\n");
				fr=temp;
			//	printf("before operation 1 \n");
				if(temp->next==NULL)
				{
					//printf("after operation1\n");
					temp->next=r;
					r->next=NULL;
					//printf("buffer is %s\n",(char *)((r)->iov[1]).iov_base);
					//printf("buffer is %s\n",(char *)((temp)->iov[1]).iov_base);
					//printf("adding in if %d /n",((struct hdr *)(((r)->iov[0]).iov_base))->seq);
					//countnode++;
					//printf("count node three %d /n",countnode);
			//		pthread_mutex_unlock(&countnode_mutex);
					break;
				}
				temp=temp->next;
				continue;
			}

			//else if(((struct hdr *)(((temp)->iov[0]).iov_base))->seq>((struct hdr *)(((r)->iov[0]).iov_base))->seq)

			else
			{
				//printf("it is here 3\n");
				//if(fr==NULL)
				//printf("this is after null near fr\n");
				fr->next=r;
				r->next=temp;
				//printf("buffer is %s\n",(char *)((r)->iov[1]).iov_base);
				//printf("buffer is %s\n",(char *)((temp)->iov[1]).iov_base);
				//countnode++;
			//	printf("adding in else %d /n",((struct hdr *)(((r)->iov[0]).iov_base))->seq);
		//		pthread_mutex_unlock(&countnode_mutex);
				break;
             }

			}
		}

	}




	

	void sendack()
	{

		struct slwnd *temp;
		temp=h;
		int i=ack();
		int j=1;

		if(i==0)
		{
			pthread_mutex_lock(&countnode_mutex);
			recentack=track;
			pthread_mutex_unlock(&countnode_mutex);
			return;
		}
/*

	if(((struct hdr *)(((temp)->iov[0]).iov_base))->seq!=1 && recentack==0)
	{
		pthread_mutex_lock(&countnode_mutex);
		recentack=0;
		pthread_mutex_unlock(&countnode_mutex);
		return;
	}
*/

	while((temp->next)!=NULL && j<i)

	{
		temp=temp->next;
		++j;
	}
	pthread_mutex_lock(&countnode_mutex);
	recentack=((struct hdr *)(((temp)->iov[0]).iov_base))->seq;
	pthread_mutex_unlock(&countnode_mutex);
	}







	int length()
	{
		struct slwnd *temp;
				temp=h;
				//temp1=h;
				//pthread_mutex_lock(&countnode_mutex);
				int a=0;
				while(temp!=NULL)
				{
					a++;
					temp=temp->next;
				}
				//pthread_mutex_unlock(&countnode_mutex);
				return a;

	}


	int ack()
	{
		struct slwnd *temp,*fr;
		temp=h;
		fr=NULL;
		int i=0;
		//pthread_mutex_lock(&countnode_mutex);
		int k=length();
		//pthread_mutex_unlock(&countnode_mutex);
		if(k==1)
       {
    	   return (i+1);
       }
		if(k==0)
			return 0;

		while((temp->next)!=NULL)
		{
			//pthread_mutex_lock(&countnode_mutex);
			fr=temp;
			temp=temp->next;
			if(((struct hdr *)(((temp)->iov[0]).iov_base))->seq -((struct hdr *)(((fr)->iov[0]).iov_base))->seq==1)
			{
              ++i;
			}
			else
			{
			//	pthread_mutex_unlock(&countnode_mutex);
				break;
			}

		}
		//pthread_mutex_unlock(&countnode_mutex);
		return (i+1);
	}




	/*
	void ack()
	{
		struct slwnd *temp;
		temp=h;
		//temp1=h;
		//temp=temp->next;

		//printf("value of a is %d \n",a);
		while(temp!=NULL)
		{
			//printf("value of a is %d \n",a);
			if(((struct hdr *)(((temp)->iov[0]).iov_base))->seq==((struct hdr *)(((h)->iov[0]).iov_base))->seq+a)
				{
				temp=temp->next;
				printf("value of a is %d \n",a);
				a++;
				acknwld=a;
				continue;
				}

			else
			{
				acknwld=a;
				break;
			}
		}
	}
*/








/*

	void ack()
	{
		int a=0;
		struct slwnd *temp;
		temp=h;

		//pthread_mutex_lock(&acknwld_mutex);
		while(temp!=NULL)
		{
			++a;
		//	printf("%d \n ",a);
			if(((struct hdr *)(((temp)->iov[0]).iov_base))->seq==a)
			{

		//		printf("%d \n ",a);
				//pthread_mutex_lock(&acknwld_mutex);
				acknwld=a;
				//pthread_mutex_unlock(&acknwld_mutex);
				temp=temp->next;
				continue;
			}
			else
			{
			//printf("%d /n ",a);
				//pthread_mutex_lock(&acknwld_mutex);
				acknwld=a;
				//pthread_mutex_unlock(&acknwld_mutex);
				break;
          //  printf("this is the acknwlgd %d \n",acknwld);
			}
		}

		//pthread_mutex_unlock(&acknwld_mutex);
	}
*/


int flagforthread=0;

	void *display(void *args)
	{
		FILE  *fp1;
		fp1=fopen("ck.txt","w");
		//printf("calling thread \n");
		while(1)
		{

		float p= (float)drand48();
		double v=(-1)*cpr.mean*log(p)*1000;
		//double sleeptime=log(v);

//		printf("the sleep time is %f \n",v);

		//log(drand48())*meantime*(-1)

			usleep(v);
			//pthread_mutex_lock(&countnode_mutex);

			//printf("calling thread \n");
		struct slwnd *temp,*temp1;
		temp=h;

		if(h==NULL)
			continue;

		if (recentack<((struct hdr *)(((temp)->iov[0]).iov_base))->seq)
			continue;

		int i=1;
		int j=ack();
//		printf("the value of j in display is %d",j);


		while(temp!=NULL && i<=j)
		{
				//printf("header is %d\n",((struct hdr *)(((temp)->iov[0]).iov_base))->seq);
				printf("the contents of the packet %d are : \n",((struct hdr *)(((temp)->iov[0]).iov_base))->seq);
				printf("******************************************************************************\n");
				//printf("buffer is %s\n",(char *)((temp)->iov[1]).iov_base);
				printf("the size of the packet is %d ",(temp->iov[1]).iov_len);
				fwrite((char *)((temp)->iov[1]).iov_base,1,temp->iov[1].iov_len ,stdout);
				printf("*******************************************************************************\n");
				//			fwrite((char *)((temp)->iov[1]).iov_base,1,temp->iov[1].iov_len ,fp1);

				track=((struct hdr *)(((temp)->iov[0]).iov_base))->seq;
						//printf("buffer is %s \n",(char *)((temp)->iov[1]).iov_base);
				//temp1=temp;

								if(((struct hdr *)(((temp)->iov[0]).iov_base))->islast==1 && track==((struct hdr *)(((temp)->iov[0]).iov_base))->seq)
								{
									//printf("deleting the last node \n");
									//delete();
									//printf("deleted the last node so now exiting \n");
									flagforthread=1;
									goto aswe;
									//fclose(fp1);
									//break;
								}

								temp=temp->next;
								i++;

								//h=temp;
								//free(temp1);
						}

						delete();

						}
						aswe:fclose(fp1);
						//printf("flag for thread %d\n",flagforthread);
						printf("File successfully printed\n");
					}
/*
	void *display(void *args)
	{

		int ack;
		printf("this is in the display\n");
		//ack=acknwld;
		//pthread_mutex_lock(&acknwld_mutex);
		struct slwnd *temp;


		while(1)
		{

	//	pthread_mutex_lock(&countnode_mutex);
		temp=h;
		if(acknwld>0)
		{
			if(temp==NULL)
			printf("this is a " );

		while(temp!=NULL && (((struct hdr *)(((temp)->iov[0]).iov_base))->seq)<=acknwld)
		{
			printf("header is %d\n",((struct hdr *)(((temp)->iov[0]).iov_base))->seq);
			printf("buffer is %s\n",((temp)->iov[1]).iov_base);
            temp=temp->next;
		}
		delete();
		//pthread_mutex_unlock(&countnode_mutex);
		//return (NULL);
		}
		//else
		//return (NULL);
		//printf("%d sleeping \n",acknwld);
		//sleep(2);
		}
	}

*/


	void delete()

	{
//		int ack;
		//ack=acknwld;
		struct slwnd *temp;
		temp=NULL;
		int i=1;
		int j=ack();
		if(h==NULL)
		printf("this is null in delete function \n");
		while(h!=NULL && i<=j)

		{
		temp=h;			
		//printf("coming in to the delete function loop %d\n",((struct hdr *)(((temp)->iov[0]).iov_base))->seq);
               free(temp->iov[0].iov_base);
                free(temp->iov[1].iov_base);
              //  free()
                free(temp);
                i++;
                pthread_mutex_lock(&countnode_mutex);
                h=h->next;
                pthread_mutex_unlock(&countnode_mutex);
		}

	}





int main(int argc, char **argv)

{


	/*if (argc != 3)
		err_quit("usage: prifinfo_plus <inet4|inet6> <doaliases>");
*/
	h=NULL;
	int sockfd;
	FILE *fp;
	struct sockaddr_in cliaddr,servaddr;
	char clientaddr[100],serveraddr[100];
	char abc[100],pqr[100],xyz[100];
	socklen_t len,len1;
	struct sockaddr_in sbna,sbna1;
	struct sockaddr_in *ipaddr,*netmask;
	struct sockaddr_in *ipaddr1,*ipaddr2,*ipaddr3,*ipaddr4;
	int a,b,c;
	int y;

	memset((void*)&rheader,0,sizeof(rheader));
	memset((void*)&sheader,0,sizeof(sheader));
	struct in_addr ipadd,ipadd1,ipaddlb,ntmsk,subadd,subadd1,subaddlb;

	//for threads
	pthread_t cons;


	//int nbytes=sizeof(buff);
	int nbytes,nbytes1,nbytes2;
	nbytes=512-sizeof(sheader);
	//printf("nbytes %d \n",nbytes);

	bzero(&sheader,sizeof(sheader));
	bzero(&rheader,sizeof(rheader));

	//printf("this is in thewedsadsas display\n");


	 fd_set rfds,rfds1;
	           struct timeval tv;
	           int retval,retval1;
	           int maxfd,maxfd1;
	           int recval,recval1;
	           char port[100],port1[100];
	           int count=0,count1=0,count2=0;
	           char wsize[100];
	char lbaddr[10]={"127.0.0.1"};
	int optval=1;


	bzero(&cpr,sizeof(cpr));
	fp = fopen("client.in", "r");
	if (fp == NULL )
		perror("Error opening file");
	else {
		while (feof(fp) == 0) {
			fscanf(fp, "%s" "%d" "%s" "%d" "%d" "%f" "%d", cpr.serv_addr,
					&cpr.portno, cpr.file_name, &cpr.window_size, &cpr.rand_num,
					&cpr.prob, &cpr.mean);
		}
	}

	printf("the server address is: %s \n", cpr.serv_addr);
	printf("the server port number is: %d \n", cpr.portno);
	printf("the file to be transfered is: %s \n", cpr.file_name);
	printf("the window size is: %d \n", cpr.window_size);
	printf("the random seed number is: %d \n", cpr.rand_num);
	printf("the probability is: %f \n", cpr.prob);
	printf("the mean value is: %d \n", cpr.mean);

	srand48(cpr.rand_num);

	float randomnum,rax;

	struct ifi_info *ifi, *ifihead;
	struct sockaddr *sa,*sa1,*sa2,*sa3;
	u_char *ptr;
	int i, family, doaliases;

	//bzero(&buff,sizeof(buff));



	pthread_create(&cons,NULL,&display,NULL);



//	void addnode(struct node *q,struct sockaddr_in *ip,struct sockaddr_in *nm);

	int flag=0,flag1=0,flag2=0;
	unsigned long maxntmsk=0;
	char buf1[100];
	bzero(&buf1,sizeof(buf1));

	ipaddr1 = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	ipaddr2 = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	bzero(ipaddr1,sizeof(ipaddr1));

	inet_pton(AF_INET,cpr.serv_addr,&ipaddr1->sin_addr);
	/*if(NULL==ipaddr1)
	printf("this is sasasddsasdsaas \n");
	*/
	inet_pton(AF_INET,lbaddr,&ipaddr2->sin_addr);

	if((inet_pton(AF_INET,cpr.serv_addr,&servaddr.sin_addr))<0)
		printf("there is an error in server inet_pton 1 \n");


	//printf("the sadd %lu \n ",ipaddr1->sin_addr.s_addr);

/*
	if (strcmp(argv[1], "inet4") == 0)
		family = AF_INET;

#ifdef	IPv6

	else if (strcmp(argv[1], "inet6") == 0)
	family = AF_INET6;

#endif

	else
		err_quit("invalid <address-family>");
	doaliases = atoi(argv[2]);

*/
	printf("this is the interface info of the system : \n");

	for (ifihead = ifi = Get_ifi_info_plus(AF_INET, 1); ifi != NULL ;
			ifi = ifi->ifi_next)
	{

		printf("*************************************************************\n");
		printf("%s: ", ifi->ifi_name);
		if (ifi->ifi_index != 0)
			printf("(%d) ", ifi->ifi_index);
		printf("<");

		/*INDENT-OFF*/

		if (ifi->ifi_flags & IFF_UP)
			printf("UP ");
		if (ifi->ifi_flags & IFF_BROADCAST)
			printf("BCAST ");
		if (ifi->ifi_flags & IFF_MULTICAST)
			printf("MCAST ");
		if (ifi->ifi_flags & IFF_LOOPBACK)
			printf("LOOP ");
		if (ifi->ifi_flags & IFF_POINTOPOINT)
			printf("P2P ");
		printf(">\n");

		/*INDENT-ON*/

		if ((i = ifi->ifi_hlen) > 0) {
			ptr = ifi->ifi_haddr;
			do {
				printf("%s%x", (i == ifi->ifi_hlen) ? "  " : ":", *ptr++);
			} while (--i > 0);
			printf("\n");
		}
		if (ifi->ifi_mtu != 0)
			printf("  MTU: %d\n", ifi->ifi_mtu);

		if ((sa = ifi->ifi_addr) != NULL )
			printf("  IP addr: %s\n", Sock_ntop_host(sa, sizeof(*sa)));

			//inet_aton(Sock_ntop_host(sa, sizeof(*sa)),&ipadd);

		 ipaddr=(struct sockaddr_in *)ifi->ifi_addr;

//=================== cse 533 Assignment 2 modifications ======================

		if ((sa1 = ifi->ifi_ntmaddr) != NULL )
			printf("  network mask: %s\n", Sock_ntop_host(sa1, sizeof(*sa1)));

//=============================================================================
		//	inet_aton(Sock_ntop_host(sa1, sizeof(*sa1)),&ntmsk);

		netmask=(struct sockaddr_in *)ifi->ifi_ntmaddr;


		//addnode(&head,ipaddr,netmask);

		//	unsigned long nmcmp

	//		sbna.sin_addr.s_addr=(netmask->sin_addr.s_addr & ipaddr->sin_addr.s_addr);

			//inet_aton(cpr.serv_addr, &ipaddr1);
		//	inet_aton(lbaddr,&ipaddr2);

	//		sbna1.sin_addr.s_addr=(netmask->sin_addr.s_addr & ipaddr1->sin_addr.s_addr);


		if ((sa = ifi->ifi_brdaddr) != NULL )
				printf("  broadcast addr: %s\n", Sock_ntop_host(sa, sizeof(*sa)));
		if ((sa = ifi->ifi_dstaddr) != NULL )
			printf("  destination addr: %s\n", Sock_ntop_host(sa, sizeof(*sa)));

		printf("****************************************************************\n");

			unsigned long clisubaddr= (netmask->sin_addr.s_addr) & (ipaddr->sin_addr.s_addr);
		//	printf("clisubaddr %lu \n",clisubaddr);
		//	printf("clisubaddr %s \n",inet_ntop(AF_INET,&(clisubaddr),buf1,sizeof(buf1)));

			unsigned long servsubaddr= (netmask->sin_addr.s_addr) & (ipaddr1->sin_addr.s_addr);
		//	printf("servsubaddr %lu \n",servsubaddr);
		//	printf("servsubaddr %s \n",inet_ntop(AF_INET,&(servsubaddr),buf1,sizeof(buf1)));


			unsigned long lpbksubaddr = (netmask->sin_addr.s_addr) & (ipaddr2->sin_addr.s_addr);
		//	printf("lpbksubaddr %lu \n",lpbksubaddr);
		//	printf("lpbksubaddr %s \n",inet_ntop(AF_INET,&(lpbksubaddr),buf1,sizeof(buf1)));


			unsigned long ntmsk=netmask->sin_addr.s_addr;

			if(clisubaddr==servsubaddr)
					{
						flag=1;
						if(maxntmsk<ntmsk)
						{
							maxntmsk=ntmsk;
							ipaddr3=ipaddr;
						}
						if(ipaddr->sin_addr.s_addr == ipaddr1->sin_addr.s_addr)
							flag=2;
					}
			else
			{
				flag1=1;
				ipaddr4=ipaddr;
			}


		/*

			if(servsubaddr==clisubaddr)
			{
				if(serflag1=1;vsubaddr==lpbksubaddr)
				{
				printf("the server is on the same host and we assign the loop back address for client and server \n");
				inet_pton(AF_INET,"127.0.0.1",&cliaddr);
				inet_pton(AF_INET,"127.0.0.1",&servaddr);
				}
				else
				{

				}

				continue;
			}

			else if()
			{

			}



*/







		/*

				inet_aton(cpr.serv_addr,&ipadd1);
				inet_aton(lbaddr,&ipaddlb);

				subaddlb.s_addr = ipaddlb.s_addr & ntmsk.s_addr;
				long loopbsbaddr=subaddlb.s_addr;
				subadd.s_addr = ipadd.s_addr & ntmsk.s_addr;
				long clinetsbaddr=subadd.s_addr;
				printf("subnet address 1 %s \n", inet_ntoa(subadd));
				subadd1.s_addr= ipadd1.s_addr & ntmsk.s_addr;
				long serversbaddr=subadd1.s_addr;
				printf("subnet address 2 %s \n", inet_ntoa(subadd1));



				if (loopbsbaddr == serversbaddr)
				{
					printf("the is awesome \n");
				}


*/
				/*

				int n =memcmp(subadd.s_addr,subadd1.s_addr,sizeof(subadd.s_addr));

				if (n>0) printf ("'%s' is greater than '%s'.\n",subadd.s_addr,subadd1.s_addr);
				  else if (n<0) printf ("'%s' is less than '%s'.\n",subadd.s_addr,subadd1.s_addr);
				  else printf ("'%s' is the same as '%s'.\n",subadd.s_addr,subadd1.s_addr);
*/





}



	if(flag==2)
	{
		printf("the server and client are on the same host and local so using loop back addresses \n");
		if((inet_pton(AF_INET,lbaddr,&cliaddr.sin_addr))<0)
			printf("there is an error in client inet_pton 1 \n");
		if(	(inet_pton(AF_INET,lbaddr,&servaddr.sin_addr))<0)
			printf("there is an error in server inet_pton 1 \n");
	}

	else if(flag==1)
	{
		printf("the server and client are local to each other but they are not on the same host \n");
		cliaddr.sin_addr=ipaddr3->sin_addr;
		//bzero(&buf1,sizeof(buf1));
	}

	else if(flag1==1)
	{
		printf("the server is not local to the client \n");
		cliaddr.sin_addr.s_addr=ipaddr4->sin_addr.s_addr;
	}

	//printf("%s",abc);

	if(flag==1 || flag==2)
	{
		sockfd=socket(AF_INET,SOCK_DGRAM,0);
		if((setsockopt(sockfd,SOL_SOCKET,SO_DONTROUTE,&optval,sizeof(optval)))<0)
			printf("setsockopt error in client \n");
	}

	else
	{
		sockfd=socket(AF_INET,SOCK_DGRAM,0);
	}

	//bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(cpr.portno);
//	printf("debug 1 \n");
//	Inet_pton(AF_INET,cpr.serv_addr,&servaddr.sin_addr);


	//bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family=AF_INET;
	cliaddr.sin_port=htons(0);
	//printf("servsubaddsdddddddddddddddddddddddddddddddddddddddddddddddddr %s \n",inet_ntop(AF_INET,&cliaddr,buf1,sizeof(buf1)));


	//Inet_pton(AF_INET,clientaddr,&cliaddr.sin_addr);


	if((bind(sockfd,(const struct sockaddr *) &cliaddr,sizeof(cliaddr)))<0)
		printf("bind error on client socket \n");

	len=sizeof(cliaddr);

	if((a=(getsockname(sockfd,(struct sockaddr *) &cliaddr,&len)))<0)
		printf("error in getsockname of the client \n");

	else
	{
	printf("the client address IPclient is  : %s \n ",inet_ntoa(cliaddr.sin_addr));
	printf("the client port number assigned by kernel is : %d \n",ntohs(cliaddr.sin_port));
	}


	if(connect(sockfd,(const struct sockaddr *)&servaddr,sizeof(servaddr))<0)
		printf("connect error in the client \n");


	len1=sizeof(servaddr);

	if((b=(getpeername(sockfd,(struct sockaddr *) &servaddr,&len1)))<0)
			printf("error in getpeername of the client \n");
	else
	{
		printf("the server address that the client is connected to is : %s \n",inet_ntoa(servaddr.sin_addr));
		printf("the server port number that the client is connected to is : %d \n",ntohs(servaddr.sin_port));
	}



				FD_ZERO(&rfds);
				FD_SET(sockfd, &rfds);

				do{
					FD_ZERO(&rfds);
					FD_SET(sockfd, &rfds);

				count++;
				if(count>1 && count<=12)
				printf("the server didn't send any datagram after transmitting filename so retransmitting\n");
				printf("sending the filename as %s \n",cpr.file_name);
//				char bu[100];
				/*bzero(&bu,sizeof(bu));
				strcpy(bu,cpr.file_name);
				printf("sending the filename as %s \n",bu);
*/
//				int xn=write(sockfd,cpr.file_name,sizeof(cpr.file_name));
	//			printf("%d",xn);
				int xn=	send(sockfd,cpr.file_name,sizeof(cpr.file_name),0);
	//			printf("this is after send %d \n",xn);
				//recval=recv(sockfd,port,sizeof(port),0);
		//		recval=read(sockfd,port,sizeof(port));
	//			printf("this is after recv \n");
//				printf("revcd = %d %s",recval,port);
	         //   Wait up to five seconds.

	           tv.tv_sec = 5;
	           tv.tv_usec = 0;
	           maxfd=sockfd+1;
	           retval = select(maxfd, &rfds, NULL, NULL, &tv);

	           // Don’t rely on the value of tv now!

	           if (retval == -1)
	               perror("select()");
	           else
	           {
	        	   if(FD_ISSET(sockfd,&rfds))
	        			   {
	        		   	   	   rax=(float)drand48();
	        		   	   	   if(rax>=cpr.prob)
	        		   	 	 recval=recv(sockfd,port,sizeof(port),0);
	        		   	   	   else
	        		   	   	   {
	        		   	   		   printf("Dropping the ACK for Filename \n");
	        		   	   		   continue;
	        		   	   		   //goto rs;
	        		   	   	   }
	        		   	   	   if(recval<0)
	        		   	 		 printf("recv error in client \n");
	        		   	 	 else
	        		   	 		 //printf("recvd from the server \n %s ",port);
	        		   	 	 break;
	        			   }
	           }

				}while(count<12);


				if(count==12)
				{
					printf("the retransmission time out has passed still no datagram so exiting \n");
					exit(0);
				}

				else
				{
					printf("ACK received from server as port number %s\n",port);
					servaddr.sin_port=htons(atoi(port));


					if(connect(sockfd,(const struct sockaddr *)&servaddr,sizeof(servaddr))<0)
						printf("connect error in the client 2 \n");


					len1=sizeof(servaddr);

					if((b=(getpeername(sockfd,(struct sockaddr *) &servaddr,&len1)))<0)
							printf("error in getpeername of the client 2 \n");
					else
					{
						printf("the server address that the client is now connected after getting portnum to is : %s \n",inet_ntoa(servaddr.sin_addr));
						printf("the server port number that the client is now connected after getting portnum to is  : %d \n",ntohs(servaddr.sin_port));
					}



					FD_ZERO(&rfds);
					FD_SET(sockfd, &rfds);

					bzero(&port, sizeof(port));

					do {
						FD_ZERO(&rfds);
						FD_SET(sockfd, &rfds);

						count1++;
						if (count1 > 1 && count1 <= 12)
							printf("the server didn't send any datagram after transmitting window size so retransmitting\n");
						sprintf(wsize, "%d", cpr.window_size);
						//wsize=cpr.window_size;
						printf("Sending WindowSize %s \n", wsize);
						send(sockfd, wsize, sizeof(wsize), 0);
						//	            Wait up to five seconds.
						bzero(&port, sizeof(port));

						tv.tv_sec = 5;
						tv.tv_usec = 0;
						maxfd = sockfd + 1;
						retval = select(maxfd, &rfds, NULL, NULL, &tv);



						if (retval == -1)
							perror("select()");
						else
						{
							if (FD_ISSET(sockfd,&rfds))
							{
								bzero(&port,sizeof(port));
								rax=(float)drand48();
								//printf("the value of rax in second is %f \n",rax);
								if(rax>=cpr.prob)
								recval = recv(sockfd, port, sizeof(port), 0);
								else
								{
									printf("Dropping the ACK fro window size as rand num  < prob \n");
//									goto ax;
									continue;
								}
								if (recval < 0)
									printf("recv error in client 2 \n");
								else
									break;
							}
						}

					} while (count1 < 12);

					if (count1 == 12)
					{
						printf("the retransmission time out has passed still "
								"no ack from the server after sending the window size so exiting \n");
						exit(0);
					}

					else
					{

//					printf("this is the ack recieved from the server as the port %s \n",port);
						printf("HANDSHAKE COMPLETED \n ");
						printf("Receiving File \n");

					FD_ZERO(&rfds1);
					FD_SET(sockfd, &rfds1);


					//h=NULL; // find out why?


			//		int v=0;

					buff=(char *)malloc(nbytes);

					while(1)
					{
						FD_ZERO(&rfds1);
						FD_SET(sockfd, &rfds1);

						tv.tv_sec = 5;
						tv.tv_usec = 0;

					bzero(buff,nbytes);
					bzero(iov, sizeof(iov));
					iov[0].iov_base = &rheader;
					iov[0].iov_len = sizeof(rheader);
					iov[1].iov_base=buff;
					iov[1].iov_len =nbytes;

					maxfd1 = sockfd + 1;

					if (flagforthread==1)
					{
				//	printf("in again loop********************************************************\n");
					goto again;
					}


					retval1 = select(maxfd, &rfds, NULL, NULL, &tv);
					int l;

					if (flagforthread==1)
					{
					//printf("in again loop********************************************************\n");
					goto again;
					}


					if (retval1 == -1)
						perror("select()");
						else
						{
							if (flagforthread==1)
							{
						//	printf("in again loop********************************************************\n");
							goto again;
							}

							if (FD_ISSET(sockfd,&rfds1))
							{

								if (flagforthread==1)
								{
							//	printf("in again loop********************************************************\n");
								goto again;
								}

								//printf("this is in isset \n");
								recval1 =  readv(sockfd, iov, 2);

								if (recval1 < 0)
									perror("readv1 ");
									else
									{
										randomnum=(float)drand48();
										//printf("random number %f \n",randomnum);

										if(randomnum>=cpr.prob)
										{
									//	printf("incoming node %d \n",rheader.seq);
										//printf("time stamp %d \n",rheader.ts);
											printf("Window Size full Server Sent a Probe Packet \n");
											if(rheader.islast==2) // probe packet doubt
										{
										if(cpr.window_size!=length())
										{
											printf("Server sent a probe packet \n");
										//	pthread_mutex_lock(&countnode_mutex);
											sheader.islast=2;
											sheader.slwnd=cpr.window_size-length();
										//	pthread_mutex_unlock(&countnode_mutex);
											printf("Responding to probe with updated window size as %d",sheader.slwnd);
											iov[0].iov_base =&sheader;
											iov[0].iov_len = sizeof(sheader);
											y = writev(sockfd, iov, 1);
											printf("writing to the server no of bytes :%d\n", y);
											continue;
										}
										continue;
										}
											//printf("this is in the last select \n");
											//pthread_mutex_lock(&countnode_mutex);

										//	printf("the buffer contents are %s \n",buff);

										//	printf("after addhead() %d \n",((struct hdr *)((h)->iov[0].iov_base))->seq);
											if(cpr.window_size!=length())
											{
											//	pthread_mutex_lock(&countnode_mutex);
												addhead();
												sendack();
												//display();
										//		if(rheader.seq==8)
											//		break;
												//sendack();
												//rax=(float)drand48();
												//printf("the radom value for ack is %f s \n",rax);
												if(rax>=cpr.prob)
												{
												sheader.seq=rheader.seq;
												sheader.islast=rheader.islast;
												sheader.isack = recentack;//add two more fields
												sheader.ts=rheader.ts;
												l=length();
												sheader.slwnd = cpr.window_size-l;
											//	pthread_mutex_unlock(&countnode_mutex);
												printf("Sending ACK : %d   Window Size: %d \n",sheader.isack,sheader.slwnd);
												iov[0].iov_base =&sheader;
												iov[0].iov_len = sizeof(sheader);
												y = writev(sockfd, iov, 1);
												//printf("writing to the server no of bytes :%d\n", y);
												}
												else
												{
													printf("Dropping the ACK for Packets as RandomNum:%f is less than Probability\n",rax);
													//printf("the value of length is %d",length());
													continue;
												}
											}

											else
												{
												printf("Rejecting %d packet as window size id full\n",rheader.seq);
												continue;
												}

									//		printf("after addhead() %d \n",((struct hdr *)((h)->iov[0].iov_base))->seq);
											/*if(h==NULL)
											printf("h is null \n");
											if(h==NULL)
											printf("h is null \n");
											printf("ack value : recent %d \n ",recentack);
											printf("the value of length is %d",length());
											printf("the value of continous ack is %d ",ack());
											//display();
											printf("the value of recvd %d and sent %d timestamps are \n",rheader.ts,sheader.ts);
											//if(cpr.window_size!=length())	//add an else case
											printf("%d this is the updated window size \n",sheader.slwnd);
											//printf("%d this is acknwld \n",acknwld);*/
											//pthread_cond_signal(&cond);
										//	pthread_mutex_unlock(&countnode_mutex);

										}
										else
										{
											printf("Dropping Packet as Random Num %f is less than Probability \n",randomnum);
										printf("Dropped Packet : %d ",rheader.seq);
										if(rheader.seq==1 && length()==0)
											continue;
											/*sheader.seq=rheader.seq;
											sheader.islast=rheader.islast;
											sheader.isack = recentack;//add two more fields
											sheader.ts=rheader.ts;
											l=length();
											sheader.slwnd = cpr.window_size-l;
											printf("ack value : recent %d \n ",recentack);
											printf("the value of length is %d",length());
											printf("the value of continous ack is %d ",ack());
											printf("the value of recvd %d and sent %d timestamps are \n",rheader.ts,sheader.ts);
											printf("%d this is the updated window size \n",sheader.slwnd);
										//	pthread_mutex_unlock(&countnode_mutex);
											iov[0].iov_base =&sheader;
											iov[0].iov_len = sizeof(sheader);
											y = writev(sockfd, iov, 1);
											printf("writing to the server no of bytes :%d\n", y);*/
										}
									}
									//break;
							}
						}

					}

				again:	//printf("this is after the reading is completed \n");

					pthread_join(cons,NULL);

						if (close(sockfd))
						{
						perror("close");
						}

				}



			} //end of the first else

	//printf("main thread exiting after file transfer\n");
				printf("File Transfer Completed Successfully \n \n");
	free_ifi_info_plus(ifihead);
	exit(0);

}









