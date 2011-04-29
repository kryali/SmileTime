package com.smiletime;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;

/**
 * AVConnect is the class responsible for connecting sockets to A/V encoding and decoding
 * @author kiranryali
 *
 */
public class AVConnect extends Activity {
	private 	Socket avclient;
	private String name;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    	setContentView(R.layout.avconnect);
    	Intent i = getIntent();
    	Bundle b = i.getExtras();
    	name = b.getString("name");
    	fetchUser();
    }
   
    
    public void destroy(){
    	try {
			avclient.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }
    
    public void viewInit(){
    }
    
    
    
    // This shit doesn't work
    public byte[] intToByte(int value){
    	return new byte[] {
                (byte)(value >>> 24),
                (byte)(value >>> 16),
                (byte)(value >>> 8),
                (byte)value};
    }
    
    public byte[] stringToByte(String s){
    	byte[] ret = new byte[s.length()];
    	for(int i = 0; i < s.length(); i++){
    		ret[i] = (byte) s.charAt(i);
    	}
    	return ret;
    }
    
    public int byteToInt(byte[] input){
    	return ((input[3] << 24) | (input[2] << 16) | (input[1] << 8 )| input[0]);
    }
  
    
    public void fetchUser(){
    	//InetAddress dstAddress = new InetSocketAddress(, 8081);
    	try {
    		int port = 1337;
    		avclient = new Socket("173.230.140.232", port);
    		
    		// Send the FIND byte
    		OutputStream out = avclient.getOutputStream();
    		byte[] buffer = new byte[1];
    		buffer[0]= 0;
    		out.write(buffer);
    		out.flush();
    		
    		// Send the size of the name as well as the name
    		
    		//buffer = intToByte(1);
    		
    		buffer = new byte[4];
    		buffer[0] = (byte) name.length(); // only accepts 255 byte 
    		out.write(buffer);
    		out.write(stringToByte(name));
    		
    		
    		InputStream in = avclient.getInputStream();
    		
    		buffer = new byte[4];
    		in.read(buffer);
    		int size = byteToInt(buffer);
    		byte[] buffer2 = new byte[size];
    		in.read(buffer2);
    		String listStr = new String(buffer2);
    		
    		User u = new User(listStr);
    		
        	TextView connect = (TextView) findViewById(R.id.status);
        	connect.setText("User information received from server:" + u.toString());
    		//in.read();
    	} catch (Exception e) {
    		e.printStackTrace();
        	TextView connect = (TextView) findViewById(R.id.status);
        	connect.setText("[FAIL]" + e);
		}

    }
}
