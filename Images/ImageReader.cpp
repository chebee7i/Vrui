/***********************************************************************
ImageReader - Abstract base class to read images from files in a variety
of image file formats.
Copyright (c) 2012-2013 Oliver Kreylos

This file is part of the Image Handling Library (Images).

The Image Handling Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Image Handling Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Image Handling Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Images/ImageReader.h>

namespace Images {

/****************************
Methods of class ImageReader:
****************************/

ImageReader::ImageReader(IO::FilePtr sFile)
	:file(sFile)
	{
	/* Initialize the canvas size: */
	canvasSize[0]=canvasSize[1]=0U;
	
	/* Initialize the image specification: */
	imageSpec.offset[0]=imageSpec.offset[1]=0U;
	imageSpec.size[0]=imageSpec.size[1]=0U;
	imageSpec.colorSpace=Grayscale;
	imageSpec.hasAlpha=false;
	imageSpec.numChannels=0;
	imageSpec.channelSpecs=0;
	}

ImageReader::~ImageReader(void)
	{
	/* Delete the image specification: */
	delete[] imageSpec.channelSpecs;
	}

}
