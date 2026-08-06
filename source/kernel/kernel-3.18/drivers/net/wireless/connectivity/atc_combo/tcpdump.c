/*
* Copyright (c) 2023 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
#include "wi_begin.h"
#include <uapi/linux/if_ether.h>
#include <uapi/linux/ip.h>
#include <uapi/linux/tcp.h>
#include <uapi/linux/udp.h>
#include <uapi/linux/icmp.h>
#include <uapi/linux/if_arp.h>
#include <linux/module.h>
#include "wi_end.h"

#include "common.h"

#define ATC_COMBO_TCPDUMP_ICMP     (1 << 0)
#define ATC_COMBO_TCPDUMP_TCP      (1 << 1)
#define ATC_COMBO_TCPDUMP_UDP      (1 << 2)

int atc_combo_tcpdump_enable = ATC_COMBO_TCPDUMP_ICMP;

static bool atc_combo_tcpdump_tcp(void *ethhdr, bool tx, u32 seq)
{
	struct ethhdr *eth = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;

	if (!ethhdr) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	eth = (struct ethhdr *)ethhdr;
	if (ntohs(eth->h_proto) != ETH_P_IP) {
		return false;
	}
	iph = (struct iphdr *)(eth + 1);
	if (iph->protocol != IPPROTO_TCP) {
		return false;
	}
	tcph = (struct tcphdr *)((void *)iph + iph->ihl * 4);

	if (atc_combo_tcpdump_enable & ATC_COMBO_TCPDUMP_TCP) {
		COMBO_INFO("<%s>[TCP][%u] ip[%pI4 -> %pI4] port[%u -> %u] " //-V594 //-V557
				"SeqNo %u AckNo %u Flags 0x%02x SYN %u FIN %u ACK %u\n",
				tx ? "TX" : "RX", seq, &iph->saddr, &iph->daddr,
				ntohs(tcph->source), ntohs(tcph->dest),
				ntohl(tcph->seq), ntohl(tcph->ack_seq),
				*((u8 *)&tcph->ack_seq + 5),
				tcph->syn, tcph->fin, tcph->ack);
	}

	return true;
}

#define IP_PORT_BOOTP_SERVER        67
#define IP_PORT_BOOTP_CLIENT        68

#define BOOTREQUEST                 1
#define BOOTREPLY                   2

#define DHCP_MAGIC                  0x63825363

#define DHCP_OPTION_MSG_TYPE        53

#define DHCP_DISCOVER               1
#define DHCP_OFFER                  2
#define DHCP_REQUEST                3
#define DHCP_DECLINE                4
#define DHCP_ACK                    5
#define DHCP_NAK                    6
#define DHCP_RELEASE                7
#define DHCP_INFORM                 8

#define BROADCAST_FLAG              0x8000

struct bootphdr {
	u8 op;          // operation
	u8 htype;       // header type
	u8 hlen;        // header length
	u8 hops;        // hops
	u32 xid;        // transaction id
	u16 secs;       // seconds
	u16 flags;
	u32 ciaddr;     // client ip
	u32 yiaddr;     // offer/ack ip
	u32 siaddr_nip; // next server ip
	u32 gateway_nip;
	u8 chaddr[16];  // client mac
	u8 sname[64];   // server name
	u8 file[128];
	u32 magic;
	u8 option[0];
} __packed;

static bool atc_combo_tcpdump_dhcp(
		struct bootphdr *bootph, u16 ipid, bool tx, u32 seq)
{
	if (!bootph) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	if (DHCP_MAGIC != ntohl(bootph->magic)) {
		return false;
	}
	if (bootph->option[0] != DHCP_OPTION_MSG_TYPE) {
		return false;
	}
	switch (bootph->option[2]) {
	case DHCP_DISCOVER:
		COMBO_INFO("<%s>[DHCP][%u] discover "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	case DHCP_OFFER:
		COMBO_INFO("<%s>[DHCP][%u] offer %pI4 "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq, &bootph->yiaddr,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	case DHCP_REQUEST:
		COMBO_INFO("<%s>[DHCP][%u] request "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	case DHCP_ACK:
		COMBO_INFO("<%s>[DHCP][%u] ack %pI4 "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq, &bootph->yiaddr,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	case DHCP_NAK:
		COMBO_ERR("<%s>[DHCP][%u] nak yiaddr %pI4 "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq, &bootph->yiaddr,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	case DHCP_DECLINE:
		COMBO_ERR("<%s>[DHCP][%u] decline yiaddr %pI4 "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq, &bootph->yiaddr,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	case DHCP_RELEASE:
		COMBO_INFO("<%s>[DHCP][%u] release ciaddr %pI4 yiaddr %pI4 "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq, &bootph->ciaddr, &bootph->yiaddr,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	case DHCP_INFORM:
		COMBO_INFO("<%s>[DHCP][%u] inform ciaddr %pI4 yiaddr %pI4 "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq, &bootph->ciaddr, &bootph->yiaddr,
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	default:
		COMBO_WARN("<%s>[DHCP][%u] unknown type %u "
				"ipid 0x%04x xid 0x%08x chaddr %pM\n",
				tx ? "TX" : "RX", seq, bootph->option[2],
				ipid, ntohl(bootph->xid), &bootph->chaddr);
		break;
	}

	return true;
}

static bool atc_combo_tcpdump_udp(void *ethhdr, bool tx, u32 seq)
{
	struct ethhdr *eth = NULL;
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	u16 source = 0;
	u16 dest = 0;
	bool ret = false;

	if (!ethhdr) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	eth = (struct ethhdr *)ethhdr;
	if (ntohs(eth->h_proto) != ETH_P_IP) {
		return false;
	}
	iph = (struct iphdr *)(eth + 1);
	if (iph->protocol != IPPROTO_UDP) {
		return false;
	}
	udph = (struct udphdr *)((void *)iph + iph->ihl * 4);
	source = ntohs(udph->source);
	dest = ntohs(udph->dest);
	if (dest == IP_PORT_BOOTP_SERVER || dest == IP_PORT_BOOTP_CLIENT) {
		struct bootphdr *bootph = (struct bootphdr *)(udph + 1);
		ret = atc_combo_tcpdump_dhcp(bootph, ntohs(iph->id), tx, seq);
	} else {
		if (atc_combo_tcpdump_enable & ATC_COMBO_TCPDUMP_UDP) {
			COMBO_INFO("<%s>[UDP][%u] ip[%pI4 -> %pI4] port[%u -> %u] len %u\n",
					tx ? "TX" : "RX", seq, &iph->saddr, &iph->daddr,
					source, dest, ntohs(udph->len));
		}
		ret = true;
	}

	return ret;
}

static bool atc_combo_tcpdump_icmp(void *ethhdr, bool tx, u32 seq)
{
	struct ethhdr *eth = NULL;
	struct iphdr *iph = NULL;
	struct icmphdr *icmph = NULL;

	if (!ethhdr) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	eth = (struct ethhdr *)ethhdr;
	if (ntohs(eth->h_proto) != ETH_P_IP) {
		return false;
	}
	iph = (struct iphdr *)(eth + 1);
	if (iph->protocol != IPPROTO_ICMP) {
		return false;
	}
	icmph = (struct icmphdr *)((void *)iph + iph->ihl * 4);
	if (atc_combo_tcpdump_enable & ATC_COMBO_TCPDUMP_ICMP) {
		COMBO_INFO("<%s>[ICMP][%u] ip[%pI4 -> %pI4] ipid 0x%04x ttl %u "
				"type %u %s code %u id %u seq %u\n",
				tx ? "TX" : "RX", seq,
				&iph->saddr, &iph->daddr, ntohs(iph->id), iph->ttl,
				icmph->type, icmph->type == ICMP_ECHO ? "echo"
				: icmph->type == ICMP_ECHOREPLY ? "reply" : "", icmph->code,
				(u16)icmph->un.echo.id, ntohs(icmph->un.echo.sequence));
	}

	return true;
}

struct arp4hdr {
	struct arphdr h;
	u8 ar_sha[ETH_ALEN];
	u8 ar_sip[4];
	u8 ar_tha[ETH_ALEN];
	u8 ar_tip[4];
};

static bool atc_combo_tcpdump_arp(void *ethhdr, bool tx, u32 seq)
{
	struct ethhdr *eth = NULL;
	struct arphdr *arph = NULL;
	struct arp4hdr *arp4h = NULL;

	if (!ethhdr) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	eth = (struct ethhdr *)ethhdr;
	if (ntohs(eth->h_proto) != ETH_P_ARP) {
		return false;
	}
	arph = (struct arphdr *)(eth + 1);
	if (ntohs(arph->ar_pro) != ETH_P_IP) {
		return false;
	}
	arp4h = (struct arp4hdr *)arph;

	COMBO_INFO("<%s>[ARP][%u] op %u %s mac[%pM -> %pM] ip[%pI4 -> %pI4]\n",
			tx ? "TX" : "RX", seq,
			ntohs(arph->ar_op),
			ntohs(arph->ar_op) == ARPOP_REQUEST ? "REQ"
			: ntohs(arph->ar_op) == ARPOP_REPLY ? "RSP" : "",
			&arp4h->ar_sha, &arp4h->ar_tha,
			&arp4h->ar_sip, &arp4h->ar_tip);

	return true;
}

#define ETH_P_1X                    ETH_P_PAE
#define ETH_P_PRE_1X                0x88C7
#define ETH_P_WAPI_1X               0x88B4

static bool atc_combo_tcpdump_sec_frame(void *ethhdr, bool tx, u32 seq)
{
	struct ethhdr *eth = NULL;
	u8 *eapol = NULL;
	bool ret = true;

	if (!ethhdr) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	eth = (struct ethhdr *)ethhdr;
	eapol = (u8 *)(eth + 1);

	switch (ntohs(eth->h_proto)) {

	case ETH_P_1X:
		switch (eapol[1]) {
		case 0: // eap packet
			COMBO_INFO("<%s>[EAP][%u] packet code %u id %u type %u\n",
					tx ? "TX" : "RX", seq, eapol[4], eapol[5], eapol[7]);
			break;

		case 1: // eapol start
			COMBO_INFO("<%s>[EAPOL][%u] start\n", tx ? "TX" : "RX", seq);
			break;

		case 3: // key
			COMBO_INFO("<%s>[EAPOL][%u] key keyinfo 0x%04x\n",
					tx ? "TX" : "RX", seq, *(u16 *)&eapol[5]);
			break;

		default:
			COMBO_WARN("<%s>[EAPOL][%u] unknown type %u\n",
					tx ? "TX" : "RX", seq, eapol[0]);
			break;
		}
		break;

	case ETH_P_PRE_1X:
		break;

	case ETH_P_WAPI_1X:
		COMBO_WARN("<%s>[WAPI][%u] subtype %u len %u seq %u\n",
				tx ? "TX" : "RX", seq, eapol[3],
				*(u16 *)&eapol[6], *(u16 *)&eapol[8]);
		break;

	default:
		ret = false;
		break;
	}

	return ret;
}

static bool atc_combo_tcpdump_tdls(void *ethhdr, bool tx, u32 seq)
{
	struct ethhdr *eth = NULL;
	u8 *tdls = NULL;

	if (!ethhdr) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	eth = (struct ethhdr *)ethhdr;
	if (ntohs(eth->h_proto) != ETH_P_TDLS) {
		return false;
	}
	tdls = (u8 *)(eth + 1);
	COMBO_INFO("<%s>[TDLS][%u] type %u category %u action %u token %u\n",
			tx ? "TX" : "RX", seq,
			tdls[0], tdls[1], tdls[2], tdls[3]);

	return true;
}

/**
 * atc_combo_tcpdump - dump network package
 * @ethhdr: ethernet frame pointer
 * @tx:     is tx
 * @seq:    driver tx seq number
 * @retrun: is supported protocol
 */
bool atc_combo_tcpdump(void *ethhdr, bool tx, u32 seq)
{
	struct ethhdr *eth = NULL;
	bool ret = false;

	if (!ethhdr) {
		COMBO_ERR("NULL pointer\n");
		return false;
	}
	eth = (struct ethhdr *)ethhdr;

	switch (ntohs(eth->h_proto)) {

	case ETH_P_IP: {
		struct iphdr *iph = (struct iphdr *)(eth + 1);

		if (iph->version != IPVERSION) {
			break;
		}
		switch (iph->protocol) {

		case IPPROTO_TCP:
			ret = atc_combo_tcpdump_tcp(eth, tx, seq);
			break;

		case IPPROTO_UDP:
			ret = atc_combo_tcpdump_udp(eth, tx, seq);
			break;

		case IPPROTO_ICMP:
			ret = atc_combo_tcpdump_icmp(eth, tx, seq);
			break;

		default:
			break;
		}
		break;
	}
	case ETH_P_ARP:
		ret = atc_combo_tcpdump_arp(eth, tx, seq);
		break;

	case ETH_P_1X:
	case ETH_P_PRE_1X:
	case ETH_P_WAPI_1X:
		ret = atc_combo_tcpdump_sec_frame(eth, tx, seq);
		break;

	case ETH_P_TDLS:
		ret = atc_combo_tcpdump_tdls(eth, tx, seq);
		break;

	default:
		break;
	}

	return ret;
}
EXPORT_SYMBOL(atc_combo_tcpdump);

MODULE_DESCRIPTION("Dump network package");
MODULE_ALIAS("atc_combo:tcpdump");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");
