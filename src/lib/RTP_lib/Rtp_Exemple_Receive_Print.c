/*******
 * 
 * FILE INFO:
 * project:	RTP_Lib
 * file:	Rtp_Exemple_Receive_Print.c
 * started on:	05/14/03 16:54:39
 * started by:	Julien Dupasquier <jdupasquier@wanadoo.fr>
 * 
 * 
 * TODO:
 * 
 * BUGS:
 * 
 * UPDATE INFO:
 * updated on:	05/15/03 16:26:37
 * updated by:	Julien Dupasquier <jdupasquier@wanadoo.fr>
 * 
 *******/

#include		<sys/types.h>
#include		<sys/socket.h>

#include		<netinet/in.h>
#include		<arpa/inet.h>

#include		<stdio.h>
#include		<stdlib.h>
#include		<string.h>
#include		<unistd.h>
#include		<err.h>
#include 		<math.h>

#include 		"Config.h"
#include 		"RTP.h"
#include 		"Macros.h"
#include 		"Proto.h"


void			Print_context(char *msg, int len, int cid)
{
  int			i;

  printf("SSRC number                      [%i]\n",context_list[cid]->my_ssrc);
  printf("Number of packets sent           [%i]\n",context_list[cid]->sending_pkt_count);
  printf("Number of bytes sent             [%i]\n",context_list[cid]->sending_octet_count);
  printf("Version                          [%i]\n",context_list[cid]->version);
  printf("Marker flag                      [%i]\n",context_list[cid]->marker);
  printf("Padding length                   [%i]\n",context_list[cid]->padding);
  printf("CSRC length                      [%i]\n",context_list[cid]->CSRClen);
  printf("Payload type                     [%i]\n",context_list[cid]->pt);
  for(i = 0; i < context_list[cid]->CSRClen; i++)
    printf("CSRC list[%i]                     [%li]\n", i, context_list[cid]->CSRCList[i]);
  printf("First value of timestamp         [%i]\n",context_list[cid]->init_RTP_timestamp);
  printf("Current value of timestamp       [%i]\n",context_list[cid]->RTP_timestamp);
  printf("Time elapsed since the beginning [%i]\n",context_list[cid]->time_elapsed);
  printf("First sequence number            [%i]\n",context_list[cid]->init_seq_no);
  printf("Current sequence number          [%i]\n",context_list[cid]->seq_no);
  printf("Extension header Type            [%i]\n",context_list[cid]->hdr_extension->ext_type);
  printf("Extension header Len             [%i]\n",context_list[cid]->hdr_extension->ext_len);
  for(i = 0; i < context_list[cid]->hdr_extension->ext_len; i++)
    printf("Extension header[%i]              [%i]\n", i,context_list[cid]->hdr_extension->hd_ext[i]);
  printf("Message[%i] : [%s]\n\n", len, msg);
}


void			print_hdr(rtp_pkt *pkt)
{
  printf("Header du message :\n");

  printf("Version       [%d]\n", (pkt->RTP_header->flags & 0xd0) >> 6);
  printf("Padding       [%d]\n", (pkt->RTP_header->flags & 0x20) >> 5);
  printf("Ext           [%d]\n", (pkt->RTP_header->flags & 0x10) >> 4);
  printf("Cc            [%d]\n", (pkt->RTP_header->flags & 0x0f));
  printf("marker        [%d]\n", (pkt->RTP_header->mk_pt & 0x10) >> 7);
  printf("PayLoad type  [%d]\n", (pkt->RTP_header->mk_pt & 0x7f));
  printf("sq_nb         [%i]\n", ntohs(pkt->RTP_header->sq_nb));
  printf("ts            [%x]\n", ntohl(pkt->RTP_header->ts));
  printf("ssrc          [%x]\n", ntohl(pkt->RTP_header->ssrc));
  printf("csrc          [%i]\n", ntohl(pkt->RTP_header->csrc[0]));
  printf("ext->type     [%i]\n", ntohs(pkt->RTP_extension->ext_type));
  printf("ext->len      [%i]\n", ntohs(pkt->RTP_extension->ext_len));
  printf("ext[0]        [%i]\n", ntohl(pkt->RTP_extension->hd_ext[0]));
  printf("ext[1]        [%i]\n", ntohl(pkt->RTP_extension->hd_ext[1]));
  printf("len PayLoad   [%i]\n", pkt->payload_len);
  printf("PayLoad       [%s]\n", pkt->payload);
}
