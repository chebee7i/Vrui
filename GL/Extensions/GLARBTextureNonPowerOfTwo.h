/***********************************************************************
GLARBTextureNonPowerOfTwo - OpenGL extension class for the
GL_ARB_texture_non_power_of_two extension.
Copyright (c) 2006 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLEXTENSIONS_GLARBTEXTURENONPOWEROFTWO_INCLUDED
#define GLEXTENSIONS_GLARBTEXTURENONPOWEROFTWO_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_texture_non_power_of_two
#define GL_ARB_texture_non_power_of_two 1

#endif

class GLARBTextureNonPowerOfTwo:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBTextureNonPowerOfTwo*) current; // Pointer to extension object for current OpenGL context
	
	/* Constructors and destructors: */
	private:
	GLARBTextureNonPowerOfTwo(void);
	public:
	virtual ~GLARBTextureNonPowerOfTwo(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
