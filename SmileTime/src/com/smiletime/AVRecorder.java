package com.smiletime;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Semaphore;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ScrollView;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

public class AVRecorder extends Activity implements SurfaceHolder.Callback {

	private Camera mCamera;
	private Handler main;
	private MediaRecorder mRecorder;
	private boolean mPreviewRunning = false;
	private boolean isAudioRecording = false;
	private SurfaceView mSurfaceView;
	private SurfaceHolder holder;
	private int mVideoWidth = 320;
	private int mVideoHeight = 240;
	private Socket serverSocket;
	public AudioRecord audioRecord;
	private byte[] audioBuffer;
	private int audioBufferSize;
	private boolean sendAudio = true;
	//private InputStream in;
	private AudioTrack mTrack;
	private String tag = "AVR";
	private int bufferSize = 4160;
	private boolean isControlConnected = false;
	private long lastTime = -1; 
	private int frames;
	private Semaphore jpgMut;
	private Semaphore frameMut;
	

	private boolean shouldSendImage = true;
	
	private int frameRate = 50;
	private int frameDelay = 500; //Set the framerate of the images
	private String serverIP = "192.17.11.71";
	private int videoPort = 1339;
	private int controlPort = 1336;
	private OutputStream out;
	private InputStream in;
	private Socket nameserverClient;
	
	public void createAlert(){
		AlertDialog.Builder alert = new AlertDialog.Builder(this);

		alert.setTitle("Enter Username");
		alert.setMessage("Create your username");

		// Set an EditText view to get user input 
		final EditText input = new EditText(this);
		alert.setView(input);

		alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
		public void onClick(DialogInterface dialog, int whichButton) {
		  String value = input.getText().toString();
		  // Do something with value!
		  }
		});

		alert.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
		  public void onClick(DialogInterface dialog, int whichButton) {
		    // Canceled.
		  }
		});

		alert.show();
	}
	


	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		hideTitleAndNotification();
		
		setContentView(R.layout.avrecorder);
		mSurfaceView = (SurfaceView) findViewById(R.id.avsv);
		

        //mSurfaceView.setMinimumHeight(mVideoHeight);
        //mSurfaceView.setMinimumWidth(mVideoWidth);
        holder = mSurfaceView.getHolder();
        holder.setFixedSize(mVideoWidth, mVideoHeight);
        Surface s = holder.getSurface();
        s.setSize(mVideoWidth, mVideoHeight);
        
        if(frameRate != -1)
        	frameDelay = (int)(((float)1/ (float)frameRate) * 1000);
        
        //initAudio();
        
        boolean launchThreads = true;

    	Intent i = getIntent();
    	Bundle b = i.getExtras();
    	try {
    		String name = b.getString("name");
    		//setText(name);
        	fetchUser(name);
        } catch (Exception e){
        	launchThreads = false;
    		createAlert();
    		 // do nothing
        }

        jpgMut = new Semaphore(1);
        frameMut = new Semaphore(1);
        
        main = new Handler();
        initAudioRecord();
        audioSend.start();
        
        if(launchThreads){
            serverConnect();
        	launchThreads();
            initTimer();
            initLatencyTimer();
        }
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

		ImageView v = (ImageView) findViewById(R.id.image01);
		v.setOnTouchListener(new OnTouchListener() {
			
			float firstX;
			float firstY;
			float tolerance = 60;
			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				// TODO Auto-generated method stub
				// int a = event.getAction();
				if(event.getAction() == MotionEvent.ACTION_DOWN){
					//setText("Touch down (" + event.getX() + ", " + event.getY() + ")");
					firstX = event.getX();
					firstY = event.getY();
				}
				else if( event.getAction() == MotionEvent.ACTION_UP){
					float distanceX =  (event.getX() - firstX);
					float distanceY =    (event.getY()-firstY);
					//setText("Touch up (" +distanceX + ", " +  distanceY + ")");
					if( Math.abs(distanceY) > tolerance || Math.abs(distanceX) > tolerance){
						sendControl(distanceX, distanceY);
					}
				}
				return true;
			}
		});
		
		Button but = (Button) findViewById(R.id.chatButton);
		but.setOnClickListener(new OnClickListener() {

			byte[] payload = new byte[144];
			@Override
			public void onClick(View v) {

				EditText e = (EditText) findViewById(R.id.chatInput);
				String s = e.getText().toString();
				insertMessage("ME>" + s);
				byte[] message = s.getBytes();
				
				e.setText("");
				int packetType = 4;
				payload[0] = (byte) packetType;
				payload[1] = (byte) (packetType >>> 8);
				payload[2] = (byte) (packetType >>> 16);
				payload[3] = (byte) (packetType >>> 24);
				for(int i = 0; i < message.length; i++){
					payload[i+4] = message[i];
				}
				payload[message.length+4] = '\0';
				try {
					out.write(payload);
				} catch (IOException e1) {
					
					e.toString();
				}
			}
		});
	}

	
	Camera.PreviewCallback mPreviewCallback = new Camera.PreviewCallback() {
		int count = 0;
		@Override
		public void onPreviewFrame(byte[] data, Camera camera) {
			// TODO Auto-generated method stub
			//BitmapFactory f = new BitmapFactory();
		
			if(shouldSendImage){
				// the timer has fired, so now is probably a good time to send the image to keep within the right frame rates
				//if( count == 0){
					try{
						/*
						DatagramSocket s= new DatagramSocket();
						byte[] data2 = new byte[4];
						data2[0] = 100;
						DatagramPacket pkt = new DatagramPacket(data2, data2.length, new InetSocketAddress(serverIP, serverPort));
						s.send(pkt);
						setText("Sent over datagram packet..");
						*/
						YuvImage yuvi = new YuvImage(data, ImageFormat.NV21, mVideoWidth, mVideoHeight, null);
						Rect rect = new Rect(0,0,mVideoWidth,mVideoHeight);
						
						BOutputStream bos = new BOutputStream();
						yuvi.compressToJpeg(rect, 70, bos);
						//bos.send(out);
						bos.send_udp(serverIP, videoPort);
						//setText("Frame sent");
						//out.write(data);
					} catch (Exception e) {
						// TODO: handle exception
					}
					//Bitmap b = BitmapFactory.decodeByteArray(data, 0, data.length);
					//b.compress(Bitmap.CompressFormat.JPEG, 100, out);
				//}
				count++;
				shouldSendImage = false;
			}
			
		}
	};
	
	Camera.PictureCallback mPictureCallback = new Camera.PictureCallback() {
		
		@Override
		public void onPictureTaken(byte[] data, Camera camera) {
			// TODO Auto-generated method stub
			// setText("Picture snapped!");
			mPreviewRunning = false;
			camera.startPreview();
			mPreviewRunning = true;
		}
	};
	

    public int byteToInt(byte[] input){
    	return ((input[3] << 24) | (input[2] << 16) | (input[1] << 8 )| input[0]);
    }
  

    public byte[] stringToByte(String s){
    	byte[] ret = new byte[s.length()];
    	for(int i = 0; i < s.length(); i++){
    		ret[i] = (byte) s.charAt(i);
    	}
    	return ret;
    }
    

    public void fetchUser(String name){
    	//InetAddress dstAddress = new InetSocketAddress(, 8081);
    	try {
    		int port = 1337;
    		nameserverClient = new Socket("173.230.140.232", port);
    		
    		// Send the FIND byte
    		OutputStream out = nameserverClient.getOutputStream();
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
    		out.flush();
    		
    		InputStream in = nameserverClient.getInputStream();
    		
    		buffer = new byte[4];
    		in.read(buffer);
    		int size = byteToInt(buffer);
    		byte[] buffer2 = new byte[size];
    		in.read(buffer2);
    		String listStr = new String(buffer2);
    		in.close();
    		out.close();
    		User u = new User(listStr);
    		serverIP = u.ip;
    		setText(serverIP);
    		nameserverClient.close();
    		//serverPort = Integer.parseInt(u.port);
    		//in.read();
    	} catch (Exception e) {
    		e.printStackTrace();
		}

    }
    
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
      super.onConfigurationChanged(newConfig);
    }
	
    public void launchThreads(){

        
        VideoDecodeThread t = new VideoDecodeThread(handler, jpgMut);
        t.start();
        
        ControlThread ct = new ControlThread(in, out, msgHandler);
        ct.start();
        
        AudioDecodeThread at = new AudioDecodeThread();
        at.start();
    }
    	
	public void hideTitleAndNotification(){
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
	}
	
	final Handler handler = new Handler() {
        public void handleMessage(Message msg) {
        	
        	Bundle b = msg.getData();
        	byte[] packet = b.getByteArray("packet");
        	long currTime = System.currentTimeMillis();
        	float sSinceLast = ((currTime - lastTime)/(float)1000);
        	if( lastTime == -1){
        		lastTime = currTime;
        	} else if(sSinceLast > 3) {
        		// If it's been three seconds since the last time, update the fps
        		TextView v = (TextView) findViewById(R.id.fps);
        		int fps = (int) (frames/(float)sSinceLast);
        		try {
					frameMut.acquire();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
        		frames = 0;
        		frameMut.release();
        		lastTime = currTime;
        	}
        	
			int packetType = ((packet[3] << 24) | (packet[2] << 16) | (packet[1] << 8 )| packet[0]);
			if( packetType == 3){
	        	ImageView v = (ImageView) findViewById(R.id.image01);
	        	Bitmap bm = BitmapFactory.decodeByteArray(packet, 12, packet.length-12);
	        	try {
					jpgMut.acquire();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
	        	v.setImageBitmap(bm);
	        	v.invalidate();
	        	jpgMut.release();
	        	Log.d(tag, "Displaying a video jpg");
	        	try {
					frameMut.acquire();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
	        	frames++;
	        	frameMut.release();
			} 
        }
    };
    
    final Handler msgHandler = new Handler(){
    	public void handleMessage(Message msg){

        	Bundle b = msg.getData();
        	byte[] packet = b.getByteArray("message");
        	String str = new String(packet);
        	insertMessage("PEER:0> " + str);
    	}
    };
	
	public void initTimer(){
		Timer t = new Timer("frameTimer", false);
		TimerTask task = new TimerTask() {
			
			@Override
			public void run() {
				shouldSendImage = true;
				//if(mPreviewRunning)
					//mCamera.takePicture(null, null, mPictureCallback);
			}
		};
		t.schedule(task, frameDelay, frameDelay);
	}
	
	public void initLatencyTimer(){
		Timer t = new Timer("latencyTimer", false);
		LatencyTimerTask task = new LatencyTimerTask(out);
		t.schedule(task, 10000, 10000);
	}

	private int udpAudioCounter=0;
	
    private Thread audioSend= new Thread(){
    	public void run(){

    		while(true){
    			if(sendAudio){
		    		int bufferReadResult = audioRecord.read(audioBuffer, 0, audioBufferSize);
					try {
						DatagramSocket s= new DatagramSocket();				
						DatagramPacket pkt = new DatagramPacket(audioBuffer, audioBuffer.length, new InetSocketAddress(serverIP, 1338));
						s.send(pkt);
						udpAudioCounter++;
						Log.d("counter", "#packets: "+ udpAudioCounter + "readFromDev: " + bufferReadResult + " bfferLen:" + audioBuffer.length);
					} catch (Exception e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
		    		/*try {
						out.write(audioBuffer);
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}*/
    			}
    		}
    	}
    };
    
	public void initAudioRecord(){
		
		try{
			int frequency = 11025;
			int channelConfiguration = AudioFormat.CHANNEL_CONFIGURATION_MONO;
			int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
			audioBufferSize = AudioRecord.getMinBufferSize(frequency, channelConfiguration, audioEncoding);
			audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, frequency, channelConfiguration, audioEncoding, audioBufferSize);
			audioBuffer = new byte[audioBufferSize];
			//audioRecord = findAudioRecord();
			audioRecord.getState();
			audioRecord.startRecording();
		} catch (Exception e){
			e.printStackTrace();
		}
	}

	public void initAudio(){
		mRecorder = new MediaRecorder();
		mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
		mRecorder.setOutputFormat(MediaRecorder.OutputFormat.RAW_AMR);
		mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
		
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();
        path += "/test.amr";
		mRecorder.setOutputFile(path);
		
		try{
			mRecorder.prepare();
		} catch (IOException e){
			setText("prepare failed");
		}
		mRecorder.start();
		isAudioRecording = true;
	}

	public void insertMessage(String str){
		TableLayout tl = (TableLayout) findViewById(R.id.chatText);
		TableRow tr = new TableRow(this);
		TextView tv1 = new TextView(this);
		tv1.setText(str);
		tr.addView(tv1);
		tr.setBackgroundColor(0xFF212121);
		TableRow.LayoutParams lp = new TableRow.LayoutParams();
		lp.setMargins(10, 0, 0, 0);
		tr.setLayoutParams(lp);
		tr.setPadding(2, 2, 2, 2);
		tl.addView(tr);
		
		ScrollView sv = (ScrollView) findViewById(R.id.scrollChatView);
		sv.scrollTo(0, 10000);
	}
	
	public void serverConnect(){

        try {
			serverSocket = new Socket(serverIP, controlPort);
			out = serverSocket.getOutputStream();
			in = serverSocket.getInputStream();
			isControlConnected = true;
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event){
		if( keyCode == KeyEvent.KEYCODE_BACK){
			System.runFinalizersOnExit(true);
			System.exit(0);
		}
		return super.onKeyDown(keyCode, event);
	}
	
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO Auto-generated method stub
		
	}
	
	public void setText(String text){

		//TextView tv = ((TextView) findViewById(R.id.StreamText));
		//tv.setText(text);
		Toast.makeText(AVRecorder.this, text, Toast.LENGTH_SHORT).show();
	}

	public void initCamera(){
		mCamera = Camera.open();
		

        Camera.Parameters parameters = mCamera.getParameters();
        
        parameters.setPreviewSize(mVideoWidth, mVideoHeight);
        //parameters.setPreviewFormat(ImageFormat.JPEG);
        //parameters.set("orientation", "portrait");
        //parameters.set("rotation", 90);
        mCamera.setParameters(parameters);
        
		//setText(" "+ parameters.getPictureSize().height);
		try {
			mCamera.setPreviewDisplay(holder);
			mCamera.setPreviewCallback(mPreviewCallback);
		} catch (IOException e) {
			setText(e.toString());
			return;
		}
		//setText("Preview Running...");
		mCamera.startPreview();
		mPreviewRunning = true;
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stu
		initCamera();
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		if(mPreviewRunning){
			mCamera.stopPreview();
			mCamera.release();
		}
		mPreviewRunning = false;
		
	}
	
	public void sendControl(float distanceX, float distanceY){
		if(!isControlConnected)
			return;
		int sendX = (int) ((distanceX/mVideoWidth) * (1000-70) + 70);
		int sendY = (int) ((distanceY/mVideoHeight) * (750-70) + 70) * -1;
		//setText("Sending (" + sendX + ", " + sendY + ")" );
		int packetType = 1;
		byte[] payload = new byte[12];

		 payload[3]= (byte)(packetType >>> 24);
		 payload[2]= (byte)(packetType >>> 16);
		 payload[1]= (byte)(packetType >>> 8);
		 payload[0]= (byte)(packetType);

		 payload[7]= (byte)(sendX >>> 24);
		 payload[6]= (byte)(sendX >>> 16);
		 payload[5]= (byte)(sendX >>> 8);
		 payload[4]= (byte)(sendX);

		 payload[11]= (byte)(sendY >>> 24);
		 payload[10]= (byte)(sendY >>> 16);
		 payload[9]= (byte)(sendY >>> 8);
		 payload[8]= (byte)(sendY);
			
		 try {
			out.write(payload);
		} catch (IOException e) {
			//
		}
	}
	
	@Override
	public void onBackPressed(){
		System.runFinalizersOnExit(true);
		System.exit(0);
	}
	
	@Override
	public void onDestroy(){
		super.onDestroy();
		System.runFinalizersOnExit(true);
		System.exit(0);
	}
}
