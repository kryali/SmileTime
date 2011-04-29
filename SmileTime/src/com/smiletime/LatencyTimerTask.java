package com.smiletime;

import java.io.IOException;
import java.io.OutputStream;
import java.util.TimerTask;

import android.util.Log;

public class LatencyTimerTask extends TimerTask {

	OutputStream outStream;
	private String tag = "LatencyTimerTask";
	
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
    int time_sent2 = time_sent%Integer.MAX_VALUE;
		//byte[] payload = new byte[16];
		byte[] payload = new byte[12];

    // Construct the latency packet
    payload[3] = (byte)(packetType >>> 24);
    payload[2] = (byte)(packetType >>> 16);
    payload[1] = (byte)(packetType >>> 8);
    payload[0] = (byte)(packetType);

    payload[7] = (byte)(peerSender >>> 24);
    payload[6] = (byte)(peerSender >>> 16);
    payload[5] = (byte)(peerSender >>> 8);
    payload[4] = (byte)(peerSender);

    /*payload[15] = (byte)(time_sent >>> 56);
    payload[14] = (byte)(time_sent >>> 48);
    payload[13] = (byte)(time_sent >>> 40);
    payload[12] = (byte)(time_sent >>> 32);
    */
    payload[11] = (byte)(time_sent2 >>> 24);
    payload[10] = (byte)(time_sent2 >>> 16);
    payload[9]  = (byte)(time_sent2 >>> 8);
    payload[8]  = (byte)(time_sent2);
    Log.d(tag, "TEST = " + time_sent2); 
    // Send the packet
		try { outStream.write(payload); }
    catch (IOException e) { 
    	
    }
  }

}
