#ifndef UDP_H
#define UDP_H

u08 udp_receive_packet( ip_header * p, u16 len );

typedef enum udp_event_e
{
	UDP_EVENT_PACKET = 0,
} udp_event_e;

typedef u08 udp_sock;

#define INVALID_UDP_SOCK	0xff

typedef void udp_event_f( udp_sock sock, udp_event_e evt, 
						 u32 from_ip, u16 from_port, u08 const * data, u16 len );

void * udp_get_ctx( udp_sock sock );
udp_sock udp_new_sock( u16 port, void * ctx, udp_event_f * handler );
void udp_send( udp_sock sock, u32 to_ip, u16 to_port, u08 const * data, u16 len );
void udp_close( udp_sock sock );

#endif
