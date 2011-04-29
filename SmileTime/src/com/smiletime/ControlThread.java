package com.smiletime;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.util.Log;

public class ControlThread extends Thread{
	
	InputStream in;
	OutputStream out;
	private final  int latencyPacketType = 5;
	String tag = "Control";
	
	public ControlThread(InputStream input, OutputStream output){
		in = input;
		out = output;
	}
	
	

    private int byteToInt(byte[] input, int offset){
    	return ((input[3+offset] << 24) | (input[2+offset] << 16) | (input[1+offset] << 8 )| input[0+offset]);
    }
  
	public void run(){
		byte[] packetTypeArr = new byte[4];
		byte[] peer_sender = new byte[4];
		byte[] time_packet = new byte[8];
		try {
			in.read(packetTypeArr);
			int packetType = byteToInt(packetTypeArr, 0);
			switch (packetType) {
			case latencyPacketType:
				Log.d(tag,"Got a latency packet");
				in.read(peer_sender);
				int sender = ((peer_sender[3] << 24) | (peer_sender[2] << 16) | (peer_sender[1] << 8 )| peer_sender[0]);
				
				in.read(time_packet);
				long time = ((time_packet[7] << 56) | (time_packet[6] << 48) | (time_packet[5] << 40 )| time_packet[4] << 32 |
				            (time_packet[3] << 24) | (time_packet[2] << 16) | (time_packet[1] << 8 )| time_packet[0]);
				
				if( sender == 0){
					// Desktop to Mobile
				} else if (sender == 1){
					// Mobile to Desktop
				}
				Log.d(tag, "TIME: " + time);
				
				break;

			default:
				break;
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
	}
}
