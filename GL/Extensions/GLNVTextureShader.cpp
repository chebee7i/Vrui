/***********************************************************************
GLNVTextureShader - OpenGL extension class for the
GL_NV_texture_shader extension.
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

#include <GL/Extensions/GLNVTextureShader.h>

/******************************************
Static elements of class GLNVTextureShader:
******************************************/

GL_THREAD_LOCAL(GLNVTextureShader*) GLNVTextureShader::current=0;

/**********************************
Methods of class GLNVTextureShader:
**********************************/

GLNVTextureShader::GLNVTextureShader(void)
	{
	/* Get all function pointers: */
	}

GLNVTextureShader::~GLNVTextureShader(void)
	{
	}

const char* GLNVTextureShader::getExtensionName(void) const
	{
	return "GL_NV_texture_shader";
	}

void GLNVTextureShader::activate(void)
	{
	current=this;
	}

void GLNVTextureShader::deactivate(void)
	{
	current=0;
	}

bool GLNVTextureShader::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported("GL_NV_texture_shader");
	}

void GLNVTextureShader::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered("GL_NV_texture_shader"))
		{
		/* Create a new extension object: */
		GLNVTextureShader* newExtension=new GLNVTextureShader;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
