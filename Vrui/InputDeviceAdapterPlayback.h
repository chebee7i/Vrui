/***********************************************************************
InputDeviceAdapterPlayback - Class to read input device states from a
pre-recorded file for playback and/or movie generation.
Copyright (c) 2004-2008 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_INPUTDEVICEADAPTERPLAYBACK_INCLUDED
#define VRUI_INPUTDEVICEADAPTERPLAYBACK_INCLUDED

#include <string>
#include <Misc/File.h>
#include <Geometry/Vector.h>
#include <Vrui/Geometry.h>
#include <Vrui/InputDeviceAdapter.h>

/* Forward declarations: */
namespace Misc {
template <class ValueParam>
void swapEndianness(ValueParam& value);
class ConfigurationFileSection;
}
namespace Sound {
class SoundPlayer;
}
namespace Vrui {
class VRWindow;
}

namespace Vrui {

class InputDeviceAdapterPlayback:public InputDeviceAdapter
	{
	/* Embedded classes: */
	private:
	struct DeviceFileHeader // Structure to store device name and layout in device data files
		{
		/* Elements: */
		public:
		char name[40];
		int trackType;
		int numButtons;
		int numValuators;
		Vector deviceRayDirection;
		};
	
	/* Elements: */
	private:
	Misc::File inputDeviceDataFile; // File containing the input device data
	bool synchronizePlayback; // Flag whether to force the Vrui mainloop to run at the speed of the recording; by default, mainloop runs as fast as it can
	bool quitWhenDone; // Flag whether to quit the Vrui application when all saved data has been played back
	Sound::SoundPlayer* soundPlayer; // Pointer to a sound player object used to play back synchronized commentary tracks
	bool saveMovie; // Flag whether to create a movie by writing screenshots at regular intervals
	std::string movieFileNameTemplate; // Template for creating image file names; must contain exactly one %d placeholder
	int movieWindowIndex; // Index of the master node window from which to save screenshots; awkward, but windows have no names
	VRWindow* movieWindow; // Pointer to the window from which to save screenshots; determined from name on first frame
	double movieFrameTimeInterval; // Time between adjacent frames in the saved movie; == 1.0/movieFrameRate
	bool firstFrame; // Flag to indicate the first frame of the Vrui application
	double timeStamp; // Current time stamp of input device data
	double timeStampOffset; // Offset from system's wall clock time to input data's time stamp sequence
	double nextTimeStamp; // Time stamp of next frame of input device data
	double nextMovieFrameTime; // Time at which to save the next movie frame
	int nextMovieFrameCounter; // Frame index for the next movie frame
	bool done; // Flag if input file is at end
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterPlayback(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection); // Creates adapter by opening and reading pre-recorded device data file
	virtual ~InputDeviceAdapterPlayback(void);
	
	/* Methods: */
	virtual void updateInputDevices(void);
	
	/* New methods: */
	friend void swapEndianness(DeviceFileHeader&);
	bool isDone(void) const // Returns true if file has been entirely read
		{
		return done;
		}
	double getCurrentTime(void) const // Returns current data frame's time stamp
		{
		return timeStamp;
		}
	double getNextTime(void) const // Returns next data frame's time stamp
		{
		return nextTimeStamp;
		}
	};

}

#endif
