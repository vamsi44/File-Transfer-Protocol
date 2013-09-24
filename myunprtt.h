#ifndef _unp_rtt_h
#define _unp_rtt_h
#include "unp.h"
struct rtt_info
{
int rtt_rtt;
int rtt_srtt;
int rtt_rttvar;
int rtt_rto;
int rtt_nrexmt;
uint32_t rtt_base;
};
 #define RTT_RXTMIN 1
 #define RTT_RXTMAX 3
#define RTT_MAXNREXMT 12

void rtt_debug(struct rtt_info *);
void rtt_init(struct rtt_info *);
void rtt_newpack(struct rtt_info *);
int rtt_start(struct rtt_info *);
void rtt_stop(struct rtt_info *,uint32_t);
int rtt_timeout(struct rtt_info *);
uint32_t rtt_ts(struct rtt_info *);
extern int rtt_d_flag;
#endif
