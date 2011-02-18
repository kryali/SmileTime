#include "audio_record.h"

int microphone_fd = -1;
char *microphone_name = "/dev/dsp";
short *audio_buf;
int audio_buf_size;
uint8_t *audio_outbuf;
int audio_outbuf_size;
AVPacket audio_pkt;

int channels = 1;
int sample_bits = 16; //bits per sample
int sample_rate = 44100; //samples per second
int sample_size; //size of a sample in bytes
int audio_input_frame_size; //number of samples per frame

void audio_record_init()
{
	printf("[A_REC] This function initializes the audio device for recording\n");
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
	audio_input_frame_size = audio_context->frame_size;
	sample_size = channels * sample_bits/8;

  	audio_buf_size = audio_input_frame_size * sample_size;
	audio_buf =  avmalloc(audio_buf_size);
}

void audio_segment_copy()
{	
	if( (read( microphone_fd, audio_buf, audio_buf_size )) != audio_buf_size )
		perror("audio_segment_copy read: ");
}

void audio_segment_compress(AVStream *st)
{
    av_init_packet(&audio_pkt);

    audio_pkt.size= avcodec_encode_audio(audio_context, audio_outbuf, audio_outbuf_size, samples);

    if (audio_context->coded_frame && audio_context->coded_frame->pts != AV_NOPTS_VALUE)
        audio_pkt.pts= av_rescale_q(audio_context->coded_frame->pts, audio_context->time_base, st->time_base);
    audio_pkt.flags |= AV_PKT_FLAG_KEY;
    audio_pkt.stream_index= st->index;
    audio_pkt.data= audio_outbuf;
}

void audio_packet_write(AVFormatContext *oc)
{
	/* write the compressed frame in the media file */
	if (av_interleaved_write_frame(oc, &audio_pkt) != 0) {
		fprintf(stderr, "Error while writing audio frame\n");
		exit(1);
	}
}

//Frees all memory and closes codecs.
void audio_close()
{
	free(audio_outbuf);	
	free(audio_buf);
	avcodec_close(audio_context);
	av_free(audio_context);
}

AVStream *add_audio_stream(AVFormatContext *oc, enum CodecID codec_id)
{
	AVStream *st;
	st = av_new_stream(oc, 1);
	if (!st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}

	c = st->codec;
	c->codec_id = codec_id;
	c->codec_type = AVMEDIA_TYPE_AUDIO;

	// Initialize the sample context
	audio_context = avcodec_alloc_context();	
	audio_context->sample_fmt = SAMPLE_FMT_S16;
	audio_context->sample_rate = sample_rate;
	audio_context->channels = 1;
	//audio_context->bit_rate = 64000;

	// some formats want stream headers to be separate
	if(oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
		
	return st;
}

void open_audio(AVFormatContext *oc)
{
	/* find the audio encoder */
	audio_codec = avcodec_find_encoder(c->codec_id);
	if (!audio_codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}

	/* open it */
	if (avcodec_open(audio_context, audio_codec) < 0) {
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}
    
	audio_outbuf_size = 10000;
	audio_outbuf = av_malloc(audio_outbuf_size);
}
