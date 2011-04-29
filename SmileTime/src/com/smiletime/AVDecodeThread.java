package com.smiletime;

import java.net.DatagramPacket;
import java.net.DatagramSocket;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class AVDecodeThread extends Thread {

	private int recPort = 1339;
	private int maxPacketSize = 20000;
	private int bufferSize = 4160;
	private AudioTrack mTrack;
	private String tag = "VDT";
	Handler mHandler;
	
	public AVDecodeThread(Handler h) {
		mHandler = h;
		mTrack = new AudioTrack(AudioManager.STREAM_VOICE_CALL, 11025, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
		if(mTrack.getState() == AudioTrack.STATE_INITIALIZED)
			Log.d(tag, "AudioTrack is initialized");
		mTrack.play();
	}
	
	public void run(){
		try{
			Log.d("VDT", "Listening for UDP packets...");
			DatagramSocket s = new DatagramSocket(recPort);
			byte[] data = new byte[maxPacketSize];
			DatagramPacket p = new DatagramPacket(data, maxPacketSize);
			while(true){
				s.receive(p);
				int packetType = ((data[3] << 24) | (data[2] << 16) | (data[1] << 8 )| data[0]);

				if(packetType == 3){
					// This is a video packet
					Log.d("VDT", "Recieved a video packet!");
					Message msg = new Message();
					Bundle b = new Bundle();
					b.putByteArray("packet", data);
					msg.setData(b);
					//msg.
					mHandler.sendMessage(msg);
				} else if (packetType == 2 ) {
					mTrack.write(data, 12, bufferSize);
				}
			}
		} catch (Exception e){
			
		}
	}
}
