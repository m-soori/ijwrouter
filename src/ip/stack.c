#include "../common.h"
#include "stack.h"
#include "arptab.h"
#include "rfc.h"
#include "conf.h"
#include "arp.h"
#include "icmp.h"
#include "../hal_debug.h"

static send_packet_f * on_send = 0;

void __send_packet( u08 iface, u08 const * buf, u16 len )
{
	if (on_send)
		on_send( iface, buf, len );
	else
		logf( "ip: trying to send packet but no driver bound.\n" );
}

void ipstack_init( send_packet_f * send_callback )
{
	on_send = send_callback;
	// todo: initialize some other things
}

void ipstack_tick( void )
{
	arptab_tick();
}

static u08 ip_receive_packet( u08 iface, ip_header * p, u16 len )
{
	logf( "ip: got ip packet, proto=%d\n", p->proto );
	if ( p->dest_addr != 0xfffffffful && p->dest_addr != get_hostaddr() )
		return 0;

	// todo: verify checksum
	switch( p->proto )
	{
	case IPPROTO_ICMP:
		icmp_receive_packet( iface, p, len );
		break;
	}

	return (p->dest_addr == 0xfffffffful) ? 0 : 1;
}

u08 ipstack_receive_packet( u08 iface, u08 const * buf, u16 len )
{
	eth_header * eth = (eth_header *) buf;
	u16 etype = __ntohs( eth->ethertype ); 

	len;

	switch( etype )
	{
	case ethertype_ipv4:
		{
			ip_header * ip = (ip_header *) (eth + 1);
			arptab_insert( iface, ip->src_addr, eth->src );
			return ip_receive_packet( iface, ip, len - sizeof( eth_header ) );
		}

	case ethertype_arp:
			return handle_arp_packet( iface, (arp_header *) (eth + 1) );
	}

	return 0;
}