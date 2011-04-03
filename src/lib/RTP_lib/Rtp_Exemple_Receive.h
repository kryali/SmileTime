/*******
 * 
 * FILE INFO:
 * project:	RTP_Lib
 * file:	Rtp_Exemple_Receive.c
 * started on:	05/01/03 15:17:42
 * started by:	Julien Dupasquier <jdupasquier@wanadoo.fr>
 * 
 * 
 * TODO:
 * 
 * BUGS:
 * 
 * UPDATE INFO:
 * updated on:	05/14/03 16:20:19
 * updated by:	Julien Dupasquier <jdupasquier@wanadoo.fr>
 * 
 *******/

typedef struct
{
  struct sockaddr	*add;
  int			len;
  int			sid;
} t_client;



typedef struct
{
  int			fd;
  struct sockaddr	*add;
  int			len;
  t_client		*clients;
  int			family;
  int			type;
  int			port;
} t_listener;


struct us
{
  fd_set		fdset;
  t_listener		*listeners;
};


