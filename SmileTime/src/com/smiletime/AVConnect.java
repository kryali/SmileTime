package com.smiletime;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class AVConnect extends Activity {

	private 	Socket avclient;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    	setContentView(R.layout.avconnect);
    	connect();
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
    
    public void connect(){
    	//InetAddress dstAddress = new InetSocketAddress(, 8081);
    	try {
    		int port = 8089;
    		avclient = new Socket("192.17.11.51", port);
    		InputStream in = avclient.getInputStream();
        	TextView connect = (TextView) findViewById(R.id.status);
        	connect.setText("Socket Connected");
    		//in.read();
    	} catch (Exception e) {
    		e.printStackTrace();
        	TextView connect = (TextView) findViewById(R.id.status);
        	connect.setText("[FAIL]");
		}

    }
}
