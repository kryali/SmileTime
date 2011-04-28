package com.smiletime;

import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;

public class BOutputStream extends OutputStream {
	
		private int maxSize = 20000;
		
		byte[] buffer = new byte[maxSize];
		public int size = 4;
		

		@Override
		public void write(int oneByte) throws IOException {
			// TODO Auto-generated method stub
			if(size >= maxSize){
				throw new IOException();
			}
			buffer[size] = (byte)oneByte;
			size++;
		}
		
		@Override
		public void write(byte[] b, int off, int len) throws IOException {
			// TODO Auto-generated method stub
			for(int i = off; i < off+len; i++){
				this.write((int)b[i]);
			}
		}
		
		@Override
		public void write(byte[] b) throws IOException{
			this.write(b, size, b.length);
		}
		
		public void send(OutputStream out){
			try {
				//out.write(intToByteArray(size));
				buffer[0] = (byte)size;
				buffer[1] = (byte)(size >>> 8);
				buffer[2] = (byte)(size >>> 16);
				buffer[3] = (byte)(size >>> 24);
				out.write(buffer, 0, size);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		public void send_udp(String serverIP, int serverPort){

			try{
				DatagramSocket s= new DatagramSocket();
				buffer[0] = (byte)size;
				buffer[1] = (byte)(size >>> 8);
				buffer[2] = (byte)(size >>> 16);
				buffer[3] = (byte)(size >>> 24);
				DatagramPacket pkt = new DatagramPacket(buffer, buffer.length, new InetSocketAddress(serverIP, serverPort));
				s.send(pkt);
			} catch( IOException e){
				
			}
		}
		
		public byte[] intToByteArray(int value){
			 return new byte[] {
		                (byte)value,
		                (byte)(value >>> 8),   
		                (byte)(value >>> 16), 
		                (byte)(value >>> 24)
			 		};
		}
}
