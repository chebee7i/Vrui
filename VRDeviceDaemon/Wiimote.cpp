/***********************************************************************
Wiimote - Class to communicate with a Nintendo Wii controller via
bluetooth.
Copyright (c) 2007 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>

#include "Wiimote.h"

/****************
Helper functions:
****************/

namespace Misc {

template <>
class ValueCoder<Wiimote::AxisSettings>
	{
	/* Methods: */
	public:
	static std::string encode(const Wiimote::AxisSettings& v)
		{
		std::vector<float> values;
		values.push_back(v.minValue);
		values.push_back(v.maxValue);
		values.push_back(v.center);
		values.push_back(v.flat);
		return ValueCoder<std::vector<float> >::encode(values);
		};
	static Wiimote::AxisSettings decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		/* Decode string as vector of integers: */
		std::vector<float> values=ValueCoder<std::vector<float> >::decode(start,end,decodeEnd);
		if(values.size()!=4)
			throw DecodingError(std::string("Wrong number of elements in ")+std::string(start,end));
		
		/* Convert vector to result structure: */
		Wiimote::AxisSettings result;
		result.minValue=values[0];
		result.maxValue=values[1];
		result.center=values[2];
		result.flat=values[3];
		return result;
		};
	};

}

/************************
Methods of class Wiimote:
************************/

void Wiimote::writePacket(unsigned char packet[],size_t packetSize)
	{
	/* Set the packet's rumble bit: */
	if(rumble)
		packet[2]|=0x01;
	else
		packet[2]&=~0x01;
	
	/* Lock the write socket: */
	Threads::Mutex::Lock writeSocketLock(writeSocketMutex);
	
	/* Write the packet: */
	write(writeSocket,packet,packetSize);
	}

void Wiimote::writeUploadPacket(void)
	{
	/* Prepare the data upload packet: */
	unsigned char writeData[]={0x52,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	/* Set the upload address space: */
	if(uploadToRegister)
		writeData[2]|=0x04;
	
	/* Set the upload address: */
	int tempAddress=uploadStartAddress;
	for(int i=2;i>=0;--i,tempAddress>>=8)
		writeData[3+i]=(unsigned char)(tempAddress&0xff);
	
	/* Determine and set the upload size: */
	int tempSize=uploadSize;
	if(tempSize>16)
		tempSize=16;
	writeData[6]=(unsigned char)(tempSize);
	
	/* Copy the upload data: */
	for(int i=0;i<tempSize;++i,++uploadDataPtr)
		writeData[7+i]=*uploadDataPtr;
	
	/* Write the upload packet: */
	writePacket(writeData,sizeof(writeData));
	
	/* Update the upload state: */
	uploadStartAddress+=tempSize;
	uploadSize-=tempSize;
	}

void Wiimote::setReportingMode(bool insideReader)
	{
	/* Assemble the data request packet: */
	unsigned char requestData[]={0x52,0x12,0x00,0x00};
	
	if(readContinuously)
		requestData[2]|=0x04;
	
	if(extensionDevice!=NONE)
		{
		if(readAccelerometers&&readIRTracking)
			requestData[3]=0x37;
		else if(readAccelerometers)
			requestData[3]=0x35;
		else if(readIRTracking)
			requestData[3]=0x36;
		else
			requestData[3]=0x32;
		}
	else
		{
		if(readAccelerometers&&readIRTracking)
			requestData[3]=0x33;
		else if(readAccelerometers)
			requestData[3]=0x31;
		else if(readIRTracking)
			requestData[3]=0x36;
		else
			requestData[3]=0x30;
		}
	
	/* Set the appropriate IR camera reporting mode for the data reporting mode: */
	if(readIRTracking)
		{
		if(insideReader)
			{
			/* Write to the camera's register area: */
			unsigned char IRMode[]={0x52,0x16,0x04,0xb0,0x00,0x33,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
			if(extensionDevice==NONE&&readAccelerometers&&readIRTracking)
				IRMode[7]=0x03;
			writePacket(IRMode,sizeof(IRMode));

			/* Wait for acknowledgment: */
			unsigned char packet[21];
			ssize_t packetSize;
			while((packetSize=read(readSocket,packet,sizeof(packet)))==0||int(packet[1])!=0x22)
				;
			}
		else
			{
			unsigned char IRMode=0x01;
			if(extensionDevice==NONE&&readAccelerometers&&readIRTracking)
				IRMode=0x03;
			uploadData(true,0xb00033,&IRMode,sizeof(unsigned char));
			}
		}
	
	/* Send the packet: */
	writePacket(requestData,sizeof(requestData));
	}

void Wiimote::updateCoreButtons(unsigned char* buttonData)
	{
	/* Extract and compactify button state from button data: */
	buttonState=(int(buttonData[1])&0x1f)|((int(buttonData[1])&0x80)>>2)|((int(buttonData[0])&0x1f)<<6);
	}

void Wiimote::updateCoreAccelerometers(unsigned char* buttonData,unsigned char* accelerometerData)
	{
	/* Assemble raw accelerometer values from MSBs and LSBs: */
	rawAccelerometers[0]=(int(accelerometerData[0])<<1)|((int(buttonData[0])>>6)&0x01);
	rawAccelerometers[1]=(int(accelerometerData[1])<<1)|((int(buttonData[1])>>5)&0x01);
	rawAccelerometers[2]=(int(accelerometerData[2])<<1)|((int(buttonData[1])>>6)&0x01);
	
	/* Store calibrated accelerometer data: */
	for(int i=0;i<3;++i)
		accelerometers[i]=(float(rawAccelerometers[i])-accelerometerZeros[i])/accelerometerGains[i];
	}

void Wiimote::updateExtension(unsigned char* extensionData)
	{
	/* Decode the extension data (thank you fucking much, DMCA!): */
	for(int i=0;i<6;++i)
		extensionData[i]=(extensionData[i]^0x17)+0x17;
	
	if(extensionDevice==NUNCHUK)
		{
		/* Update the extension button state: */
		buttonState|=((~int(extensionData[5]))&0x03)<<11;
		
		/* Update the nunchuck joystick state: */
		for(int i=0;i<2;++i)
			{
			float val=float(extensionData[i]);
			if(val<joystickAxes[i].minValue)
				joystick[i]=-1.0f;
			else if(val<joystickAxes[i].center-joystickAxes[i].flat)
				joystick[i]=(val-(joystickAxes[i].center-joystickAxes[i].flat))/((joystickAxes[i].center-joystickAxes[i].flat)-joystickAxes[i].minValue);
			else if(val<joystickAxes[i].center+joystickAxes[i].flat)
				joystick[i]=0.0f;
			else if(val<joystickAxes[i].maxValue)
				joystick[i]=(val-(joystickAxes[i].center+joystickAxes[i].flat))/(joystickAxes[i].maxValue-(joystickAxes[i].center+joystickAxes[i].flat));
			else
				joystick[i]=1.0f;
			}
		
		if(readAccelerometers)
			{
			/* Assemble raw accelerometer values from MSBs and LSBs: */
			rawAccelerometers[3]=(int(extensionData[2])<<2)|((int(extensionData[5])>>2)&0x03);
			rawAccelerometers[4]=(int(extensionData[3])<<2)|((int(extensionData[5])>>4)&0x03);
			rawAccelerometers[5]=(int(extensionData[4])<<2)|((int(extensionData[5])>>6)&0x03);
			
			/* Store calibrated accelerometer data: */
			for(int i=0;i<3;++i)
				accelerometers[3+i]=(float(rawAccelerometers[3+i])-accelerometerZeros[3+i])/accelerometerGains[3+i];
			}
		}
	}

void Wiimote::updateIRTrackingBasic(unsigned char* irTrackingData)
	{
	/* Decode IR tracking data in basic mode: */
	for(int i=0;i<2;++i)
		{
		int x0=int(irTrackingData[0+i*5])|((int(irTrackingData[2+i*5])&0x30)<<4);
		int y0=int(irTrackingData[1+i*5])|((int(irTrackingData[2+i*5])&0xc0)<<2);
		int x1=int(irTrackingData[3+i*5])|((int(irTrackingData[2+i*5])&0x03)<<8);
		int y1=int(irTrackingData[4+i*5])|((int(irTrackingData[2+i*5])&0x0c)<<6);
		if(x0!=0x3ff&&y0!=0x3ff)
			{
			trackValids[0+i*2]=true;
			trackXs[0+i*2]=float(x0);
			trackYs[0+i*2]=float(y0);
			}
		else
			trackValids[0+i*2]=false;
		if(x1!=0x3ff&&y1!=0x3ff)
			{
			trackValids[1+i*2]=true;
			trackXs[1+i*2]=float(x1);
			trackYs[1+i*2]=float(y1);
			}
		else
			trackValids[1+i*2]=false;
		}
	}

void Wiimote::updateIRTrackingExtended(unsigned char* irTrackingData)
	{
	/* Decode IR tracking data in extended mode: */
	for(int i=0;i<4;++i)
		{
		int x=int(irTrackingData[0+i*3])|((int(irTrackingData[2+i*3])&0x30)<<4);
		int y=int(irTrackingData[1+i*3])|((int(irTrackingData[2+i*3])&0xc0)<<2);
		
		if(x!=0x3ff&&y!=0x3ff)
			{
			trackValids[i]=true;
			trackXs[i]=float(x);
			trackYs[i]=float(y);
			}
		else
			trackValids[i]=false;
		}
	}

void* Wiimote::receiverThreadMethod(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Process data packets from the device until interrupted: */
	while(true)
		{
		/* Read the next data packet: */
		unsigned char packet[21]; // 21 is maximum packet size used by Wiimote HID protocol
		ssize_t packetSize=read(readSocket,packet,sizeof(packet));
		if(packetSize>0&&packet[0]==0xa1)
			{
			/* Process the data packet: */
			switch(int(packet[1]))
				{
				case 0x20: // Status report packet
					{
					/* Store the battery level: */
					batteryLevel=int(packet[7]);
					
					/* Check if the Wiimote has a connected extension device: */
					if(packet[4]&0x02)
						{
						/* Enable the extension (and the silly data encryption): */
						unsigned char enableExtension[]={0x52,0x16,0x04,0xa4,0x00,0x40,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
						writePacket(enableExtension,sizeof(enableExtension));
						
						/* Wait for acknowledgment: */
						while((packetSize=read(readSocket,packet,sizeof(packet)))==0||int(packet[1])!=0x22)
							;
						
						/* Query the type of extension device: */
						unsigned char queryExtensionType[]={0x52,0x17,0x04,0xa4,0x00,0xfe,0x00,0x02};
						writePacket(queryExtensionType,sizeof(queryExtensionType));
						
						/* Wait for the result data: */
						while((packetSize=read(readSocket,packet,sizeof(packet)))==0||int(packet[1])!=0x21)
							;
						
						/* Extract the extension device type: */
						switch((int(packet[7])<<8)|int(packet[8]))
							{
							case 0x0000:
								extensionDevice=NONE;
								break;
							
							case 0xffff:
								extensionDevice=PARTIALLY_CONNECTED;
								break;
							
							case 0xfefe:
								extensionDevice=NUNCHUK;
								break;
							
							case 0xfdfd:
								extensionDevice=CLASSIC_CONTROLLER;
								break;
							}
						
						if(extensionDevice==NUNCHUK)
							{
							if(needExtensionCalibration)
								{
								/* Read the nunchuk's calibration data: */
								unsigned char calibration[]={0x52,0x17,0x04,0xa4,0x00,0x20,0x00,0x10};
								writePacket(calibration,sizeof(calibration));
								
								/* Wait for the result data: */
								while((packetSize=read(readSocket,packet,sizeof(packet)))==0||int(packet[1])!=0x21)
									;
								
								/* Decrypt the result data: */
								for(int i=7;i<packetSize;++i)
									packet[i]=(packet[i]^0x17)+0x17;
								
								/* Store the joystick axis values: */
								for(int i=0;i<2;++i)
									{
									joystickAxes[i].minValue=float(packet[16+i*3]);
									joystickAxes[i].maxValue=float(packet[15+i*3]);
									joystickAxes[i].center=float(packet[17+i*3]);
									joystickAxes[i].flat=(joystickAxes[i].maxValue-joystickAxes[i].minValue)*0.05f;
									}
								
								/* Store the accelerometer zeros and gains: */
								for(int i=0;i<3;++i)
									{
									accelerometerZeros[3+i]=float(int(packet[7+i])<<2);
									accelerometerGains[3+i]=float(int(packet[11+i])<<2)-accelerometerZeros[3+i];
									}
								}
							}
						}
					else
						{
						/* Reset the extension's state: */
						buttonState&=0xffff;
						for(int i=0;i<2;++i)
							joystick[i]=0.0f;
						for(int i=0;i<3;++i)
							accelerometers[3+i]=0.0f;
						
						/* Update the Wiimote state: */
						extensionDevice=NONE;
						}
					
					/* Re-enable normal data reporting: */
					setReportingMode(true);
					break;
					}
				
				case 0x21: // Data download packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					{
					/* Lock the download state: */
					Threads::Mutex::Lock downloadLock(downloadMutex);
					
					if(downloadActive)
						{
						/* Check for errors: */
						downloadError=int(packet[4])&0x0f;
						if(downloadError!=0)
							{
							/* Wake up the requester: */
							downloadCompleteCond.broadcast();
							}
						else
							{
							/* Store the data contained in the report: */
							int packetDataSize=(int(packet[4])>>4)+1;
							for(int i=0;i<packetDataSize&&i<int(downloadSize);++i,++downloadDataPtr)
								*downloadDataPtr=packet[7+i];
							
							/* Update the download state: */
							downloadSize-=packetDataSize;
							if(downloadSize==0)
								{
								/* Wake up the requester: */
								downloadCompleteCond.broadcast();
								}
							}
						}
					}
					break;
				
				case 0x22: // Acknowledgment for write request
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					{
					/* Lock the upload state: */
					Threads::Mutex::Lock uploadLock(uploadMutex);
					
					if(uploadActive)
						{
						/* Check for errors: */
						uploadError=int(packet[5])&0x0f;
						if(uploadError!=0||uploadSize==0)
							{
							/* Wake up the requester: */
							uploadCompleteCond.broadcast();
							}
						else
							{
							/* Write the next upload packet: */
							writeUploadPacket();
							}
						}
					}
					break;
				
				case 0x30: // Button data packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					break;
				
				case 0x31: // Button and accelerometer data packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					/* Update the accelerometer state: */
					updateCoreAccelerometers(packet+2,packet+4);
					break;
				
				case 0x32: // Button and extension data packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					/* Update the extension state: */
					updateExtension(packet+4);
					break;
				
				case 0x33: // Button, accelerometer and IR tracking data packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					/* Update the accelerometer state: */
					updateCoreAccelerometers(packet+2,packet+4);
					
					/* Update the IR tracking state: */
					updateIRTrackingExtended(packet+7);
					break;
				
				case 0x35: // Button, accelerometer and extension data packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					/* Update the accelerometer state: */
					updateCoreAccelerometers(packet+2,packet+4);
					
					/* Update the extension state: */
					updateExtension(packet+7);
					break;
				
				case 0x36: // Button, IR tracking and extension data packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					/* Update the IR tracking state: */
					updateIRTrackingBasic(packet+4);
					
					/* Update the extension state: */
					updateExtension(packet+14);
					break;
				
				case 0x37: // Button, accelerometer, IR tracking and extension data packet
					/* Update the button state: */
					updateCoreButtons(packet+2);
					
					/* Update the accelerometer state: */
					updateCoreAccelerometers(packet+2,packet+4);
					
					/* Update the IR tracking state: */
					updateIRTrackingBasic(packet+7);
					
					/* Update the extension state: */
					updateExtension(packet+17);
					break;
				}
			
			/* Call event callbacks: */
			EventCallbackData cbData(this);
			eventCallbacks.call(&cbData);
			
			/* Wake up any suspended listeners: */
			eventCond.broadcast();
			}
		}
	
	return 0;
	}

unsigned char* Wiimote::downloadData(bool fromRegister,int address,size_t size)
	{
	/* Lock the download area (automatically released at end of method): */
	Threads::Mutex::Lock downloadLock(downloadMutex);
	
	/* Wait until all ongoing data downloads are finished: */
	while(downloadActive)
		downloadCompleteCond.wait(downloadMutex);
	
	/* Initialize the download: */
	downloadActive=true;
	downloadStartAddress=address;
	downloadSize=size&0xffff;
	downloadDataBuffer=new unsigned char[downloadSize];
	downloadDataPtr=downloadDataBuffer;
	downloadError=0;
	
	/* Send the download command to the Wiimote: */
	unsigned char readData[]={0x52,0x17,0x00,0x00,0x00,0x00,0x00,0x00};
	if(fromRegister)
		readData[2]|=0x04;
	for(int i=2;i>=0;--i)
		{
		readData[3+i]=(unsigned char)(address&0xff);
		address>>=8;
		}
	for(int i=1;i>=0;--i)
		{
		readData[6+i]=(unsigned char)(size&0xff);
		size>>=8;
		}
	writePacket(readData,sizeof(readData));
	
	/* Wait for the data to arrive: */
	downloadCompleteCond.wait(downloadMutex);
	
	/* Clean up the download area and return the read data: */
	downloadActive=false;
	unsigned char* result=downloadDataBuffer;
	downloadDataBuffer=0;
	if(downloadError!=0)
		{
		delete[] result;
		result=0;
		}
	return result;
	}

bool Wiimote::uploadData(bool toRegister,int address,const unsigned char* data,size_t size)
	{
	/* Lock the upload area (automatically released at end of method): */
	Threads::Mutex::Lock uploadLock(uploadMutex);
	
	/* Wait until all ongoing data uploads are finished: */
	while(uploadActive)
		uploadCompleteCond.wait(uploadMutex);
	
	/* Initialize the upload: */
	uploadActive=true;
	uploadToRegister=toRegister;
	uploadStartAddress=address;
	uploadSize=size&0xffff;
	uploadDataBuffer=data;
	uploadDataPtr=uploadDataBuffer;
	uploadError=0;
	
	/* Send the first upload command to the Wiimote: */
	writeUploadPacket();
	
	/* Wait for the upload to complete: */
	uploadCompleteCond.wait(uploadMutex);
	
	/* Clean up the upload area and return an error flag: */
	uploadActive=false;
	return uploadError==0;
	}

Wiimote::Wiimote(const char* deviceName,Misc::ConfigurationFile& configFile)
	:writeSocket(-1),readSocket(-1),
	 needExtensionCalibration(true),
	 readContinuously(false),readAccelerometers(false),readIRTracking(false),
	 ledMask(0x1),
	 rumble(false),
	 extensionDevice(NONE),
	 downloadActive(false),downloadDataBuffer(0),
	 uploadActive(false)
	{
	/* Check if the device name is actually a bluetooth device address: */
	bool isAddress=true;
	if(deviceName!=0)
		{
		for(const char* dnPtr=deviceName;*dnPtr!='\0'&&isAddress;++dnPtr)
			{
			if((dnPtr-deviceName)%3==2)
				{
				if(*dnPtr!=':')
					isAddress=false;
				}
			else
				if(!isxdigit(*dnPtr))
					isAddress=false;
			}
		}
	else
		isAddress=false;
	
	/* Get the device's address: */
	bdaddr_t deviceAddress;
	int btSocket=-1;
	try
		{
		/* Get a handle to the local bluetooth device: */
		int btDeviceId=hci_get_route(0);
		if(btDeviceId<0)
			Misc::throwStdErr("Wiimote::Wiimote: Could not get handle to local bluetooth device");
		btSocket=hci_open_dev(btDeviceId);
		if(btSocket<0)
			Misc::throwStdErr("Wiimote::Wiimote: Could not connect to local bluetooth device");
		
		if(isAddress)
			{
			/* Use the address provided by the caller directly: */
			str2ba(deviceName,&deviceAddress);
			
			/* Retrieve the remote device's name to ensure it's a Wiimote: */
			char remoteDeviceName[256];
			if(hci_read_remote_name(btSocket,&deviceAddress,sizeof(remoteDeviceName),remoteDeviceName,0)<0||strcmp("Nintendo RVL-CNT-01",remoteDeviceName)!=0)
				Misc::throwStdErr("Wiimote::Wiimote: Device at address %s is not a Wiimote",deviceName);
			}
		else
			{
			inquiry_info* iis=0;
			try
				{
				/* Scan for nearby bluetooth devices: */
				int maxNumResponses=255;
				inquiry_info* iis=new inquiry_info[maxNumResponses];
				int numResponses=hci_inquiry(btDeviceId,8,maxNumResponses,0,&iis,IREQ_CACHE_FLUSH);
				if(numResponses<0)
					Misc::throwStdErr("Wiimote::Wiimote: Error while scanning for nearby bluetooth devices");
				
				/* Search for the first device of the given name: */
				if(deviceName==0)
					deviceName="Nintendo RVL-CNT-01";
				bool deviceFound=false;
				for(int i=0;i<numResponses&&!deviceFound;++i)
					{
					/* Read the remote device's name: */
					char remoteDeviceName[256];
					if(hci_read_remote_name(btSocket,&iis[i].bdaddr,sizeof(remoteDeviceName),remoteDeviceName,0)>=0&&strcmp(deviceName,remoteDeviceName)==0)
						{
						deviceAddress=iis[i].bdaddr;
						deviceFound=true;
						}
					}
				if(!deviceFound)
					Misc::throwStdErr("Wiimote::Wiimote: Device \"%s\" not found",deviceName);
				
				/* Clean up: */
				delete[] iis;
				}
			catch(...)
				{
				/* Clean up and re-throw the exception: */
				delete[] iis;
				throw;
				}
			}
		}
	catch(...)
		{
		/* Clean up and re-throw the exception: */
		if(btSocket>=0)
			close(btSocket);
		throw;
		}
	
	/* Clean up: */
	if(btSocket>=0)
		close(btSocket);
	
	/* Connect to the device using the L2CAP protocol: */
	writeSocket=socket(AF_BLUETOOTH,SOCK_SEQPACKET,BTPROTO_L2CAP);
	sockaddr_l2 writeSocketAddress;
	writeSocketAddress.l2_family=AF_BLUETOOTH;
	writeSocketAddress.l2_psm=htobs(0x11);
	writeSocketAddress.l2_bdaddr=deviceAddress;
	if(connect(writeSocket,reinterpret_cast<struct sockaddr*>(&writeSocketAddress),sizeof(writeSocketAddress))<0)
		Misc::throwStdErr("Wiimote::Wiimote: Unable to connect to device \"%s\" for writing",deviceName);
	readSocket=socket(AF_BLUETOOTH,SOCK_SEQPACKET,BTPROTO_L2CAP);
	sockaddr_l2 readSocketAddress;
	readSocketAddress.l2_family=AF_BLUETOOTH;
	readSocketAddress.l2_psm=htobs(0x13);
	readSocketAddress.l2_bdaddr=deviceAddress;
	if(connect(readSocket,reinterpret_cast<struct sockaddr*>(&readSocketAddress),sizeof(readSocketAddress))<0)
		{
		close(writeSocket);
		Misc::throwStdErr("Wiimote::Wiimote: Unable to connect to device \"%s\" for reading",deviceName);
		}
	
	/* Turn off the blinking LEDs: */
	unsigned char setLEDs[]={0x52,0x11,0x00};
	setLEDs[2]=(unsigned char)((ledMask<<4)&0xff);
	writePacket(setLEDs,sizeof(setLEDs));
	
	/* Initialize Wiimote state: */
	buttonState=0x0;
	for(int i=0;i<2;++i)
		joystick[i]=0.0f;
	for(int i=0;i<6;++i)
		accelerometers[i]=0.0f;
	for(int i=0;i<4;++i)
		{
		trackValids[i]=false;
		trackXs[i]=0.0f;
		trackYs[i]=0.0f;
		}
	
	/* Start the data receiving thread: */
	receiverThread.start(this,&Wiimote::receiverThreadMethod);
	
	/* Wait until the receiver thread processed the status packet: */
	waitForEvent();
	
	/* Go to connected device's section: */
	char connectedDeviceAddress[19];
	ba2str(&deviceAddress,connectedDeviceAddress);
	configFile.setCurrentSection(connectedDeviceAddress);
	
	try
		{
		/* Read the accelerometer zeros: */
		std::vector<float> cfazs=configFile.retrieveValue<std::vector<float> >("./accelerometerZeros");
		if(cfazs.size()!=6)
			throw 42;
		for(int i=0;i<6;++i)
			accelerometerZeros[i]=cfazs[i];
		
		/* Read the accelerometer gains: */
		std::vector<float> cfags=configFile.retrieveValue<std::vector<float> >("./accelerometerGains");
		if(cfags.size()!=6)
			throw 42;
		for(int i=0;i<6;++i)
			accelerometerGains[i]=cfags[i];
		
		/* Read the joystick axis settings: */
		std::vector<AxisSettings> jass=configFile.retrieveValue<std::vector<AxisSettings> >("./joystickAxes");
		if(jass.size()!=2)
			throw 42;
		for(int i=0;i<2;++i)
			joystickAxes[i]=jass[i];
		
		needExtensionCalibration=false;
		}
	catch(...)
		{
		/* Download the Wiimote's calibration data: */
		unsigned char* calibration=downloadData(false,0x000016,8);
		if(calibration!=0)
			{
			/* Store the accelerometer zeros and gains: */
			for(int i=0;i<3;++i)
				{
				accelerometerZeros[i]=float(int(calibration[i])<<1);
				accelerometerGains[i]=float(int(calibration[4+i])<<1)-accelerometerZeros[i];
				}
			
			delete[] calibration;
			}
		else
			{
			/* Go back to device's section: */
			configFile.setCurrentSection("..");
			Misc::throwStdErr("Wiimote::Wiimote: Unable to read calibration data from device \"%s\"",deviceName);
			}
		}
	
	/* Go back to device's section: */
	configFile.setCurrentSection("..");
	}

Wiimote::~Wiimote(void)
	{
	/* Shut down the receiver thread: */
	receiverThread.cancel();
	receiverThread.join();
	
	/* Delete dangling state: */
	delete[] downloadDataBuffer;
	
	/* Close communications with the Wiimote: */
	close(writeSocket);
	close(readSocket);
	}

int Wiimote::getNumButtons(void) const
	{
	int result=0;
	switch(extensionDevice)
		{
		case NONE:
		case PARTIALLY_CONNECTED:
			result=11;
			break;
		
		case NUNCHUK:
			result=13;
			break;
		
		case CLASSIC_CONTROLLER:
			result=13; // Don't actually know correct number, don't really care
			break;
		}
	return result;
	}

void Wiimote::waitForEvent(void)
	{
	/* Suspend on the event condition variable: */
	eventCond.wait();
	}

Wiimote::Vector Wiimote::getAcceleration(int deviceIndex) const
	{
	Vector result;
	deviceIndex*=3;
	for(int i=0;i<3;++i)
		{
		result[i]=accelerometers[deviceIndex+i];
		if(i<2)
			result[i]=-result[i];
		}
	return result;
	}

void Wiimote::setLEDState(int newLedMask)
	{
	ledMask=newLedMask;
	unsigned char setLEDs[]={0x52,0x11,0x00};
	setLEDs[2]=(unsigned char)((ledMask<<4)&0xff);
	writePacket(setLEDs,sizeof(setLEDs));
	}

void Wiimote::setRumble(bool enable)
	{
	/* Set the rumble state: */
	rumble=enable;
	
	/* Set the data report format (just some arbitrary report to enable the rumble pack): */
	setReportingMode();
	}

void Wiimote::requestContinuousReports(bool enable)
	{
	/* Set the reporting state: */
	readContinuously=enable;
	
	/* Set the data report format: */
	setReportingMode();
	}

void Wiimote::requestAccelerometers(bool enable)
	{
	/* Set the reporting state: */
	readAccelerometers=enable;
	
	/* Set the data report format: */
	setReportingMode();
	}

void Wiimote::requestIRTracking(bool enable)
	{
	/* Check if the IR camera needs to be initialized or stopped: */
	if(enable&&!readIRTracking)
		{
		/* Send the initialization sequence for the IR camera: */
		unsigned char enableIR1[]={0x52,0x13,0x04};
		writePacket(enableIR1,sizeof(enableIR1));
		unsigned char enableIR2[]={0x52,0x1a,0x04};
		writePacket(enableIR2,sizeof(enableIR2));
		unsigned char enableIR3[]={0x01};
		uploadData(true,0xb00030,enableIR3,sizeof(enableIR3));
		unsigned char sensitivity1[]={0x02,0x00,0x00,0x71,0x01,0x00,0xaa,0x00,0x64};
		uploadData(true,0xb00000,sensitivity1,sizeof(sensitivity1));
		unsigned char sensitivity2[]={0x63,0x03};
		uploadData(true,0xb0001a,sensitivity2,sizeof(sensitivity2));
		unsigned char enableIR4[]={0x08};
		uploadData(true,0xb00030,enableIR4,sizeof(enableIR4));
		}
	else if(!enable&&readIRTracking)
		{
		/* Stop the IR camera: */
		unsigned char disableIR1[]={0x52,0x13,0x00};
		writePacket(disableIR1,sizeof(disableIR1));
		unsigned char disableIR2[]={0x52,0x1a,0x00};
		writePacket(disableIR2,sizeof(disableIR2));
		}
	
	/* Set the reporting state: */
	readIRTracking=enable;
	
	/* Set the data report format: */
	setReportingMode();
	}
