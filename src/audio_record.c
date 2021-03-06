#include "audio_record.h"
#include "recorder_server.h"
#include "include.h"

extern int bytes_sent;
extern pthread_mutex_t bytes_sent_mutex;

int microphone_fd = -1;
char *microphone_name = "/dev/dsp";

short *audio_buf;
int audio_buf_size;

uint8_t *audio_outbuf;
int audio_outbuf_size;

AVPacket audio_pkt;
AVFormatContext *output_context;

int channels = 1;
int sample_bits = 16; //bits per sample
int sample_rate = 44100; //samples per second
int sample_size; //size of a sample in bytes
int audio_input_frame_size; //number of samples per frame

void audio_record_init(AVOutputFormat *fmt, AVFormatContext *oc)
{
	// Open the microphone device
	microphone_fd = open( microphone_name, O_RDWR );
	if(microphone_fd == -1){
		perror("Opening microphone");
		exit(1);
	}
	// Set sampling parameters
	if( ioctl( microphone_fd, SOUND_PCM_WRITE_BITS, &sample_bits ) == -1){
		perror("Write bits failed");
		exit(1);
	}	
	if( ioctl( microphone_fd, SOUND_PCM_WRITE_RATE, &sample_rate ) == -1){
		perror("rate failed");
		exit(1);
	}
	if( ioctl( microphone_fd, SOUND_PCM_WRITE_CHANNELS, &channels ) == -1){
		perror("channels failed");
		exit(1);
	}

	audio_st = NULL;
	output_context = oc;
	if (fmt->audio_codec != CODEC_ID_NONE) {
		add_audio_stream(fmt->audio_codec);
	}
	if (audio_st) {
		open_audio();
	}
	
	audio_input_frame_size = audio_context->frame_size;
	sample_size = channels * sample_bits/8;

  	audio_buf_size = audio_input_frame_size * sample_size;
	audio_buf =  av_malloc(audio_buf_size);

	audioq = malloc(sizeof(RecorderPacketQueue));
	recorder_packet_queue_init(audioq);
}

void audio_segment_copy()
{	
	if( (read( microphone_fd, audio_buf, audio_buf_size )) != audio_buf_size )
		perror("audio_segment_copy read: ");
}

void audio_segment_compress()
{
	av_init_packet(&audio_pkt);

	audio_pkt.size= avcodec_encode_audio(audio_context, audio_outbuf, audio_outbuf_size, audio_buf);

	if (audio_context->coded_frame && audio_context->coded_frame->pts != AV_NOPTS_VALUE){
		printf("pts\n");
		audio_pkt.pts= av_rescale_q(audio_context->coded_frame->pts, audio_context->time_base, audio_st->time_base);
	}
	audio_pkt.flags |= AV_PKT_FLAG_KEY;
	audio_pkt.stream_index= audio_st->index;
	audio_pkt.data= audio_outbuf;
}

void audio_segment_queue()
{
	recorder_packet_queue_put(audioq, &audio_pkt);
}

AVPacket net_pkt;
void audio_segment_write()
{
	//if(audioq->size > 0)
		//printf("audioqsize: %d\n", audioq->size);

	if(recorder_packet_queue_get(audioq, &net_pkt) == 1)
	{
	  // Transmit the audio packet
		av_packet av;
		av.av_data = net_pkt;
		HTTP_packet* http = av_to_network_packet(&av);

		int size = http->length-1;
		if( write(audiofd, &size, sizeof(size)) ==0){
			perror("audio:write");
		}

		xwrite(audiofd, http);

    // Track bandwidth
    pthread_mutex_lock(&bytes_sent_mutex);
    bytes_sent += http->length;
    pthread_mutex_unlock(&bytes_sent_mutex);

		destroy_HTTP_packet(http);
	}//yo dawg, do we have to free the avpacket's data here?
}

//Frees all memory and closes codecs.
void audio_close()
{
	free(audioq);
	free(audio_outbuf);	
	free(audio_buf);
	avcodec_close(audio_context);
	av_free(audio_context);
}

void add_audio_stream(enum CodecID codec_id)
{
	audio_st = av_new_stream(output_context, 1);
	if (!audio_st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}
	audio_context = audio_st->codec;	
	//audio_context->time_base = (AVRational){1, 25};
	audio_context->codec_id = codec_id;
	audio_context->codec_type = AVMEDIA_TYPE_AUDIO;
	audio_context->sample_fmt = SAMPLE_FMT_S16;
	audio_context->sample_rate = sample_rate;
	audio_context->channels = channels;
	audio_context->bit_rate = 64000;
	
	// some formats want stream headers to be separate
	if(output_context->oformat->flags & AVFMT_GLOBALHEADER)
		audio_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

void open_audio()
{
	/* find the audio encoder */
	audio_codec = avcodec_find_encoder(audio_context->codec_id);
	if (!audio_codec) {
		fprintf(stderr, "audio codec not found\n");
		exit(1);
	}

	/* open it */
	if (avcodec_open(audio_context, audio_codec) < 0) {
		fprintf(stderr, "could not open audio codec\n");
		exit(1);
	}
    
	audio_outbuf_size = 10000;
	audio_outbuf = av_malloc(audio_outbuf_size);
}
