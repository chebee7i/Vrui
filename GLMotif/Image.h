/***********************************************************************
Image - Class for widgets displaying image as textures.
Copyright (c) 2011 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMOTIF_IMAGE_INCLUDED
#define GLMOTIF_IMAGE_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Images/RGBImage.h>
#include <GLMotif/Widget.h>

namespace GLMotif {

class Image:public Widget,public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		bool npotdtSupported; // Flag if the OpenGL context supports non-power-of-two-dimension textures
		GLuint textureObjectId; // ID of texture object holding image texture
		unsigned int textureSize[2]; // Width and height of texture containing image
		unsigned int version; // Version number of image in image texture object
		GLfloat regionTex[4]; // Texture coordinates to display current image region
		unsigned int regionVersion; // Version number of displayed image region
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	Images::RGBImage image; // The displayed image
	unsigned int version; // Version number of image
	GLfloat resolution[2]; // The horizontal and vertical resolution of the image in pixels per GLMotif length unit
	GLfloat region[4]; // Region of the image currently mapped to the widget's interior in pixel units
	Box imageBox; // Extents of image inside the widget's interior
	unsigned int regionVersion; // Version number of displayed image region
	
	/* Constructors and destructors: */
	public:
	Image(const char* sName,Container* sParent,const Images::RGBImage& sImage,const GLfloat sResolution[2],bool sManageChild =true); // Creates an image widget displaying the given image at the given resolution
	Image(const char* sName,Container* sParent,const char* imageFileName,const GLfloat sResolution[2],bool sManageChild =true); // Creates an image widget displaying the given image file at the given resolution
	
	/* Methods from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	
	/* New methods: */
	const Images::RGBImage& getImage(void) const // Returns the current image
		{
		return image;
		}
	const GLfloat* getResolution(void) const // Returns the current image's resolution
		{
		return resolution;
		}
	GLfloat getResolution(int dimension) const // Ditto
		{
		return resolution[dimension];
		}
	const GLfloat* getRegion(void) const // Returns the currently displayed image region
		{
		return region;
		}
	GLfloat getRegionMin(int dimension) const // Returns the minimum of the currently displayed image region in the given dimension
		{
		return region[dimension];
		}
	GLfloat getRegionMax(int dimension) const // Returns the maximum of the currently displayed image region in the given dimension
		{
		return region[2+dimension];
		}
	void setRegion(const GLfloat newRegion[4]); // Sets the displayed image region and adapts it to the current widget aspect ratio
	};

}

#endif
