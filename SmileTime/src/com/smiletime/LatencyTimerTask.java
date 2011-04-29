package com.smiletime;

import java.io.IOException;
import java.io.OutputStream;
import java.util.TimerTask;

public class LatencyTimerTask extends TimerTask {

	OutputStream outStream;
	
	public LatencyTimerTask(OutputStream out){
		outStream = out;
	}
	
	@Override
	public void run() {
		// TODO Auto-generated method stub
		sendLatencyPacketToDesktop();
	}
	


	public void sendLatencyPacketToDesktop() {

    int peerSender = 1;
    int packetType = 5; // Latency packet
    long time_sent = System.currentTimeMillis();
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

    // Send the packet
		try { outStream.write(payload); }
    catch (IOException e) { 
    	
    }
  }

}
