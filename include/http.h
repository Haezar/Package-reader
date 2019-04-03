
#ifndef __HTTP_H__
#define __HTTP_H__

#include <sys/types.h>
#include <time.h>

#include "util.h"


typedef enum _http_status http_status;
enum _http_status
{
    HTTP_ST_100=100,   
    HTTP_ST_101,       
    HTTP_ST_102,       
    HTTP_ST_199=199,   

    HTTP_ST_200,       
    HTTP_ST_201,      
    HTTP_ST_202,      
    HTTP_ST_203,       
    HTTP_ST_204,       
    HTTP_ST_205,       
    HTTP_ST_206,      
    HTTP_ST_207,       
    HTTP_ST_299=299,   

    HTTP_ST_300,       
    HTTP_ST_301,       
    HTTP_ST_302,       
    HTTP_ST_303,       
    HTTP_ST_304,       
    HTTP_ST_305,       
    HTTP_ST_307,      
    HTTP_ST_399=399,   

    HTTP_ST_400,       
    HTTP_ST_401,       
    HTTP_ST_402,       
    HTTP_ST_403,       
    HTTP_ST_404,       
    HTTP_ST_405,       
    HTTP_ST_406,       
    HTTP_ST_407,      
    HTTP_ST_408,       
    HTTP_ST_409,       
    HTTP_ST_410,       
    HTTP_ST_411,       
    HTTP_ST_412,       
    HTTP_ST_413,       
    HTTP_ST_414,       
    HTTP_ST_415,       
    HTTP_ST_416,       
    HTTP_ST_417,      
    HTTP_ST_422=422,   
    HTTP_ST_423,       
    HTTP_ST_424,       
    HTTP_ST_499=499,   

    HTTP_ST_500,       
    HTTP_ST_501,       
    HTTP_ST_502,       
    HTTP_ST_503,       
    HTTP_ST_504,       
    HTTP_ST_505,       
    HTTP_ST_507=507,   
    HTTP_ST_599=599,   
    HTTP_ST_NONE
};


typedef struct _http_st_code http_st_code;
struct _http_st_code
{
    int num;          
    http_status st;     
};

extern http_st_code HTTP_STATUS_CODE_ARRAY[];	


typedef enum _http_mthd http_mthd;
enum _http_mthd
{
    HTTP_MT_OPTIONS = 0, 
    HTTP_MT_GET,
    HTTP_MT_HEAD,
    HTTP_MT_POST,
    HTTP_MT_PUT,
    HTTP_MT_DELETE,
    HTTP_MT_TRACE,
    HTTP_MT_CONNECT,
    HTTP_MT_PATCH,
    HTTP_MT_LINK,
    HTTP_MT_UNLINK,
    HTTP_MT_PROPFIND,    
    HTTP_MT_MKCOL,
    HTTP_MT_COPY,
    HTTP_MT_MOVE,
    HTTP_MT_LOCK,
    HTTP_MT_UNLOCK,
    HTTP_MT_POLL,        
    HTTP_MT_BCOPY,
    HTTP_MT_BMOVE,
    HTTP_MT_SEARCH,
    HTTP_MT_BDELETE,
    HTTP_MT_PROPPATCH,
    HTTP_MT_BPROPFIND,
    HTTP_MT_BPROPPATCH,
    HTTP_MT_LABEL,             
    HTTP_MT_MERGE,             
    HTTP_MT_REPORT,            
    HTTP_MT_UPDATE,           
    HTTP_MT_CHECKIN,           
    HTTP_MT_CHECKOUT,          
    HTTP_MT_UNCHECKOUT,        
    HTTP_MT_MKACTIVITY,       
    HTTP_MT_MKWORKSPACE,       
    HTTP_MT_VERSION_CONTROL,   
    HTTP_MT_BASELINE_CONTROL,  
    HTTP_MT_NOTIFY,            
    HTTP_MT_SUBSCRIBE,
    HTTP_MT_UNSUBSCRIBE,
    HTTP_MT_ICY,             
    HTTP_MT_NONE
};

extern char *HTTP_METHOD_STRING_ARRAY[];	


typedef enum _http_ver http_ver;
enum _http_ver
{
    HTTP_VER_1_0,
    HTTP_VER_1_1,
    HTTP_VER_NONE
};


typedef struct _request_t request_t;
struct _request_t
{
	http_ver 	version;
	http_mthd	method;
	char*		host;
	char*		uri;
	char*		user_agent;
	char*		referer;
	char*		connection;
	char*		accept;
	char*		accept_encoding;
	char*		accept_language;
	char*		accept_charset;
	char*		cookie;

	char*		content_type;
	char*		content_encoding;
	char*		content_length;
	int			hdlen;	
};


typedef struct _response_t response_t;
struct _response_t
{
	http_status 	status;
	http_ver	 	version;
	char*			server;
	char*			date;
	char*			expires;
	char*			location;
	char*			etag;
	char*			accept_ranges;
	char*			last_modified;
	char*			content_type;
	char*			content_encoding;
	char* 			content_length;
	char*		 	age;
	int				hdlen;	
};


typedef struct _http_pair_t	http_pair_t;
struct _http_pair_t
{
	request_t	*request_header;
	response_t	*response_header;
	time_t	req_fb_sec;
	time_t	req_fb_usec;
	time_t	req_lb_sec;
	time_t	req_lb_usec;
	time_t	rsp_fb_sec;
	time_t	rsp_fb_usec;
	time_t	rsp_lb_sec;
	time_t	rsp_lb_usec;
	u_int32_t	req_total_len;
	u_int32_t	rsp_total_len;
	u_int32_t	req_body_len;
	u_int32_t	rsp_body_len;
	http_pair_t	*next;
};

char* IsRequest(const char *p, const int datalen);	    
char* IsResponse(const char *p, const int datalen);	    
BOOL IsHttpPacket(const char *p, const int datalen);	

http_pair_t* http_new(void);						    
void http_free(http_pair_t *h);			                
request_t* http_request_new(void);				        
void http_request_free(request_t *req);	               
response_t* http_response_new(void);			        
void http_response_free(response_t *rsp);			    
int http_add_request(http_pair_t *h, request_t *req);	
int http_add_response(http_pair_t *h, response_t *rsp);	

int http_parse_request(request_t *request, const char *data, const char *dataend);		
int http_parse_response(response_t *response, const char *data, const char *dataend);

#endif 
