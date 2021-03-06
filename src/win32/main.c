// ijw router win32 stub

#pragma comment( lib, "ws2_32.lib" )
#include "../common.h"
#include "../ip/rfc.h"
#include "../ip/conf.h"
#include "../ip/arptab.h"
#include "../hal_ethernet.h"
#include "../hal_debug.h"
#include "../user.h"
#include "../hal_time.h"

#include "../ip/stack.h"
#include "../ip/tcp.h"

#include "../httpserv/httpserv.h"
#include "../fs.h"

#include <assert.h>

u08 charge_for_packet( eth_packet * p )
{
	mac_addr lanside = (p->dest_iface == IFACE_WAN) 
		? p->packet->src : p->packet->dest;

	user_t * u = get_user( lanside );

	if (!u)
	{
		log_printf("!!! user creation failed !!!\n");
		return eth_discard( p );
	}

	u->credit += p->len;
	return eth_forward(p);
}

void eth_inject_packet( u08 iface, u08 const * data, u16 len )
{
	eth_packet p;
	p.src_iface = IFACE_INTERNAL;
	p.dest_iface = iface;
	p.len = len;
	p.packet = (eth_header *)data;

	eth_inject( &p );
}

u08 handle_packet( eth_packet * p )
{
	if (p->dest_iface == IFACE_INTERNAL)
	{
		ipstack_receive_packet( p->src_iface, (u08 const *)p->packet, p->len );
		return eth_discard( p );
	}

	if (p->dest_iface == IFACE_BROADCAST)
	{
		ipstack_receive_packet( p->src_iface, (u08 const *)p->packet, p->len );
		return eth_forward( p );
	}

	if (!arptab_queryif( &p->dest_iface, &p->packet->dest ))
		p->dest_iface = IFACE_BROADCAST;

	//return eth_forward( p );

	//p->dest_iface = IFACE_BROADCAST;

/*	if (p->dest_iface != IFACE_WAN && p->src_iface != IFACE_WAN)
	{
		log_printf( "-- pure local --\n" );
		return eth_forward( p );
	}	*/

	return charge_for_packet( p );
}

extern void dhcp_init( void );
extern void dhcp_process( void );

extern void nbns_process( void );
extern void nbns_init( void );

extern void sntp_process( void );

int main( void )
{
	u08 interfaces = eth_init();

	ipstack_init( eth_inject_packet );

	if (!interfaces)
		log_printf( "! no interfaces available\n" );

	dhcp_init();
	nbns_init();

	fs_init();

	httpserv_init();

	restore_users();

	for(;;)
	{
		eth_packet p;
		if (eth_getpacket( &p ))
			handle_packet( &p );
		dhcp_process();
		nbns_process();
		sntp_process();
		tcp_process( get_time() );

		do_periodic_save();
	}
}