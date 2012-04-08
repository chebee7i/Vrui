/***********************************************************************
OpenFile - Convenience function to open files of several types using the
BufferedFile abstraction and distribute among a cluster via a multicast
pipe.
Copyright (c) 2011 Oliver Kreylos

This file is part of the Portable Communications Library (Comm).

The Portable Communications Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Portable Communications Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Communications Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Comm/OpenFile.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/FileNameExtensions.h>
#include <IO/StandardFile.h>
#include <IO/GzippedFile.h>
#include <Comm/MulticastPipeMultiplexer.h>
#include <Comm/StandardFile.h>

namespace Comm {

IO::File* openFile(MulticastPipeMultiplexer* multiplexer,const char* fileName,IO::File::AccessMode accessMode)
	{
	if(multiplexer!=0)
		{
		IO::File* result=0;
		
		/* Check if the file name has the .gz extension: */
		if(Misc::hasCaseExtension(fileName,".gz"))
			{
			/* Check if the caller wants to write to the file: */
			if(accessMode!=IO::File::ReadOnly)
				Misc::throwStdErr("Comm::openFile: Cannot write to gzipped files");
			
			/* Open a gzip-compressed file: */
			result=new IO::GzippedFile(fileName);
			}
		else
			{
			if(multiplexer->isMaster())
				{
				/* Open a master-side standard file: */
				result=new StandardFileMaster(*multiplexer,fileName,accessMode);
				}
			else
				{
				/* Open a slave-side standard file: */
				result=new StandardFileSlave(*multiplexer,fileName,accessMode);
				}
			}
		
		/* Return the open file: */
		return result;
		}
	else
		return IO::openFile(fileName,accessMode);
	}

IO::SeekableFile* openSeekableFile(MulticastPipeMultiplexer* multiplexer,const char* fileName,IO::File::AccessMode accessMode)
	{
	if(multiplexer!=0)
		{
		IO::SeekableFile* result=0;
		
		/* Check if the file name has the .gz extension: */
		if(Misc::hasCaseExtension(fileName,".gz"))
			{
			/* Seeking in gzipped files not supported: */
			Misc::throwStdErr("Comm::openSeekableFile: Cannot seek in gzipped files");
			}
		else
			{
			if(multiplexer->isMaster())
				{
				/* Open a master-side standard file: */
				result=new StandardFileMaster(*multiplexer,fileName,accessMode);
				}
			else
				{
				/* Open a slave-side standard file: */
				result=new StandardFileSlave(*multiplexer,fileName,accessMode);
				}
			}
		
		/* Return the open file: */
		return result;
		}
	else
		return IO::openSeekableFile(fileName,accessMode);
	}

}
