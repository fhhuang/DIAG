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
***      net.c
***
***    DESCRIPTION :
***      for NET component
***
***    HISTORY :
***       - 2015/01/15, 16:30:52, Chungmin Lai
***             File Creation
***
***************************************************************************/

/*==========================================================================
 *
 *      Library Inclusion Segment
 *
 *==========================================================================
 */
#include "cmn_type.h"
#include "switch_hal.h"
#include "err_type.h"
#include "log.h"
#include "foxCommand.h"
#include "port_utils.h"

#include "porting.h"
#include "net.h"
#include <dirent.h> 

/*==========================================================================
 *
 *      Constant
 *
 *==========================================================================
 */
#define VLAN_NONE   4095            /* untagged             */
#define VLAN_IDMASK 0x0fff          /* mask of valid vlan id    */

/*==========================================================================
 *
 *      Structrue segment
 *
 *==========================================================================
 */

/*==========================================================================
 *
 *      Static Variable segment
 *
 *==========================================================================
 */

uchar       NetOurEther[6] = {0};
uchar       NetBcastAddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uchar       NetEtherNullAddr[6] = { 0, 0, 0, 0, 0, 0 };
uchar       NetServEther[6];
uchar       NetArpWaitPacketMAC[6]; /* MAC address of waiting packet's destination  */

ushort      NetOurVLAN = 0xFFFF;         /* default is without VLAN  */

IPaddr_t    NetOurIP=0x0a8dc8de;    /* Our IP addr (0 = unknown)        */
IPaddr_t    NetServerIP=0x0a8dc8f6; /* Our IP addr (0 = unknown)        */
IPaddr_t    NetOurGatewayIP=0;      /* Our gateways IP address  */
IPaddr_t    NetOurSubnetMask=0;

IPaddr_t    NetArpWaitReplyIP=0;
IPaddr_t    NetArpWaitPacketIP=0;

static ushort PingSeqNo=313;
static ushort oldPingSeqNo;
static int ping_times=0;
static int ping_reply_count=0;
static int tx_port=1;

UINT8 tx_data[1518+64];
UINT8 rx_data[1518+64];
extern uchar NetTxPacket[1518+64];
unsigned    NetIPID;

int net_time_out=0;

/*==========================================================================
 *
 *      Function Definition Segment
 *
 *==========================================================================
 */
int NetSendPacket(uchar *pktbuf,uchar *ether,int len);
int NetSendUDPPacket(uchar *pktbuf,uchar *ether, IPaddr_t dest, int dport, int sport, int len);
int NetCksumOk(uchar * ptr, int len);
unsigned NetCksum(uchar * ptr, int len);

/*==========================================================================
 *
 *      Static Funtion Body segment
 *
 *==========================================================================
 */
/* write IP in network byteorder */
static inline void NetWriteIP(void *to, IPaddr_t ip)
{
    memcpy(to, (void*)&ip, sizeof(ip));
}

static inline IPaddr_t NetReadIP(void *from)
{
    IPaddr_t ip;
    memcpy((void*)&ip, from, sizeof(ip));
    return ip;
}

/* copy IP */
static inline void NetCopyIP(void *to, void *from)
{
    memcpy(to, from, sizeof(IPaddr_t));
}

static void ICMPHandler(IP_t *ip, int len, IPaddr_t src_ip, Ethernet_t *et)
{
    ICMP_t *icmph = (ICMP_t *)&ip->udp_src;

    switch (icmph->type)
    {
        case ICMP_REDIRECT:
            if (icmph->code != ICMP_REDIR_HOST)
            {
                return;
            }
            log_printf(" ICMP Host Redirect to %pI4 ", &icmph->un.gateway);
            break;
        default:
            PingHandler(et,ip,len);
            break;
    }
}

/*==========================================================================
 *
 *      Local Function Body segment
 *
 *==========================================================================
 */
void
NetSetIP(volatile uchar * xip, IPaddr_t dest, int dport, int sport, int len)
{
    volatile IP_t *ip = (IP_t *)xip;

    /*  If the data is an odd number of bytes, zero the
     *  byte after the last byte so that the checksum
     *  will work.
     */
    if (len & 1)
    {
        xip[IP_HDR_SIZE + len] = 0;
    }

    /*  Construct an IP and UDP header.
     *  (need to set no fragment bit - XXX)
     */
    ip->ip_hl_v  = 0x45;        /* IP_HDR_SIZE / 4 (not including UDP) */
    ip->ip_tos   = 0;
    ip->ip_len   = htons(IP_HDR_SIZE + len);
    ip->ip_id    = htons(NetIPID++);
    ip->ip_off   = htons(0x4000);   /* No fragmentation */
    ip->ip_ttl   = 255;
    ip->ip_p     = 17;      /* UDP */
    ip->ip_sum   = 0;
    NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
    NetCopyIP((void*)&ip->ip_dst, &dest);      /* - "" - */
    ip->udp_src  = htons(sport);
    ip->udp_dst  = htons(dport);
    ip->udp_len  = htons(8 + len);
    ip->udp_xsum = 0;
    ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);
}

int
NetEthHdrSize(void)
{
    ushort myvlanid;

    myvlanid = ntohs(NetOurVLAN);
    if (myvlanid == (ushort)-1)
    {
        myvlanid = VLAN_NONE;
    }

    return ((myvlanid & VLAN_IDMASK) == VLAN_NONE) ? ETHER_HDR_SIZE : VLAN_ETHER_HDR_SIZE;
}

int
NetSetEther(volatile uchar * xet, uchar * addr, uint prot)
{
    Ethernet_t *et = (Ethernet_t *)xet;
    ushort myvlanid;

    myvlanid = ntohs(NetOurVLAN);

    if (myvlanid == (ushort)-1)
    {
        myvlanid = VLAN_NONE;
    }
        
    memcpy (et->et_dest, addr, 6);
    memcpy (et->et_src, NetOurEther, 6);

    if ((myvlanid & VLAN_IDMASK) == VLAN_NONE)
    {
        et->et_protlen = htons(prot);
        return ETHER_HDR_SIZE;
    }
    else
    {
        VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)xet;
        vet->vet_vlan_type = htons(PROT_VLAN);
        vet->vet_tag = ((0 << 5) | (myvlanid & VLAN_IDMASK));
        vet->vet_type = htons(prot);
        return VLAN_ETHER_HDR_SIZE;
    }
}

int NetUpdateEther(Ethernet_t *et, uchar *addr, uint prot)
{
    ushort protlen;

    memcpy(et->et_dest, addr, 6);
    memcpy(et->et_src, NetOurEther, 6);
    protlen = ntohs(et->et_protlen);
    if (protlen == PROT_VLAN)
    {
        VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)et;
        vet->vet_type = htons(prot);
        return VLAN_ETHER_HDR_SIZE;
    }
    else if (protlen > 1514)
    {
        et->et_protlen = htons(prot);
        return ETHER_HDR_SIZE;
    }
    else
    {
        /* 802.2 + SNAP */
        struct e802_hdr *et802 = (struct e802_hdr *)et;
        et802->et_prot = htons(prot);
        return E802_HDR_SIZE;
    }
}

int ArpHandler(UINT8 *payload,int size)
{
    volatile UINT8 *pkt_buf; 
    volatile uchar *NetRxPkt;   /* Current receive packet       */
    int     NetRxPktLen;        /* Current rx packet length     */
    Ethernet_t *et;
    VLAN_Ethernet_t *vet;       
    int proto_type;
    IPaddr_t tmp;
    ARP_t   *arp;

    pkt_buf=payload;
    et = (Ethernet_t *)pkt_buf;
    if (size < ETHER_HDR_SIZE)
    {
        return;
    }

    proto_type = ntohs(et->et_protlen);

    if(proto_type==PROT_VLAN)
    {
        vet = (VLAN_Ethernet_t *)payload;
        proto_type=ntohs(vet->vet_type);
        NetRxPkt =payload+VLAN_ETHER_HDR_SIZE;
        NetRxPktLen=size-VLAN_ETHER_HDR_SIZE;
    }
    else
    {
        NetRxPkt =payload+ETHER_HDR_SIZE;
        NetRxPktLen=size-ETHER_HDR_SIZE;
    }

    arp = (ARP_t *)NetRxPkt;
    if (NetRxPktLen < ARP_HDR_SIZE)
    {
        log_printf("bad length %d < %d\n", size, ARP_HDR_SIZE);
        return;
    }
    if (ntohs(arp->ar_hrd) != ARP_ETHER)
    {
        return;
    }
    if (ntohs(arp->ar_pro) != PROT_IP)
    {
        return;
    }
    if (arp->ar_hln != 6)
    {
        return;
    }
    if (arp->ar_pln != 4)
    {
        return;
    }

    if (NetOurIP == 0)
    {
        return;
    }

    if (NetReadIP(&arp->ar_data[16]) != NetOurIP)
    {
        if(NetReadIP(&arp->ar_data[6]) != NetServerIP)
        {
            return;
        }
    }

    switch (ntohs(arp->ar_op))
    {
        case ARPOP_REQUEST:
            /* reply with our IP address    */
            pkt_buf = (uchar *)et;
            pkt_buf += NetSetEther(pkt_buf, et->et_src, PROT_ARP);
            arp->ar_op = htons(ARPOP_REPLY);
            memcpy   (&arp->ar_data[10], &arp->ar_data[0], 6);
            NetCopyIP(&arp->ar_data[16], &arp->ar_data[6]);
            memcpy   (&arp->ar_data[ 0], NetOurEther, 6);
            NetCopyIP(&arp->ar_data[ 6], &NetOurIP);

            (void) net_TXSend((uchar *)et, (pkt_buf - (uchar *)et) + ARP_HDR_SIZE);
            return;

        case ARPOP_REPLY:
            /* arp reply */
            /* are we waiting for a reply */
            log_printf("Got ARP REPLY, set server/gtwy eth addr (%02x:%02x:%02x:%02x:%02x:%02x)\n",
                arp->ar_data[0], arp->ar_data[1],
                arp->ar_data[2], arp->ar_data[3],
                arp->ar_data[4], arp->ar_data[5]);

            tmp = NetReadIP(&arp->ar_data[6]);

            /* matched waiting packet's address */
            if (tmp == NetArpWaitReplyIP)
            {
                /* save address for later use */
                memcpy(NetArpWaitPacketMAC, &arp->ar_data[0], 6);
                memcpy(NetServEther,NetArpWaitPacketMAC,6);
            }
            return;
        default:
            log_printf("Unexpected ARP opcode 0x%x\n", ntohs(arp->ar_op));
            return;
    }
}

void PingHandler(Ethernet_t *et, IP_t *ip, int len)
{
    ICMP_t *icmph = (ICMP_t *)&ip->udp_src;
    IPaddr_t src_ip;
    int eth_hdr_size;

    switch (icmph->type)
    {
        case ICMP_ECHO_REPLY:
            src_ip = NetReadIP((void *)&ip->ip_src);
            if (src_ip == NetServerIP)
            {
                oldPingSeqNo=htons(icmph->un.echo.sequence);
                ping_reply_count++;
                if(ping_times>0)
                {
                    PingSend();
                    ping_times--;
                }
            }
            return;
        case ICMP_ECHO_REQUEST:
            eth_hdr_size = NetUpdateEther(et, et->et_src, PROT_IP);

            ip->ip_sum = 0;
            ip->ip_off = 0;
            NetCopyIP((void *)&ip->ip_dst, &ip->ip_src);
            NetCopyIP((void *)&ip->ip_src, &NetOurIP);
            ip->ip_sum = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP >> 1);

            icmph->type = ICMP_ECHO_REPLY;
            icmph->checksum = 0;
            icmph->checksum = ~NetCksum((uchar *)icmph,(len - IP_HDR_SIZE_NO_UDP) >> 1);

            net_TXSend((uchar *)et, eth_hdr_size + len);
            return;
    }
}

int IPHandler(UINT8 *payload,int size)
{
    IP_t     *ip;
    int      ip_len;
    UINT8    *NetRxPkt;        /* Current receive packet       */
    int      NetRxPktLen;      /* Current rx packet length     */
    IPaddr_t dst_ip;
    IPaddr_t src_ip;
    Ethernet_t *et;
    VLAN_Ethernet_t *vet;       
    int proto_type;

    et = (Ethernet_t *)payload;
    proto_type = ntohs(et->et_protlen);

    if(proto_type==PROT_VLAN)
    {
        vet = (VLAN_Ethernet_t *)payload;
        proto_type=ntohs(vet->vet_type);
        NetRxPkt =payload+VLAN_ETHER_HDR_SIZE;
        NetRxPktLen=size-VLAN_ETHER_HDR_SIZE;
    }
    else
    {
        NetRxPkt =payload+ETHER_HDR_SIZE;
        NetRxPktLen=size-ETHER_HDR_SIZE;
    }

    ip=(IP_t *)NetRxPkt;
    if (NetRxPktLen < IP_HDR_SIZE)
    {
        log_printf ("len bad %d < %d\n", NetRxPktLen, IP_HDR_SIZE);
        return;
    }

    if (NetRxPktLen < ntohs(ip->ip_len))
    {
        log_printf("len bad %d < %d\n", NetRxPktLen, ntohs(ip->ip_len));
        return;
    }
    ip_len = ntohs(ip->ip_len);

    if ((ip->ip_hl_v & 0xf0) != 0x40)
    {
        return;
    }

    if (ip->ip_off & htons(0x1fff))
    {
        /* Can't deal w/ fragments */
        return;
    }

    if (!NetCksumOk((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2))
    {
        return;
    }
    dst_ip = NetReadIP(&ip->ip_dst);
    if (NetOurIP && dst_ip != NetOurIP && dst_ip != 0xFFFFFFFF)
    {
        return;
    }

    src_ip = NetReadIP(&ip->ip_src);

    switch(ip->ip_p)
    {
        case IPPROTO_ICMP:
            ICMPHandler(ip, ip_len, src_ip, et);
            break;
        case IPPROTO_UDP:
            TftpHandler((uchar *)ip +IP_HDR_SIZE,
                        ntohs(ip->udp_dst),
                        ntohs(ip->udp_src),
                        ntohs(ip->udp_len) - 8);
            break;
        default:
            break;
    }
}

void ArpRequest (void)
{
    int i;
    UINT8 *pkt;
    ARP_t *arp;
    Ethernet_t *eth_header;
    int pkt_size=0;

    pkt = (UINT8 *)tx_data;

    memset(tx_data, 0, 1582);

    pkt += NetSetEther (pkt, NetBcastAddr, PROT_ARP);

    arp = (ARP_t *) pkt;

    arp->ar_hrd = htons (ARP_ETHER);
    arp->ar_pro = htons (PROT_IP);

    arp->ar_hln = 6;
    arp->ar_pln = 4;
    arp->ar_op = htons (ARPOP_REQUEST);

    /* source ET addr   */
    memcpy (&arp->ar_data[0], NetOurEther, 6);
    /* source IP addr   */
    NetWriteIP ((uchar *) & arp->ar_data[6], NetOurIP);

    for (i = 10; i < 16; ++i)
    {
        /* dest ET addr = 0 */
        arp->ar_data[i] = 0;
    }

    NetWriteIP ((uchar *) & arp->ar_data[16], NetArpWaitReplyIP);
    net_TXSend(tx_data,pkt-tx_data+ARP_HDR_SIZE);
}

int PingSend(void)
{
    static uchar mac[6];
    volatile IP_t *ip;
    volatile ushort *s;
    uchar *pkt;

    /* XXX always send arp request */
    pkt = NetTxPacket ;

    NetArpWaitPacketIP = NetServerIP;

    pkt += NetSetEther(pkt, mac, PROT_IP);

    ip = (volatile IP_t *)pkt;

    /*  Construct an IP and ICMP header.  (need to set no fragment bit - XXX)
     */
    ip->ip_hl_v  = 0x45;        /* IP_HDR_SIZE / 4 (not including UDP) */
    ip->ip_tos   = 0;
    ip->ip_len   = htons(IP_HDR_SIZE_NO_UDP + 8);
    ip->ip_id    = htons(NetIPID++);
    ip->ip_off   = htons(IP_FLAGS_DFRAG);   /* Don't fragment */
    ip->ip_ttl   = 255;
    ip->ip_p     = 0x01;        /* ICMP */
    ip->ip_sum   = 0;
    NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
    NetCopyIP((void*)&ip->ip_dst, &NetServerIP);       /* - "" - */
    ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);

    s = &ip->udp_src;       /* XXX ICMP starts here */
    s[0] = htons(0x0800);       /* echo-request, code */
    s[1] = 0;           /* checksum */
    s[2] = 0;           /* identifier */
    s[3] = htons(PingSeqNo++);  /* sequence number */
    s[1] = ~NetCksum((uchar *)s, 8/2);
    /* size of the waiting packet */
    /* and do the ARP request */
    udelay(100);
    NetSendPacket(NetTxPacket,NetServEther,0);
    return 1;   /* waiting */
}

int NetSendPacket(uchar *pktbuf,uchar *ether,int len)
{
    uchar *pkt=pktbuf;

    if (memcmp(ether, NetEtherNullAddr, 6) == 0)
    {
        ArpRequest();
        return 1;
    }

    pkt += NetSetEther (pkt, ether, PROT_IP);
    net_TXSend(pktbuf, (pkt - pktbuf) + IP_HDR_SIZE + len);

    return 0;
}

int NetSendUDPPacket(uchar *pktbuf,uchar *ether, IPaddr_t dest, int dport, int sport, int len)
{
    uchar *pkt=pktbuf;

    /* convert to new style broadcast */
    if (dest == 0)
    {
        dest = 0xFFFFFFFF;
    }

    /* if broadcast, make the ether address a broadcast and don't do ARP */
    if (dest == 0xFFFFFFFF)
    {
        ether = NetBcastAddr;
    }

    /* if MAC address was not discovered yet, save the packet and do an ARP request */
    if (memcmp(ether, NetEtherNullAddr, 6) == 0)
    {
        /* waiting */
        ArpRequest();
        return 1;
    }

    pkt += NetSetEther (pkt, ether, PROT_IP);
    NetSetIP (pkt, dest, dport, sport, len);
    net_TXSend(pktbuf, (pkt - pktbuf) + IP_HDR_SIZE + len + 4);
    return 0;   /* transmitted */
}

GT_STATUS netRecvCB
(
    IN UINT8      unit,
    IN UINT8      queueIdx,
    IN UINT32     numOfBuff,
    IN UINT8      *packetBuffs[],
    IN UINT32     buffLen[],
    IN void       *rxParamsPtr
)
{
    volatile UINT8 *pkt_buf = packetBuffs[0];
    UINT8 *NetRxPkt;        /* Current receive packet       */
    int   NetRxPktLen;      /* Current rx packet length     */
    Ethernet_t *et;
    VLAN_Ethernet_t *vet;       
    int proto_type;
    UINT32   size = (buffLen[0]-4);

    if (size < ETHER_HDR_SIZE)
    {
        log_printf("recv pkt size is small\n");
        return;
    }

    if(size > (1518+64))
    {
        log_printf("recv pkt size is large\n");
        return;
    }

    et = (Ethernet_t *)pkt_buf;

    proto_type = ntohs(et->et_protlen);

    if(proto_type==PROT_VLAN)
    {
        vet = (VLAN_Ethernet_t *)pkt_buf;
        proto_type=ntohs(vet->vet_type);
        NetRxPkt =pkt_buf+VLAN_ETHER_HDR_SIZE;
        NetRxPktLen=size-VLAN_ETHER_HDR_SIZE;
    }
    else
    {
        NetRxPkt =pkt_buf+ETHER_HDR_SIZE;
        NetRxPktLen=size-ETHER_HDR_SIZE;
    }
 
    switch(proto_type)
    {
        case PROT_ARP:
            ArpHandler(pkt_buf,size);
            break;
        case PROT_IP:
            IPHandler(pkt_buf,size);
            break;
        default:
            break;
    }

    return E_LB_CHECK_SUCCESS;
}

int net_TXSend(UINT8 *pkt,int size)
{
    if(size < 64)
    {
        size = 64;
    }
    switch_halEthTx(tx_port,pkt,size);   
}

void checkPingLoop(void)
{
    /* receive fd set */
    fd_set allset,rset;
    int i = 0;
    int maxfd = 0;
    struct timeval tv;
    unsigned char chardata[2];
    int n;

    FD_ZERO(&allset);
    FD_SET(0,&allset);

    /* start trigger */
    net_time_out=0;
    while (1) 
    {
        tv.tv_sec = 1;
        tv.tv_usec = 100;

        memcpy(&rset,&allset,sizeof(fd_set));

        if(select(maxfd+1, &rset, NULL, NULL, &tv)>0)
        {
            if(FD_ISSET(0, &rset))
            {
                n=read(0,chardata,1);
                if(chardata[0]=='1')
                {
                    break;
                }
            }
        }

        if(ping_times>0)
        {
            if(PingSeqNo != oldPingSeqNo)
            {
                log_printf(".");
                PingSend();
                ping_times--;
            }
        }
        else
        {
            break;
        }
    }
}

int net_Init(void)
{
    INT32 ret=E_TYPE_SUCCESS;
    memset(NetArpWaitPacketMAC,0,6);
    NetArpWaitPacketIP = 0;
    NetArpWaitReplyIP = 0;

    switch_halCallbackRegister(netRecvCB);

    /* assign system MAC address for net working */
    /* 20161128 -- Fixed MAC address always is default board_mac_addr 
    00:90:e5:00:00:01, if netset mac command */
    if(NetOurEther[0] == 0x0 && NetOurEther[1] == 0x0 && NetOurEther[2] == 0x0)
    {
        switch_halMacAddrGet(NetOurEther);
        log_printf("Default MAC address, %02x:%02x:%02x:%02x:%02x:%02x \n", NetOurEther[0], NetOurEther[1], NetOurEther[2], NetOurEther[3], NetOurEther[4], NetOurEther[5]);
         
    }else {
        log_printf("Netset MAC address, %02x:%02x:%02x:%02x:%02x:%02x \n", NetOurEther[0], NetOurEther[1], NetOurEther[2], NetOurEther[3], NetOurEther[4], NetOurEther[5]);
    }
    
    switch_halSetCpuMac(NetOurEther);
}

int net_clean(void)
{
    memset(NetArpWaitPacketMAC,0,6);
    NetArpWaitPacketIP = 0;
    NetArpWaitReplyIP = 0;
}

void ip_to_string (IPaddr_t x, char *s)
{
    x = ntohl (x);
    sprintf (s, "%d.%d.%d.%d",
         (int) ((x >> 24) & 0xff),
         (int) ((x >> 16) & 0xff),
         (int) ((x >> 8) & 0xff), (int) ((x >> 0) & 0xff)
    );
}

void print_IPaddr (IPaddr_t x)
{
    char tmp[16];

    ip_to_string (x, tmp);

    puts (tmp);
}

int
NetCksumOk(uchar * ptr, int len)
{
    return !((NetCksum(ptr, len) + 1) & 0xfffe);
}

unsigned
NetCksum(uchar * ptr, int len)
{
    ulong   xsum;
    ushort *p = (ushort *)ptr;

    xsum = 0;
    while (len-- > 0)
    {
        xsum += *p++;
    }
    xsum = (xsum & 0xffff) + (xsum >> 16);
    xsum = (xsum & 0xffff) + (xsum >> 16);
    return (xsum & 0xffff);
}

INT32 halStr2MacAddr
(
    IN UINT8 *macStr,
    OUT UINT8 *macAddr
)
{
    UINT32 i;
    UINT32 j;
    UINT8 *pStrMac = macStr;
    UINT8 mac;

    /* Remove ':' from MAC affress */
    UINT8 enet_addr[6*2+1];

    for(i=0,j=0; i < sizeof(enet_addr) ;j++ )
    { 
        if( pStrMac[j] != ':' )
        {
            enet_addr[i] = pStrMac[j];
            i++;
        }
    }

    for(i=0; i < 6 ;i++)
    {
        mac = enet_addr[i*2];
        if( (mac >= '0') && (mac <= '9') ) 
        {
            mac = mac - '0';
        }
        else if( (mac >= 'a') && (mac <= 'f') )
        {
            mac = ( mac - 'a' + 10 );
        }
        else if( (mac >= 'A') && (mac <= 'F') )
        {
            mac = ( mac - 'A' + 10 );
        }
        macAddr[i] = mac << 4;

        mac = enet_addr[i*2+1];
        if( (mac >= '0') && (mac <= '9') ) 
        {
            mac = mac - '0';
        }
        else if( (mac >= 'a') && (mac <= 'f') )
        {
            mac = ( mac - 'a' + 10 );
        }
        else if( (mac >= 'A') && (mac <= 'F') )
        {
            mac = ( mac - 'A' + 10 );
        }
        macAddr[i] += mac;
    }
    return E_TYPE_SUCCESS;
}

INT32 do_netfunc
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    extern void checkTFTPLoop(void);
    int ret=E_TYPE_SUCCESS;

    if( strstr(argv[0], "netinit") )
    {
        net_Init();
        memset(NetServEther, 0, 6);
        memcpy (&NetArpWaitReplyIP, &NetServerIP, sizeof(IPaddr_t));
    }
    else if( strstr(argv[0], "ping") )
    {
        int ping_total_time;
        if( argc < 2 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if(argc >2)
        {
            ping_times=simple_strtoul(argv[2], NULL, 10);
        }
        if(!ping_times)
        {
            ping_times=1;
        }
        ping_total_time=ping_times;
        ping_reply_count=0;
        net_Init();
        NetServerIP=inet_addr(argv[1]);
        memset(NetServEther, 0, 6);

        memcpy (&NetArpWaitReplyIP, &NetServerIP, sizeof(IPaddr_t));

        ArpRequest();
        udelay(100000);

        PingSend();
        ping_times--;

        checkPingLoop();
        log_printf("\nPing packet (%d/%d) : ",ping_reply_count,ping_total_time);

        switch_halInvalidCpuMac(NetOurEther);
        if(ping_reply_count==0)
        {
            log_printf("Fail \n");
            ret = E_TYPE_SOCKET_TIMEOUT;
            goto __CMD_ERROR;
        }
        log_printf("Pass \n");
    }
    else if( strstr(argv[0], "txport") )
    {
        if( argc != 2 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        tx_port=simple_strtoul(argv[1], NULL, 10);
        if( (!tx_port) || (tx_port >port_utilsTotalFixedPortGet()))
        {
            tx_port=1;
        }
        log_printf("TX port %d \n",tx_port);
    }
    else if( strstr(argv[0], "netdeinit") )
    {
        net_clean();
    }
    else if ( strstr(argv[0], "netset") )
    {
        if( (argc < 3) || (argc > 4) )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }

        if ( strstr(argv[1], "ipaddr") )
        {
            NetOurIP=inet_addr(argv[2]);
        }
        else if ( strstr(argv[1], "serverip") )
        {
            NetServerIP=inet_addr(argv[2]);
        }
        else if ( strstr(argv[1], "gatewayip") )
        {
            NetOurGatewayIP=inet_addr(argv[2]);
        }
        else if ( strstr(argv[1], "netmask") )
        {
            NetOurSubnetMask=inet_addr(argv[2]);
        }
        else if ( strstr(argv[1], "uboot") )
        {
            tftpsetfilename(argv[2]);
            tftp_set_flashname(2, argv[3]);
        }
        else if ( strstr(argv[1], "linux") )
        {
            tftpsetfilename(argv[2]);
            tftp_set_flashname(0, argv[3]);
        }
        else if ( strstr(argv[1], "customer") )
        {
            tftpsetfilename(argv[2]);
            tftp_set_flashname(4, argv[3]);
        }
        else if ( strstr(argv[1], "firmware") ) /* 20160630 copy fw images to /tmp */
        {
            tftpsetfilename(argv[2]);
            tftp_set_flashname(3, argv[3]);
        }
        else if ( strstr(argv[1], "mac") )
        {
            ret = halStr2MacAddr((UINT8 *)argv[2], NetOurEther );
            if ( ret != E_TYPE_SUCCESS )
            {
                log_printf("Invalid MAC adddress\n");
                goto __CMD_ERROR;
            }
            log_printf("Program MAC address, %02x:%02x:%02x:%02x:%02x:%02x \n", NetOurEther[0], NetOurEther[1], NetOurEther[2], NetOurEther[3], NetOurEther[4], NetOurEther[5]);
        }        
    }
    else if ( strstr(argv[0], "run") )
    {
        if( argc != 2 )
        {
            ERR_PRINT_CMD_USAGE(argv[0]);
            ret = E_TYPE_INVALID_CMD_FORMAT;
            goto __CMD_ERROR;
        }
        if ( strstr(argv[1], "blup2") )
        {
            net_Init();

            if(NetOurSubnetMask==0)
            {
                NetOurSubnetMask=inet_addr("255.255.255.0");
            }

            /* zero out server ether in case the server ip has changed */
	        memset(NetServEther, 0, 6);
            memcpy (&NetArpWaitReplyIP, &NetServerIP, sizeof(IPaddr_t));

            if((NetArpWaitReplyIP & NetOurSubnetMask) != (NetOurIP& NetOurSubnetMask))
            {
                if (NetOurGatewayIP == 0)
                {
                    log_printf("## Warning: gatewayip needed but not set\n");
                }
                else
                {
                    memcpy (&NetArpWaitReplyIP, &NetOurGatewayIP, sizeof(IPaddr_t));
                }
            }
            ArpRequest();
            udelay(100000);
            TftpStart();
            checkTFTPLoop();
        }
    }
    return ret;
__CMD_ERROR:
    return ret;
}



/* function: do_lsfunc
  * List information about the FILEs in Linux 
  */
INT32 do_lsfunc
(
    IN cmd_tbl_t *  cmdtp,
    IN INT32        flag,
    IN INT32        argc,
    IN INT8 *       argv[]
)
{
    pid_t childPid, waitPid;     
    int ret=E_TYPE_SUCCESS;

    if (argc >= 4)
    {
        ERR_PRINT_CMD_USAGE(argv[0]);
        ret = E_TYPE_INVALID_CMD_FORMAT;
        goto __CMD_ERROR;
    }
 
    childPid = fork();
    if(childPid == 0)
    {
        if(execvp("ls",argv) < 0)
        {
            log_printf("do_lsfunc: execvp ls command failed!!!\n");
            ret = E_TYPE_INVALID_DATA;
            goto __CMD_ERROR;
        }
    }
    do
    {
        waitPid = waitpid(childPid, NULL, 0);
    }while(waitPid == 0);
    
    return ret;
__CMD_ERROR:
    return ret;
}

U_BOOT_CMD(
    run,   CONFIG_SYS_MAXARGS,    1,  do_netfunc,
        "run \t\t- run netfunc parameters.\n",
        "do net function\n"
);

U_BOOT_CMD(
    ls,   CONFIG_SYS_MAXARGS,    1,  do_lsfunc,
        "ls \t\t- list information about the FILEs.\n",
        "run ls to list directory contents.\n"
);

U_BOOT_CMD(
    netset,   CONFIG_SYS_MAXARGS,    1,  do_netfunc,
        "netset \t\t- set netfunc parameters for TFTP upgrade.\n",
        " <ipaddr> | <serverip> | <gatewayip> | <netmask> | <mac> | <uboot> | <linux> | <firmware> \n"
        "  - ipaddr: Specify the device IP address.\n"
        "  - serverip: Specify the TFTP server IP address.\n"
        "  - gatewayip: Specify the gateway IP address.\n"
        "  - netmask: Specify the netmask.\n"
        "  - mac: Specify the device MAC address.\n"
        "  - uboot: Specify uboot filename.\n"
        "  - linux: Specify DIAG filename.\n"
        "  - firmware: Specify firmware filename.\n"
);

U_BOOT_CMD(
    ping,   CONFIG_SYS_MAXARGS,    1,  do_netfunc,
        "ping \t\t- ping specific IP address.\n",
        " <IP Addr>\n"
        "  - IP Addr: Specify the IP address.\n"
);

U_BOOT_CMD(
    txport,   CONFIG_SYS_MAXARGS,    1,  do_netfunc,
        "txport \t\t- Specify the TFTP port.\n",
        " <port>\n"
        "  - port: Specify the port connecte to TFTP server. Valid values are <1-port max|all>.\n"
);
