package com.smiletime;

import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;

public class BOutputStream extends OutputStream {
	
		private int maxSize = 20000;
		
		byte[] buffer = new byte[maxSize];
		public int size = 12;
		

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
				int packetType = 3;
				buffer[0] = (byte)packetType;
				buffer[1] = (byte)(packetType >>> 8);
				buffer[2] = (byte)(packetType >>> 16);
				buffer[3] = (byte)(packetType >>> 24);
				buffer[4] = (byte)size;
				buffer[5] = (byte)(size >>> 8);
				buffer[6] = (byte)(size >>> 16);
				buffer[7] = (byte)(size >>> 24);
				buffer[8] = (byte)size;
				buffer[9] = (byte)(size >>> 8);
				buffer[10] = (byte)(size >>> 16);
				buffer[11] = (byte)(size >>> 24);
				out.write(buffer, 0, size);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		public void send_udp(String serverIP, int serverPort){

			try{
				DatagramSocket s= new DatagramSocket();
				int packetType = 3;
				buffer[0] = (byte)packetType;
				buffer[1] = (byte)(packetType >>> 8);
				buffer[2] = (byte)(packetType >>> 16);
				buffer[3] = (byte)(packetType >>> 24);
				buffer[4] = (byte)size;
				buffer[5] = (byte)(size >>> 8);
				buffer[6] = (byte)(size >>> 16);
				buffer[7] = (byte)(size >>> 24);
				buffer[8] = (byte)size;
				buffer[9] = (byte)(size >>> 8);
				buffer[10] = (byte)(size >>> 16);
				buffer[11] = (byte)(size >>> 24);
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
