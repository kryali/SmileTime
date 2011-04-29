package com.smiletime;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;

public class mainActivity extends Activity{
    /** Called when the activity is first created. */
	
	private Socket avSender;
	private ParcelFileDescriptor pfd;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
		this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        setContentView(R.layout.main);
        setUpButtons();
    }
    

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
      super.onConfigurationChanged(newConfig);
    }
	
    
    public void connectToServer(){

		try {
			 avSender = new Socket("192.17.11.36", 8080);
			 pfd = ParcelFileDescriptor.fromSocket(avSender);
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }
    
    public void setUpButtons(){
    	ImageButton connect = (ImageButton) findViewById(R.id.connect);
    	connect.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// Launch new activity
				startActivity(new Intent(mainActivity.this, ListUsers.class));
			}
		});
    	

        ImageButton videoRecord = (ImageButton) findViewById(R.id.startVideo);

        videoRecord.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				startActivity(new Intent(mainActivity.this, AVRecorder.class));
			}
		});
    }

}