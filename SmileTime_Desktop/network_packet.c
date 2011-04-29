#include "network_packet.h"

HTTP_packet* create_HTTP_packet(int length)
{
	HTTP_packet* p = malloc(sizeof(HTTP_packet));
	p->length = length;
	p->message = malloc(length);
	return p;
}

void destroy_HTTP_packet(HTTP_packet* packet)
{
	if(packet != NULL){
		free(packet->message);
		free(packet);
	}
}

// converts a control_packet into a HTTP_packet
HTTP_packet* control_to_network_packet(control_packet* packet)
{
	int length = sizeof(control_packet);
	HTTP_packet* network_packet = create_HTTP_packet(1+length);
	((char*)network_packet->message)[0] = CONTROL_PACKET;
	memcpy((network_packet->message)+1, packet, length);
	return network_packet;
}

// converts a pantilt_packet into a HTTP_packet
HTTP_packet* pantilt_to_network_packet(pantilt_packet* packet)
{
	int length = sizeof(pantilt_packet);
	HTTP_packet* network_packet = create_HTTP_packet(length);
	memcpy(network_packet->message, packet, length);
	return network_packet;
}

// converts a av_packet into a HTTP_packet
HTTP_packet* av_to_network_packet(av_packet* packet, void* data)
{
	int length0 = sizeof(av_packet);
	int length1 = packet->length;
	HTTP_packet* network_packet = create_HTTP_packet(length0 + length1);
	memcpy((network_packet->message), packet, length0);
	memcpy((network_packet->message)+length0, data, length1);
	return network_packet;
}

//network packets -> data structs
int get_packet_type(HTTP_packet* network_packet)
{
	return ((int*)network_packet->message)[0];
}

control_packet* to_control_packet(HTTP_packet* network_packet)
{
	int length = sizeof(control_packet);
	control_packet* cp = malloc(length);
	memcpy(cp, network_packet->message, length);
	return cp;
}

pantilt_packet* to_pantilt_packet(HTTP_packet* network_packet)
{
	int length = sizeof(pantilt_packet);
	pantilt_packet* pt = malloc(length);
	memcpy(pt, network_packet->message, length);
	return pt;
}

text_packet* to_text_packet(HTTP_packet* network_packet)
{
	int length = sizeof(text_packet);
	text_packet* tp = malloc(length);
	memcpy(tp, network_packet->message, length);
	return tp;
}

latency_packet* to_latency_packet(HTTP_packet* network_packet)
{
	int length = sizeof(latency_packet);
	latency_packet* lp = malloc(length);
	memcpy(lp, network_packet->message, length);
	return lp;
}

av_packet* to_av_packet(HTTP_packet* network_packet)
{
	av_packet* av = malloc(sizeof(av_packet));
	memcpy(av, network_packet->message, sizeof(av_packet));
	//av->buff.start = malloc(av->buff.length);
	//memcpy(av->buff.start, network_packet->message + sizeof(av_packet), av->buff.length);
	return av;
}

pantilt_packet* generate_pantilt_packet(int type, int pan, int tilt)
{
	pantilt_packet* pt = malloc(sizeof(pantilt_packet));
	pt->packetType = PANTILT_PACKET;
	pt->pan = pan;
	pt->tilt = tilt;
	return pt;
}
