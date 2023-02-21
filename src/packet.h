//
// Author: Jon Trulson <jon@radscan.com>
// Copyright (c) 1994-2018 Jon Trulson
//
// The MIT License
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED


#include "cb.h"
#include "protocol.h"

#define PKT_MAXSIZE     1024	/* no packet should ever be this large. gulp.*/

typedef int (*dispatchProc_t)(char *);

typedef struct _packetent {
    uint32_t        pktid;
    uint32_t        size;
    const char      *name;
    dispatchProc_t dispatch;
} packetEnt_t;

/* this will be implemented in cproc.c and sproc.c to initialize the
 *  (*dispatch) member of the server/clientPackets[] arrays in packet.c
 */
int procDispatchInit(uint16_t vers, packetEnt_t *pktList, int numpkts);

/* input/output ring buffer sizes. TCP RB's are larger to handle cases
 * where UDP is not available and the TCP RB is used instead.
 */
#define PKT_UDP_RB_MAX    (PKT_MAXSIZE * 64)
#define PKT_TCP_RB_MAX    (PKT_UDP_RB_MAX * 2)

/* for pktWrite, whether to send via UDP (if available) */
#define PKT_SENDTCP    0
#define PKT_SENDUDP    1

/* special value for pktSetSocketFds to ignore a set */
#define PKT_SOCKFD_NOCHANGE  (-2)

/* error/severity codes for Acks, should make sure these sync to
   pktSeverity2String(int psev) */
#define PSEV_INFO     0
#define PSEV_WARN     1
#define PSEV_ERROR    2
#define PSEV_FATAL    3

/* error codes */
#define PERR_OK           0	/* no error */
#define PERR_UNSPEC       1	/* unspecified */
#define PERR_BADPROTO     2	/* bad protocol */
#define PERR_BADCMN       3	/* common block mismatch */
#define PERR_INVUSER      4 	/* invalid username */
#define PERR_NOUSER       5 	/* no such user */
#define PERR_INVPWD       6 	/* invalid password */
#define PERR_BADPWD       7 	/* wrong password */
#define PERR_CLOSED       8 	/* game closed */
#define PERR_REGISTER     9 	/* register failed */
#define PERR_NOSHIP       10 	/* no slots available */
#define PERR_LOSE         11 	/* lose in menu() */
#define PERR_FLYING       12 	/* already flying a ship (newship()) */
#define PERR_TOOMANYSHIPS 13	/* *obsolete* available for re-use */
#define PERR_CANCELED     14	/* an operation (bombing, etc) was cancelled
				   for some reason. */
#define PERR_DONE         15    /* finished something - like beaming */
#define PERR_IDLETIMEOUT  16	/* timed out in menu (5+ mins idle) */
#define PERR_PINGRESP     17	/* a ping reponse for nCP */

typedef struct _pkt_stats {
    unsigned int  rxBytes;        /* tx/rx byte counts */
    unsigned int  txBytes;
    unsigned int  pingAvg;        /* in ms */

    // UDP info
    unsigned int duplicatePackets;
    unsigned int oooPackets;
    unsigned int lostPackets;
} pktStats_t;

#ifdef NOEXTERN_PACKET
pktStats_t          pktStats = {};
packetEnt_t         *serverPackets = NULL;
int                 serverPktMax = 0;
#else
extern pktStats_t   pktStats;
extern packetEnt_t  *serverPackets;
extern int          serverPktMax;
#endif

#define PKT_PROCSP(_pkt)                                        \
    ( (*serverPackets[((uint8_t)(_pkt)[0])].dispatch)((_pkt)) )

int   pktInit(void);
void  pktSetClientMode(int isclient);
uint16_t pktGetClientProtocolVersion();
int   pktSetClientProtocolVersion(uint16_t vers);
void  pktSetSocketFds(int tcpsock, int udpsock);

void  pktSendAckUDP(int socktype, uint8_t state, uint32_t payload);
int   pktSendAck(uint8_t severity, uint8_t code);
int   pktIsConnDead(void);
int   pktNotImpl(char *nothing);

void  pktSetNodelay(void);

const char *pktSeverity2String(int psev);

int   pktClientPacketSize(int type);
int   pktServerPacketSize(int type);

int   pktSocketHasData(int sock);
int   pktWrite(int socktype, void *data);
int   pktReadPacketReady(void);
int   pktRead(char *buf, int len, unsigned int delay);
int   pktIsValid(int pkttype, void *pkt);
void  pktSetNonBlocking(int s, bool enable);

bool  pktNoNetwork(void);
void  pktCloseNetwork(void);

#endif /* PACKET_H_INCLUDED */
