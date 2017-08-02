/***************************************************************************
***
***    Copyright 2005  Hon Hai Precision Ind. Co. Ltd.
***    All Rights Reserved.
***    No portions of this material shall be reproduced in any form without
***    the written permission of Hon Hai Precision Ind. Co. Ltd.
***
***    All information contained in this document is Hon Hai Precision Ind.
***    Co. Ltd. company private, proprietary, and trade secret property and
***    are protected by international intellectual property laws and treaties.
***
****************************************************************************
***
***    FILE NAME :
***      net.h
***
***    DESCRIPTION :
***      for NET component
***
***    HISTORY :
***       - 2015/01/15, 16:30:52, Chungmin Lai
***             File Creation
***
***************************************************************************/
#ifndef __NET_SERVICE_H
#define __NET_SERVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#ifndef ulong
#define ulong unsigned long
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef IPaddr_t
#define IPaddr_t unsigned long
#endif

#define PROT_IP             0x0800      /* IP protocol          */
#define PROT_ARP            0x0806      /* IP ARP protocol      */
#define PROT_RARP           0x8035      /* IP ARP protocol      */
#define PROT_VLAN           0x8100      /* IEEE 802.1q protocol     */

#define IPPROTO_ICMP        1           /* Internet Control Message Protocol    */
#define IPPROTO_UDP         17          /* User Datagram Protocol       */

#define ETHER_MAC_SIZE      6
#define ETHER_HDR_SIZE      14          /* Ethernet header size     */
#define ARP_HDR_SIZE        (8+20)      /* Size assuming ethernet   */

#define VLAN_HLEN           4
#define VLAN_ETHER_HDR_SIZE 18          /* VLAN Ethernet header size    */

/* 802 + SNAP + ethernet header size */
#define E802_HDR_SIZE       (sizeof(struct e802_hdr))

#define IP_FLAGS_RES        0x8000      /* reserved */
#define IP_FLAGS_DFRAG      0x4000      /* don't fragments */
#define IP_FLAGS_MFRAG      0x2000      /* more fragments */

#define IP_HDR_SIZE_NO_UDP  (sizeof (IP_t) - 8)
#define IP_HDR_SIZE         (sizeof (IP_t))

/* ICMP stuff (just enough to handle (host) redirect messages)
 */
#define ICMP_ECHO_REPLY     0           /* Echo reply           */
#define ICMP_REDIRECT       5           /* Redirect (change route)  */
#define ICMP_ECHO_REQUEST   8           /* Echo request         */

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET      0           /* Redirect Net         */
#define ICMP_REDIR_HOST     1           /* Redirect Host        */

/*==========================================================================
 *
 *      Macro Definition Segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Type and Structure Definition Segment
 *
 *==========================================================================
 */
typedef struct {
    uchar       et_dest[6]; /* Destination node     */
    uchar       et_src[6];  /* Source node          */
    ushort      et_protlen; /* Protocol or length       */
    uchar       et_dsap;    /* 802 DSAP         */
    uchar       et_ssap;    /* 802 SSAP         */
    uchar       et_ctl;     /* 802 control          */
    uchar       et_snap1;   /* SNAP             */
    uchar       et_snap2;
    uchar       et_snap3;
    ushort      et_prot;    /* 802 protocol         */
} Ethernet_t;

/*  Ethernet header
 */
typedef struct {
    uchar       vet_dest[6];    /* Destination node     */
    uchar       vet_src[6]; /* Source node          */
    ushort      vet_vlan_type;  /* PROT_VLAN            */
    ushort      vet_tag;    /* TAG of VLAN          */
    ushort      vet_type;   /* protocol type        */
} VLAN_Ethernet_t;

struct e802_hdr {
    uchar       et_dest[6]; /* Destination node     */
    uchar       et_src[6];  /* Source node          */
    ushort      et_protlen; /* Protocol or length       */
    uchar       et_dsap;    /* 802 DSAP         */
    uchar       et_ssap;    /* 802 SSAP         */
    uchar       et_ctl;     /* 802 control          */
    uchar       et_snap1;   /* SNAP             */
    uchar       et_snap2;
    uchar       et_snap3;
    ushort      et_prot;    /* 802 protocol         */
};

/*  Address Resolution Protocol (ARP) header.
 */
typedef struct
{
    ushort      ar_hrd;     /* Format of hardware address   */
#   define ARP_ETHER        1       /* Ethernet  hardware address   */
    ushort      ar_pro;     /* Format of protocol address   */
    uchar       ar_hln;     /* Length of hardware address   */
    uchar       ar_pln;     /* Length of protocol address   */
    ushort      ar_op;      /* Operation            */
#   define ARPOP_REQUEST    1       /* Request  to resolve  address */
#   define ARPOP_REPLY      2       /* Response to previous request */

#   define RARPOP_REQUEST   3       /* Request  to resolve  address */
#   define RARPOP_REPLY     4       /* Response to previous request */

    /*
     * The remaining fields are variable in size, according to
     * the sizes above, and are defined as appropriate for
     * specific hardware/protocol combinations.
     */
    uchar       ar_data[0];
} ARP_t;

/*  Internet Protocol (IP) header.
 */
typedef struct {
    uchar       ip_hl_v;    /* header length and version    */
    uchar       ip_tos;     /* type of service      */
    ushort      ip_len;     /* total length         */
    ushort      ip_id;      /* identification       */
    ushort      ip_off;     /* fragment offset field    */
    uchar       ip_ttl;     /* time to live         */
    uchar       ip_p;       /* protocol         */
    ushort      ip_sum;     /* checksum         */
    IPaddr_t    ip_src;     /* Source IP address        */
    IPaddr_t    ip_dst;     /* Destination IP address   */
    ushort      udp_src;    /* UDP source port      */
    ushort      udp_dst;    /* UDP destination port     */
    ushort      udp_len;    /* Length of UDP packet     */
    ushort      udp_xsum;   /* Checksum         */
} IP_t;

typedef struct icmphdr {
    uchar       type;
    uchar       code;
    ushort      checksum;
    union {
        struct {
            ushort  id;
            ushort  sequence;
        } echo;
        ulong   gateway;
        struct {
            ushort  __unused;
            ushort  mtu;
        } frag;
    } un;
} ICMP_t;

#endif
