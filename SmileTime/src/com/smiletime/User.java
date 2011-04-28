package com.smiletime;

public class User {
	public String name;
	public String ip;
	public String port;
	public int protocol;

	public User(String string){
		String[] data = string.split("#");
		this.name = data[0];
		String[] network = data[1].split(":");
		this.ip = network[0];
		this.port = network[1];
		this.protocol = Integer.parseInt(data[2]);
	}
	
	public String toString(){ 
		return ("\n Name: "+ this.name + "\n IP:" + this.ip + "\n Port:" + this.port);
	}
}
