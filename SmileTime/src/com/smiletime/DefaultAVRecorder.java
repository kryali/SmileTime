package com.smiletime;

import java.io.IOException;

import android.app.Activity;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class DefaultAVRecorder extends Activity implements SurfaceHolder.Callback {

	private MediaRecorder mRecorder = null;
	private String mFileName = null;
	private SurfaceView mPreview = null;
    private SurfaceHolder holder;
	private int mVideoHeight = 300;
	private int mVideoWidth = 400;
	private String LOG_TAG = "VIDEO";
	private boolean isRecording = false;
	

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.avrecorder);
        

        mPreview = (SurfaceView) findViewById(R.id.sv);
        mPreview.setMinimumHeight(mVideoHeight);
        mPreview.setMinimumWidth(mVideoWidth);
        holder = mPreview.getHolder();
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        holder.setFixedSize(mVideoWidth, mVideoHeight);
        Surface s = holder.getSurface();
        s.setSize(mVideoWidth, mVideoHeight);
        holder.addCallback(this);
        
        mFileName = Environment.getExternalStorageDirectory().getAbsolutePath();
        mFileName += "/test.mp4";
                
    }
	

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.d(LOG_TAG, "surface changed");
		
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surface created");

        Button videoRecord = (Button) findViewById(R.id.startVideo);

        videoRecord.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				startVideoRecording();
			}
		});

		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surface destroyed");
		if(isRecording){
			mRecorder.stop();
			mRecorder.reset();
			mRecorder.release();
		}
		isRecording = false;
	}
	

    
    public void startVideoRecording(){
    	Log.e("VIDEO", "Setting up video recorder");
    	mRecorder = new MediaRecorder();
    	mRecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);
    	
    	mRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
    	mRecorder.setVideoFrameRate(20);
    	mRecorder.setVideoSize(mVideoWidth, mVideoHeight);
    	mRecorder.setPreviewDisplay(holder.getSurface());
    	mRecorder.setOutputFile(mFileName);
    	mRecorder.setVideoEncoder(MediaRecorder.VideoEncoder.H264);
    	try {
    		mRecorder.prepare();
    	} catch (IOException e){
    		Log.e(LOG_TAG, "perpare() failed");
    		//mRecorder.stop();
    	}
    	mRecorder.start();
    	isRecording = true;
    }
}
