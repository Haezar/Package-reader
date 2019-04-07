#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>

#include "util.h"
#include "flow.h"
#include "packet.h"
#include "tcp.h"
#include "http.h"

/***********************
 * Static functions      *
 ***********************/
static void 
flow_reset(flow_t *f);
static int 
hook_packet(flow_t *f, packet_t *packet, BOOL src);
static int 
cal_packet(flow_t *f, packet_t *packet, BOOL src);
static int 
compare_sequence_time(seq_t *seq1, seq_t *seq2);

/* Reset a flow_t object to reuse */
static void 
flow_reset(flow_t *f){
	packet_t *cp;

	while(f->pkt_dst_f != NULL){
		cp = f->pkt_dst_f;
		f->pkt_dst_f = f->pkt_dst_f->next;
		packet_free(cp);
	}

	while(f->pkt_src_f != NULL){
		cp = f->pkt_src_f;
		f->pkt_src_f = f->pkt_src_f->next;
		packet_free(cp);
	}

	if(f->order != NULL){
		order_free(f->order);
		f->order = order_new();
	}
	/*
	 * Note: maintain the f->socket.
	 */
	f->pkt_src_e = NULL;
	f->pkt_dst_e = NULL;
	f->rtt = 0;
	f->syn_sec = 0;
	f->syn_usec = 0;
	f->ack2_sec = 0;
	f->ack2_usec = 0;
	f->fb_sec = 0;
	f->fb_usec = 0;
	f->lb_sec = 0;
	f->lb_usec = 0;
	f->last_action_sec = 0;
	f->last_action_usec = 0;
	f->payload_src = 0;
	f->payload_dst = 0;
	f->pkt_src_n = 0;
	f->pkt_dst_n = 0;
	f->pkts_src = 0;
	f->pkts_dst = 0;
	f->http = 0;
	f->close = 0;
}

/* Serve for flow_add_packet(). Add packet to the flow packet chain. */
static int 
hook_packet(flow_t *f, packet_t *packet, BOOL src){
	/* set some pointers */
	packet_t **pkt_f = NULL, **pkt_e = NULL;
	u_int32_t *pkt_num = NULL;

	if(src == TRUE){
		pkt_f = &(f->pkt_src_f);
		pkt_e = &(f->pkt_src_e);
		pkt_num = &(f->pkt_src_n);
	}else{
		pkt_f = &(f->pkt_dst_f);
		pkt_e = &(f->pkt_dst_e);
		pkt_num = &(f->pkt_dst_n);
	}

	if((*pkt_num) == 0){
		(*pkt_f) = packet;
	}else{
		(*pkt_e)->next = packet;
	}

	(*pkt_e) = packet;
	(*pkt_e)->next = NULL;
	(*pkt_num)++;
	return 0;
}

/* Serve for flow_add_packet(). Update flow packet number and bytes, then update the last action time of flow. */
static int 
cal_packet(flow_t *f, packet_t *packet, BOOL src){
	u_int32_t *pkt_num = NULL, *byt_num = NULL;
	struct timeval tv;
	struct timezone tz;
	seq_t *seq = NULL;

	if(src == TRUE){
		pkt_num = &(f->pkts_src);
		byt_num = &(f->payload_src);
	}else{
		pkt_num = &(f->pkts_dst);
		byt_num = &(f->payload_dst);
	}

	(*pkt_num)++;
	(*byt_num) += packet->tcp_dl;

	/*
	 * Update the last action time of flow.
	 * Used to delete some dead flows forcedly.
	 */
	gettimeofday(&tv, &tz);
	f->last_action_sec = tv.tv_sec;
	f->last_action_usec = tv.tv_usec;
	
	/* Process ordering, which must be right AFTER flow info updated */
	seq = seq_pkt(packet);
	if ( packet->http == 0 ){
		/*
		 * The packet without http payload will NOT be stored.
		 */
		seq->pkt = NULL;
	}
	tcp_order(f->order, seq, src);

	return 0;
}

/* Compare seq_t objects if equel */
static int 
compare_sequence_time(seq_t *seq1, seq_t *seq2){
	u_int32_t	sec1 = seq1->cap_sec;
	u_int32_t	usec1 = seq1->cap_usec;
	u_int32_t	sec2 = seq2->cap_sec;
	u_int32_t	usec2 = seq2->cap_usec;
	int ret = 0;

	if(sec1 > sec2 || (sec1 == sec2 && usec1 > usec2)){
		ret = 1;
	}else if (sec1 < sec2 || (sec1 == sec2 && usec1 < usec2)){
		ret = -1;
	}else{
		ret = 0;
	}

	return ret;
}

/***********************
 * Flow functions      *
 ***********************/
/* Initiate both the flow hash table and flow queue */
int
flow_init(void){
	int ret;
	ret = flow_hash_init();
	if(ret != 0){
		return -1;
	}

	ret = flow_queue_init();
	if(ret != 0){
		return -1;
	}

	return 0;
}

/* Create a new flow record and initiate its members */
flow_t*
flow_new(void){
	flow_t *f;
	f = MALLOC(flow_t, 1);
	memset(f, 0, sizeof(flow_t));
	f->order = order_new();
	return f;
}


/* Free a flow_t object */
int 
flow_free(flow_t *f){
	packet_t *cp;
	http_pair_t	*h;
	while(f->pkt_dst_f != NULL){
		cp = f->pkt_dst_f;
		f->pkt_dst_f = f->pkt_dst_f->next;
		packet_free(cp);
	}

	while(f->pkt_src_f != NULL){
		cp = f->pkt_src_f;
		f->pkt_src_f = f->pkt_src_f->next;
		packet_free(cp);
	}

	if(f->order != NULL){
		order_free(f->order);
	}

	if(f->http_f != NULL){
		h = f->http_f;
		f->http_f = f->http_f->next;
		http_free(h);
	}

	free(f);
	return 0;
}

/* Compare two flow_s objects */
int 
flow_socket_cmp(flow_s *a, flow_s *b){
	return memcmp(a, b, sizeof(flow_s));
}

/* Add a http_pair_t object to flow's http_pair_t chain */
int 
flow_add_http(flow_t *f, http_pair_t *h){
	if(f->http_cnt == 0){
		f->http_f = h;
	}else{
		f->http_e->next = h;
	}
	f->http_e = h;
	f->http_e->next = NULL;
	f->http_cnt++;
	return 0;
}

/* Add a packet_t object to flow's packet_t chain */
int 
flow_add_packet(flow_t *f, packet_t *packet, register BOOL src){
	pthread_mutex_lock(&(f->hmb->mutex));

	if( f->http == FALSE ){
		if( f->pkt_src_n >= 5){
			/* We make sure that the flow is not a HTTP flow,
			 * then remove it */
			packet_free(packet);
			flow_free(flow_hash_delete(f));
			pthread_mutex_unlock(&(f->hmb->mutex));
			return 1;
		}
	}

	/* TH_RST:
	 * If the flow is reset by sender or receiver*/
	if((packet->tcp_flags & TH_RST) == TH_RST){
		if( f->pkts_src < 4){
			// Flow with uncomplete information. Drop it.
			packet_free(packet);
			flow_free(flow_hash_delete(f));
			pthread_mutex_unlock(&(f->hmb->mutex));
			return 1;
		}else{
			cal_packet(f, packet, src);
			packet_free(packet);
			f->close = TRUE;
			flow_queue_enq(flow_hash_delete(f));
			pthread_mutex_unlock(&(f->hmb->mutex));
            return 0;
		}
	}

	/* TH_ACK: third handshake */
	if(f->pkts_src == 1 && src == TRUE){
		if((packet->tcp_flags & TH_ACK) == TH_ACK){
			f->ack2_sec = packet->cap_sec;
			f->ack2_usec = packet->cap_usec;
			/* round trip time in microsecond */
			f->rtt = (f->ack2_sec - f->syn_sec) * 1000000 + (f->ack2_usec - f->syn_usec);

			cal_packet(f, packet, src);
			packet_free(packet);
			pthread_mutex_unlock(&(f->hmb->mutex));
			return 0;
		}
	}

	/* TH_FIN:
	 * The flow will be closed if the both fins are detected */
	if( (packet->tcp_flags & TH_FIN) == TH_FIN){
		if( src == TRUE ){
			f->close = CLIENT_CLOSE;
		}else{
			f->close = SERVER_CLOSE;
		}		
		cal_packet(f, packet, src);
		packet_free(packet);

		if(f->close == CLIENT_CLOSE  || f->close == SERVER_CLOSE){		/* && or || */
			/* flow finished and send it to the flow queue */
			f->close = TRUE;
			flow_queue_enq(flow_hash_delete(f));
		}

		pthread_mutex_unlock(&(f->hmb->mutex));
		return 0;
	}

	/* other packets, without sequence number checked */
	if(src == TRUE){
		if( f->pkts_src == 0){
			/* syn */
			f->syn_sec = packet->cap_sec;
			f->syn_usec = packet->cap_usec;

			cal_packet(f, packet, src);
			packet_free(packet);
		}else{
			if(packet->tcp_flags == TH_SYN){
				/*syn repeatly*/
				flow_reset(f);		// Reset flow
				f->syn_sec = packet->cap_sec;
				f->syn_usec = packet->cap_usec;
				cal_packet(f, packet, src);
				packet_free(packet);
			}else{
				if(packet->http != 0 ){
					f->http = TRUE;
					/*
					 * only packets with HTTP payload
					 * are hooked on the packet chain
					 */
					hook_packet(f, packet, src);
					cal_packet(f, packet,src);
				}else{
					cal_packet(f, packet, src);
					packet_free(packet);
				}
			}
		}
	}else{
		if(packet->http != 0){
			f->http = TRUE;
			/*
			 * only packets with HTTP payload
			 * are hooked on the packet chain
			 */
			hook_packet(f, packet, src);
			cal_packet(f, packet, src);
		}else{
			cal_packet(f, packet, src);
			packet_free(packet);
		}
	}

	pthread_mutex_unlock(&(f->hmb->mutex));
	return 0;
}

