/*******
 *
 * FILE INFO:
 * project:	RTP_lib
 * file:	Rtp_Exemple_Send.c
 * started on:	03/26/03
 * started by:	Cedric Lacroix <lacroix_cedric@yahoo.com>
 *
 *
 * TODO:
 *
 * BUGS:
 *
 * UPDATE INFO:
 * updated on:	05/14/03 17:11:47
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


#include 		"Config.h"
#include 		"RTP.h"
#include 		"Types.h"
#include 		"Proto.h"





int			main(void)
{
	char		buffer[MAX_PAYLOAD_LEN];
	context		cid;
	u_int32		period;
	u_int32		t_inc;
	u_int16		size_read;
	u_int16		last_size_read;
	FILE		*fd;


	conx_context_t 	 *coucou = NULL;
	remote_address_t *s	 = NULL;



	period = Get_Period_us(PAYLOAD_TYPE);
	last_size_read = 1;

	Init_Socket();
	RTP_Create(&cid);
	RTP_Add_Send_Addr(cid, "192.168.0.94", UDP_PORT, 6);

	Set_Extension_Profile(cid, 27);
	Add_Extension(cid, 123456);
	Add_Extension(cid, 654321);
	Add_CRSC(cid, 12569);

	fd = fopen("coucou.txt", "r");
	while (size_read = fread(buffer, 1, MAX_PAYLOAD_LEN, fd))
	{
		t_inc = last_size_read * period;
		RTP_Send(cid, t_inc, 0, PAYLOAD_TYPE, buffer, size_read);
		last_size_read = size_read;
	}

	fclose(fd);
	RTP_Destroy(cid);
	Close_Socket();
	return (0);
}
