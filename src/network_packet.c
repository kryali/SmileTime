#include "network_packet.h"

HTTP_packet* create_HTTP_packet(int length)
{
	HTTP_packet* p = malloc(sizeof(HTTP_packet));
	p->length = 1+length;
	p->message = malloc(1+length);
	return p;
}

void destroy_HTTP_packet(HTTP_packet* packet)
{
	free(packet->message);
	free(packet);
}

// converts a control_packet into a HTTP_packet
HTTP_packet* control_to_network_packet(control_packet* packet)
{
	int length = sizeof(control_packet);
	HTTP_packet* network_packet = create_HTTP_packet(length);
	((char*)network_packet->message)[0] = CONTROL_PACKET;
	memcpy((network_packet->message)+1, packet, length);
  printf("Audio: %d\n", packet->audio_codec_ctx.codec_id);
  printf("Video: %d\n", packet->video_codec_ctx.codec_id);
	return network_packet;
}

// converts a pantilt_packet into a HTTP_packet
HTTP_packet* pantilt_to_network_packet(pantilt_packet* packet)
{
	int length = sizeof(pantilt_packet);
	HTTP_packet* network_packet = create_HTTP_packet(length);
	((char*)network_packet->message)[0] = PANTILT_PACKET;
	memcpy((network_packet->message)+1, packet, length);
	return network_packet;
}

// converts a av_packet into a HTTP_packet
HTTP_packet* av_to_network_packet(av_packet* packet)
{
	int length1 = sizeof(av_packet);
	int length2 = packet->av_data.size;//sizeof(packet->av_data.data);
	HTTP_packet* network_packet = create_HTTP_packet(length1 + length2);
	((char*)network_packet->message)[0] = AV_PACKET;
	memcpy((network_packet->message)+1, packet, length1);
	memcpy((network_packet->message)+1+length1, packet->av_data.data, length2);
	return network_packet;
}


//network packets -> data structs
char get_packet_type(HTTP_packet* network_packet)
{
	char packetType = ((char*)network_packet->message)[0];
	return packetType;
}

control_packet* to_control_packet(HTTP_packet* network_packet)
{
	int length = sizeof(control_packet);
	control_packet* cp = malloc(length);
	memcpy(cp, network_packet->message + 1, length);
  cp->audio_codec_ctx.codec = &cp->audio_codec;
  cp->video_codec_ctx.codec = &cp->video_codec;
  printf("TO_PACKET Audio: %d\n", cp->audio_codec_ctx.codec_id);
  printf("TO_PACKET Video: %d\n", cp->video_codec_ctx.codec_id);
	return cp;
}

pantilt_packet* to_pantilt_packet(HTTP_packet* network_packet)
{
	int length = sizeof(pantilt_packet);
	pantilt_packet* pt = malloc(length);
	memcpy(pt, network_packet->message + 1, length);
	return pt;
}

av_packet* to_av_packet(HTTP_packet* network_packet)
{
	av_packet* av = (av_packet*) malloc(sizeof(av_packet));
	memcpy(av, network_packet->message + 1, sizeof(av_packet));
//	av->av_data.data = malloc(av->av_data.size);
	uint8_t * data = malloc(av->av_data.size);
	printf("av->av_data.size %d\n", av->av_data.size);
	printf("av->av_data.data 0x%x\n", av->av_data.data);
	av->av_data.data = data;
	printf("(malloc'd) av->av_data.data 0x%x\n", av->av_data.data);
	memcpy(av->av_data.data, network_packet->message + 1 + sizeof(av_packet), av->av_data.size);
	return av;
}

pantilt_packet* generate_pantilt_packet(int type, int distance)
{
	pantilt_packet* pt = malloc(sizeof(pantilt_packet));
	pt->type = type;
	pt->distance = distance;
	return pt;
}

pantilt_packet* generate_pan_packet(int distance)
{
	return generate_pantilt_packet(PAN, distance);
}

pantilt_packet* generate_tilt_packet(int distance)
{
	return generate_pantilt_packet(TILT, distance);
}
