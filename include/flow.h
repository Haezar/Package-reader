
#ifndef FLOW_H_
#define FLOW_H_

#include <pthread.h>
#include <sys/types.h>

#include "packet.h"
#include "order.h"
#include "http.h"
#include "util.h"


typedef struct _flow_s	flow_s;
struct _flow_s
{
	u_int32_t	saddr;
	u_int32_t	daddr;
	u_int16_t	sport;
	u_int16_t	dport;
};

typedef struct _hash_mb_t hash_mb_t;


typedef struct _flow_t flow_t;

struct _hash_mb_t
{
	flow_t	*first;
	flow_t	*last;
	pthread_mutex_t mutex;
	int		elm_cnt;
};


struct _flow_t
{
	flow_t		*next;
	flow_t		*prev;

	packet_t	*pkt_src_f;		
	packet_t	*pkt_src_e;		
	packet_t	*pkt_dst_f;		
	packet_t	*pkt_dst_e;		
	u_int32_t	pkt_src_n;		
	u_int32_t	pkt_dst_n;		
	u_int32_t	pkts_src;       
	u_int32_t	pkts_dst;	    
	order_t		*order;		    
	hash_mb_t	*hmb;		    

	flow_s		socket;		   
	u_int8_t	rtt;		   
	time_t		syn_sec;	   
	time_t		syn_usec;	    
	time_t		ack2_sec;	    
	time_t		ack2_usec;	    
	time_t		fb_sec;		    
	time_t		fb_usec;	   
	time_t		lb_sec;		    
	time_t		lb_usec;	   
	u_int32_t	payload_src;	
	u_int32_t 	payload_dst;	

	BOOL		http;		    
	http_pair_t		*http_f;	
	http_pair_t		*http_e;	
	u_int32_t	http_cnt;	    

	time_t		last_action_sec;	
	time_t		last_action_usec;	
	u_int8_t	close;
#define CLIENT_CLOSE	0x01
#define SERVER_CLOSE	0x02
#define FORCED_CLOSE	0x04
};

int flow_init(void);	       
flow_t* flow_new(void);		 
int flow_free(flow_t*);	       
int flow_add_packet(flow_t *f, packet_t *packet, BOOL src);	
int flow_socket_cmp(flow_s *a, flow_s *b);	              
int flow_extract_http(flow_t *f);			               
int flow_add_http(flow_t *f, http_pair_t *h);	            


int flow_hash_init(void);			       
flow_t* flow_hash_new(flow_s s);		   
flow_t* flow_hash_delete(flow_t *f);	    
flow_t* flow_hash_find(flow_s s);		   
int flow_hash_add_packet(packet_t *packet);	
int flow_hash_clear(void);	                
int flow_hash_size(void);	                
int flow_hash_fcnt(void);	                
int flow_hash_scnt(void);	                
int flow_scrubber(const int timeslot);	   
void flow_hash_print(void);	                


int flow_queue_init(void);		   
int flow_queue_enq(flow_t *flow);	
flow_t* flow_queue_deq(void);		
int flow_queue_clear(void);		   
int flow_queue_len(void);		    
void flow_queue_print(void);	    

#endif 
