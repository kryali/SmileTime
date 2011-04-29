package com.smiletime;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ObjectOutputStream.PutField;
import java.net.Socket;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class ListUsers extends ListActivity {

	private 	Socket socketClient;
	private String[] users;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    	//setContentView(R.layout.avconnect);
    	connect();
    	setListAdapter(new ArrayAdapter<String>(this, R.layout.list_item, users));
    	
    	ListView lv = getListView();
    	lv.setTextFilterEnabled(true);
    	
    	lv.setOnItemClickListener(new OnItemClickListener() {
    		public void onItemClick(AdapterView<?> parent, View view, int position, long id){
    			//Toast.makeText(getApplicationContext(), ((TextView) view).getText(), Toast.LENGTH_SHORT).show();
    			//Intent i = new Intent(ListUsers.this, AVConnect.class);
    			Intent i = new Intent(ListUsers.this, AVRecorder.class);
    			i.putExtra("name", ((TextView) view).getText());
    			startActivity(i);
    		}
    	});
    }
   
    
  
    
    public void connect(){
    	//InetAddress dstAddress = new InetSocketAddress(, 8081);
    	try {
    		int port = 1337;
    		socketClient = new Socket("173.230.140.232", port);
    		
    		// Send the LIST byte
    		OutputStream out = socketClient.getOutputStream();
    		byte[] buffer = new byte[1];
    		buffer[0]= 3;
    		out.write(buffer);
    		out.flush();

    		InputStream in = socketClient.getInputStream();
    		int size = in.read();
    		
    		byte[] buffer2 = new byte[size];
    		in.read(buffer2);
    		String listStr = new String(buffer2);
    		
    		users =  listStr.split("#");
    		in.close();
    		out.close();
    		socketClient.close();
        	//TextView connect = (TextView) findViewById(R.id.status);
        	///connect.setText("Socket Connected, size: " + size + ": " + users[0]);
    		//in.read();
    	} catch (Exception e) {
    		e.printStackTrace();
        	TextView connect = (TextView) findViewById(R.id.status);
        	connect.setText("[FAIL]" + e);
		}

	}
}
