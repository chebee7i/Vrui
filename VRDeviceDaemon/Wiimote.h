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

#ifndef WIIMOTE_INCLUDED
#define WIIMOTE_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Threads/Mutex.h>
#include <Threads/Cond.h>
#include <Threads/MutexCond.h>
#include <Threads/Thread.h>
#include <Geometry/Vector.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}

class Wiimote
	{
	/* Embedded classes: */
	public:
	enum Extension // Enumerated type for extension device types
		{
		NONE=0,PARTIALLY_CONNECTED,NUNCHUK,CLASSIC_CONTROLLER
		};
	
	enum Button // Enumerated type for button indices
		{
		/* Wiimote core buttons: */
		BUTTON_TWO=0,BUTTON_ONE,BUTTON_B,BUTTON_A,BUTTON_MINUS,
		BUTTON_HOME,BUTTON_LEFT,BUTTON_RIGHT,BUTTON_DOWN,BUTTON_UP,BUTTON_PLUS,
		
		/* Nunchuck extension buttons: */
		BUTTON_Z,BUTTON_C
		};
	
	typedef Geometry::Vector<float,3> Vector; // Type for acceleration vectors
	
	class EventCallbackData:public Misc::CallbackData // Class to report reception of Wiimote events
		{
		/* Elements: */
		public:
		Wiimote* wiimote; // Pointer to Wiimote that just received an event
		
		/* Constructors and destructors: */
		EventCallbackData(Wiimote* sWiimote)
			:wiimote(sWiimote)
			{
			};
		};
	
	struct AxisSettings // Structure with calibration settings for a joystick axis
		{
		/* Elements: */
		public:
		float minValue;
		float maxValue;
		float center;
		float flat;
		};
	
	/* Elements: */
	private:
	Threads::Mutex writeSocketMutex; // Mutex to serialize access to the write socket
	int writeSocket; // Socket to write to the Wiimote using the L2CAP protocol, channel 0x11
	int readSocket; // Socket to read from the Wiimote using the L2CAP protocol, channel 0x13
	Threads::Thread receiverThread; // ID of data receiving thread
	
	/* Current Wiimote mode: */
	AxisSettings joystickAxes[2]; // Settings for the nunchuk controller's analog joystick axes
	float accelerometerZeros[6]; // Zero values for the Wiimote's and extension device's accelerometers
	float accelerometerGains[6]; // Gain values (normalized for gravity) for the Wiimote's and extension device's accelerometers
	bool needExtensionCalibration; // Flag whether calibration data for extension devices needs to be downloaded
	bool readContinuously; // Flag whether to report values continuously
	bool readAccelerometers; // Flag if accelerometer data from the Wiimote and any extension is requested
	bool readIRTracking; // Flag if IR camera tracking data is requested
	int ledMask; // Current bit mask of active LEDs
	bool rumble; // Flag if the rumble motor is active
	Threads::MutexCond eventCond; // Condition variable to signal events to listeners
	Misc::CallbackList eventCallbacks; // List of callbacks upon Wiimote event reception
	
	/* Current Wiimote state: */
	int batteryLevel; // Current battery level (0-200)
	Extension extensionDevice; // Type of currently connected extension device
	int buttonState; // Bit field of Wiimote core buttons followed by extension buttons
	float joystick[2]; // Analog joystick values for nunchuck extension device
	int rawAccelerometers[6]; // Raw accelerometer values for Wiimote and any connected extension device
	float accelerometers[6]; // Accelerometer values for Wiimote and any connected extension device
	bool trackValids[4]; // Valid flags for targets tracked by the IR camera
	float trackXs[4]; // X positions of targets tracked by the IR camera
	float trackYs[4]; // X positions of targets tracked by the IR camera
	
	/* State to read bulk data from the Wiimote: */
	Threads::Mutex downloadMutex; // Mutex serializing access to the data download facility
	bool downloadActive; // Flag if a data download is currently in progress
	int downloadStartAddress; // Start address of data in Wiimote's memory or register address space (for sanity checking)
	size_t downloadSize; // Number of outstanding bytes
	unsigned char* downloadDataBuffer; // Data buffer; allocated by requester
	unsigned char* downloadDataPtr; // Pointer to the end of the already read data
	int downloadError; // Error indicator (0 - no error, 7 - reading from write-only address, 8 - reading from non-existent memory)
	Threads::Cond downloadCompleteCond; // Condition variable to signal completion of data download to requester
	
	/* State to upload bulk data to the Wiimote: */
	Threads::Mutex uploadMutex; // Mutex serializing access to the data upload state
	bool uploadActive; // Flag if a data upload is currently in progress
	bool uploadToRegister; // Flag whether to upload to a register or memory
	int uploadStartAddress; // Start address of data in Wiimote's memory or register address space
	size_t uploadSize; // Number of outstanding bytes
	const unsigned char* uploadDataBuffer; // Data buffer
	const unsigned char* uploadDataPtr; // Pointer to the end of the already written data
	int uploadError; // Error indicator (value yet undetermined)
	Threads::Cond uploadCompleteCond; // Condition variable to signal completion ot data upload to requester
	
	/* Private methods: */
	void writePacket(unsigned char packet[],size_t packetSize); // Safely writes a packet to the Wiimote; modifies packet with rumble flag
	void writeUploadPacket(void); // Writes the next chunk of data from the upload area to the Wiimote
	void setReportingMode(bool insideReader =false); // Sets the Wiimote's data reporting mode based on the currently requested features
	void updateCoreButtons(unsigned char* buttonData); // Updates core button state
	void updateCoreAccelerometers(unsigned char* buttonData,unsigned char* accelerometerData); // Updates core accelerometer state
	void updateExtension(unsigned char* extensionData); // Updates extension buttons, joystick, and accelerometers if requested
	void updateIRTrackingBasic(unsigned char* irTrackingData); // Updates all tracked IR targets in basic mode
	void updateIRTrackingExtended(unsigned char* irTrackingData); // Updates all tracked IR targets in extended mode
	void* receiverThreadMethod(void); // Thread method for the data receiving thread
	unsigned char* downloadData(bool fromRegister,int address,size_t size); // Method to download bulk data from the Wiimote; returns pointer to new[]-allocated data on success, 0 otherwise
	bool uploadData(bool toRegister,int address,const unsigned char* data,size_t size); // Method to upload bulk data to the Wiimote; waits for acknowledgment and returns true on success
	
	/* Constructors and destructors: */
	public:
	Wiimote(const char* deviceName,Misc::ConfigurationFile& configFile); // Connects to a Wiimote that has the given address or name
	~Wiimote(void); // Disconnects from the Wiimote
	
	/* Methods: */
	int getBatteryLevel(void) const // Returns the current battery charge level in percent
		{
		return (batteryLevel+1)/2;
		};
	Extension getExtensionDevice(void) const // Returns the type of the currently connected extension device
		{
		return extensionDevice;
		};
	int getNumButtons(void) const; // Returns the number of buttons on the Wiimote and all extension devices
	void waitForEvent(void); // Suspends the calling thread until a data packet arrives from the Wiimote
	Misc::CallbackList& getEventCallbacks(void) // Returns the event reception callback list
		{
		return eventCallbacks;
		};
	bool getButtonState(int buttonName) const // Returns the current state of the given button
		{
		return (buttonState&(1<<buttonName))!=0x0;
		};
	float getJoystickValue(int axisIndex) const // Returns the current value of the given axis of the nunchuck extension's joystick
		{
		return joystick[axisIndex];
		};
	int getRawAccelerometerValue(int accelerometerIndex) const // Returns the current raw value of the given accelerometer
		{
		return rawAccelerometers[accelerometerIndex];
		};
	float getAccelerometerValue(int accelerometerIndex) const // Returns the current value of the given accelerometer
		{
		return accelerometers[accelerometerIndex];
		};
	Vector getAcceleration(int deviceIndex) const; // Returns acceleration vector of given device (0: Wiimote, 1: Nunchuk extension)
	bool getIRTargetValid(int targetIndex) const // Returns the valid flag for the given IR tracking target
		{
		return trackValids[targetIndex];
		};
	float getIRTargetX(int targetIndex) const // Returns the X position of the given IR tracking target
		{
		return trackXs[targetIndex];
		};
	float getIRTargetY(int targetIndex) const // Returns the Y position of the given IR tracking target
		{
		return trackYs[targetIndex];
		};
	void setLEDState(int newLedMask); // Sets the status of the Wiimote's LEDs
	void setRumble(bool enable); // Enables/disables the rumble pack
	void requestContinuousReports(bool enable); // Enables/disables continuous data reporting
	void requestAccelerometers(bool enable); // Enables/disables reporting of accelerometer values
	void requestIRTracking(bool enable); // Enables/disables reporting of IR camera tracking data
	};

#endif
