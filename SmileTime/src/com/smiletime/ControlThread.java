package com.smiletime;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class ControlThread extends Thread{

	InputStream in;
	OutputStream out;
	private final  int latencyPacketType = 5;
	private final int messagePacketType = 4;
	private Handler handle;
	String tag = "Control";

	public ControlThread(InputStream input, OutputStream output, Handler h){
		in = input;
		out = output;
		handle = h;
	}	

	private int byteToInt(byte[] input, int offset){
		return ((input[3+offset] << 24) | (input[2+offset] << 16) | (input[1+offset] << 8 )| input[0+offset]);
	}

	public void run(){
		byte[] packetTypeArr = new byte[4];
		byte[] peer_sender = new byte[4];
		//byte[] time_packet = new byte[8];
		byte[] time_packet = new byte[4];
		byte[] message_packet = new byte[140];
		//byte[] payload = new byte[16];
		byte[] payload = new byte[12];

		while(true){
			try {
				in.read(packetTypeArr);
				int packetType = byteToInt(packetTypeArr, 0);
				switch (packetType) {
				case latencyPacketType:
					//Log.d(tag,"Got a latency packet");
					in.read(peer_sender);
					int sender = ((peer_sender[3] << 24) | (peer_sender[2] << 16) | (peer_sender[1] << 8 )| peer_sender[0]);

					in.read(time_packet);

					//int sent_time = ((time_packet[7] << 56) | (time_packet[6] << 48) | (time_packet[5] << 40 )| time_packet[4] << 32
					//		| (time_packet[3] << 24) | (time_packet[2] << 16) | (time_packet[1] << 8 )| time_packet[0]);

					int sent_time = byteToInt(time_packet,0);// DISCREPENCY here.  bytes are not the same as the ones sent.
					if( sender == 0){
							// Desktop to Mobile
							//sendLatencyPacketToDesktop(packetType, sender, time);
							//for(int i = 0 ; i < 16; i++){
							for(int i = 0 ; i < 12; i++){
								if(i < 4){
									payload[i] = packetTypeArr[i];
								} else if( i < 8) {
									payload[i] = peer_sender[i-4];
								} else {
									payload[i] = time_packet[i-8];
								}
							}
								out.write(payload);
								//Log.d(tag, "Sending latency packet");
						} else if (sender == 1){
							// Mobile to Desktop
							for(int i = 0 ; i < 8; i++){
								if(i < 4){
									payload[i] = packetTypeArr[i];
								} else if( i < 8) {
									payload[i] = peer_sender[i-4];
                }
              }
              sent_time = ((time_packet[3] << 24) | (time_packet[2] << 16) | (time_packet[1] << 8 )| time_packet[0]);
              long now = System.currentTimeMillis();
              int now2 = (int) (now%Integer.MAX_VALUE);//breakpoint here
							//Log.d(tag, "NOW = " + now);
							//Log.d(tag, "NOW = " + now2);
							//Log.d(tag, "SENT_TIME " + sent_time);
              int latency = now2 - sent_time;
							//Log.d(tag, "Sending MOBILE latency packet " + latency);

              // Create the payload and send
              payload[8] = (byte)(latency);
              payload[9] = (byte)(latency >>> 8);
              payload[10] = (byte)(latency >>> 16);
              payload[11] = (byte)(latency >>> 24);
              /*payload[12] = (byte)(latency >>> 32);
              payload[13] = (byte)(latency >>> 40);
              payload[14] = (byte)(latency >>> 48);
              payload[15] = (byte)(latency >>> 56);
              */

							out.write(payload);
						}
            else
							//Log.d(tag, "Sender not 0 or 1 ... instead its " + sender);


					break;
				case messagePacketType:
					//Log.d(tag,"Got a message packet");
					in.read(message_packet);

					Message msg = new Message();
					Bundle b = new Bundle();
					b.putByteArray("message", message_packet);
					msg.setData(b);
					handle.sendMessage(msg);
					break;
				default:
					break;
				}
			} catch (IOException e) {

			}
		}

	}


	public void sendLatencyPacketToDesktop(int packetType, int peerSender, long time_sent) {

		byte[] payload = new byte[16];

		// Construct the latency packet
		payload[3] = (byte)(packetType >>> 24);
		payload[2] = (byte)(packetType >>> 16);
		payload[1] = (byte)(packetType >>> 8);
		payload[0] = (byte)(packetType);

		payload[7] = (byte)(peerSender >>> 24);
		payload[6] = (byte)(peerSender >>> 16);
		payload[5] = (byte)(peerSender >>> 8);
		payload[4] = (byte)(peerSender);

		payload[15] = (byte)(time_sent >>> 56);
		payload[14] = (byte)(time_sent >>> 48);
		payload[13] = (byte)(time_sent >>> 40);
		payload[12] = (byte)(time_sent >>> 32);
		payload[11] = (byte)(time_sent >>> 24);
		payload[10] = (byte)(time_sent >>> 16);
		payload[9]  = (byte)(time_sent >>> 8);
		payload[8]  = (byte)(time_sent);

		try { out.write(payload); }
		catch (IOException e) { 
		}
	}
}
