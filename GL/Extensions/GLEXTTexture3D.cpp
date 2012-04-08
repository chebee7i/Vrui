/***********************************************************************
GLEXTTexture3D - OpenGL extension class for the
GL_EXT_texture3D extension.
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

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

#include <GL/Extensions/GLEXTTexture3D.h>

/***************************************
Static elements of class GLEXTTexture3D:
***************************************/

GL_THREAD_LOCAL(GLEXTTexture3D*) GLEXTTexture3D::current=0;

/*******************************
Methods of class GLEXTTexture3D:
*******************************/

GLEXTTexture3D::GLEXTTexture3D(void)
	{
	/* Get all function pointers: */
	glTexImage3DEXTProc=GLExtensionManager::getFunction<PFNGLTEXIMAGE3DEXTPROC>("glTexImage3DEXT");
	glTexSubImage3DEXTProc=GLExtensionManager::getFunction<PFNGLTEXSUBIMAGE3DEXTPROC>("glTexSubImage3DEXT");
	}

GLEXTTexture3D::~GLEXTTexture3D(void)
	{
	}

const char* GLEXTTexture3D::getExtensionName(void) const
	{
	return "GL_EXT_texture3D";
	}

void GLEXTTexture3D::activate(void)
	{
	current=this;
	}

void GLEXTTexture3D::deactivate(void)
	{
	current=0;
	}

bool GLEXTTexture3D::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported("GL_EXT_texture3D");
	}

void GLEXTTexture3D::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered("GL_EXT_texture3D"))
		{
		/* Create a new extension object: */
		GLEXTTexture3D* newExtension=new GLEXTTexture3D;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
