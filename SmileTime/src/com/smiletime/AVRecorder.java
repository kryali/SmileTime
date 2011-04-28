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

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.content.res.Resources;
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
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
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
	

	private boolean shouldSendImage = true;
	
	private int frameRate = 50;
	private int frameDelay = 500; //Set the framerate of the images
	private String serverIP = "192.17.11.71";
	private int videoPort = 1339;
	private int controlPort = 1336;
	private OutputStream out;
	private InputStream in;
	private Socket nameserverClient;
	

	
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
    		
    		
    		InputStream in = nameserverClient.getInputStream();
    		
    		buffer = new byte[4];
    		in.read(buffer);
    		int size = byteToInt(buffer);
    		byte[] buffer2 = new byte[size];
    		in.read(buffer2);
    		String listStr = new String(buffer2);
    		
    		User u = new User(listStr);
    		serverIP = u.ip;
    		setText(serverIP);
    		nameserverClient.close();
    		//serverPort = Integer.parseInt(u.port);
    		//in.read();
    	} catch (Exception e) {
    		e.printStackTrace();
        	TextView connect = (TextView) findViewById(R.id.status);
        	connect.setText("[FAIL]" + e);
		}

    }
    
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
      super.onConfigurationChanged(newConfig);
    }
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		hideTitleAndNotification();
		setContentView(R.layout.avrecorder);
		mSurfaceView = (SurfaceView) findViewById(R.id.avsv);
		
		//HorizontalScrollView h = (HorizontalScrollView) findViewById(R.id.ScrollView01);
		LinearLayout l = (LinearLayout) findViewById(R.id.llayout);
	
        //mSurfaceView.setMinimumHeight(mVideoHeight);
        //mSurfaceView.setMinimumWidth(mVideoWidth);
        holder = mSurfaceView.getHolder();
        holder.setFixedSize(mVideoWidth, mVideoHeight);
        Surface s = holder.getSurface();
        s.setSize(mVideoWidth, mVideoHeight);
        
        if(frameRate != -1)
        	frameDelay = (int)(((float)1/ (float)frameRate) * 1000);
        
        //initAudio();
        

    	Intent i = getIntent();
    	Bundle b = i.getExtras();
    	try {
    		String name = b.getString("name");
    		//setText(name);
        	fetchUser(name);
        } catch (Exception e){
        	; // do nothing
        }

        serverConnect();
        initTimer();
        
        main = new Handler();
        initAudioRecord();
        audioSend.start();
        
        AVDecodeThread t = new AVDecodeThread(handler);
        t.start();
        
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
		
		

	}
	
	public void hideTitleAndNotification(){
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
	}
	
	final Handler handler = new Handler() {
        public void handleMessage(Message msg) {
        	Bundle b = msg.getData();
        	byte[] packet = b.getByteArray("packet");

			int packetType = ((packet[3] << 24) | (packet[2] << 16) | (packet[1] << 8 )| packet[0]);
			if( packetType == 3){
	        	ImageView v = (ImageView) findViewById(R.id.image01);
	        	Resources res = getResources();
	        	//Bitmap bm = BitmapFactory.decodeResource(res, R.drawable.kiran);
	        	Bitmap bm = BitmapFactory.decodeByteArray(packet, 12, packet.length-12);
	        	v.setImageBitmap(bm);
	        	Log.d(tag, "Displaying a video jpg");
			} 
        }
    };
	
	public void initTimer(){
		Timer t = new Timer("frameTimer", false);
		TimerTask task = new TimerTask() {
			
			@Override
			public void run() {
				// TODO Auto-generated method stub
				shouldSendImage = true;
				//if(mPreviewRunning)
					//mCamera.takePicture(null, null, mPictureCallback);
			}
		};
		t.schedule(task, frameDelay, frameDelay);
	}

    private Thread audioSend= new Thread(){
    	public void run(){

    		while(true){
    			if(sendAudio){
		    		int bufferReadResult = audioRecord.read(audioBuffer, 0, audioBufferSize);
					try {
						DatagramSocket s= new DatagramSocket();				
						DatagramPacket pkt = new DatagramPacket(audioBuffer, audioBuffer.length, new InetSocketAddress(serverIP, 1338));
						s.send(pkt);
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

	public void serverConnect(){

        try {
			serverSocket = new Socket(serverIP, controlPort);
			out = serverSocket.getOutputStream();
			in = serverSocket.getInputStream();
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
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
		int sendX = (int) ((distanceX/mVideoWidth) * (1000-70) + 70);
		int sendY = (int) ((distanceY/mVideoHeight) * (750-70) + 70) * -1;
		setText("Sending (" + sendX + ", " + sendY + ")" );
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
}
