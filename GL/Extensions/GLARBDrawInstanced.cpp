/***********************************************************************
GLARBDrawInstanced - OpenGL extension class for the
GL_ARB_draw_instanced extension.
Copyright (c) 2010 Rolf Westerteiger
Fixes copyright (c) 2010 Oliver Kreylos

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

#include <GL/Extensions/GLARBDrawInstanced.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/*******************************************
Static elements of class GLARBDrawInstanced:
*******************************************/

GL_THREAD_LOCAL(GLARBDrawInstanced*) GLARBDrawInstanced::current=0;

/***********************************
Methods of class GLARBDrawInstanced:
***********************************/

GLARBDrawInstanced::GLARBDrawInstanced(void)
	:glDrawArraysInstancedARBProc(GLExtensionManager::getFunction<PFNGLDRAWARRAYSINSTANCEDARBPROC>("glDrawArraysInstancedARB")),
	 glDrawElementsInstancedARBProc(GLExtensionManager::getFunction<PFNGLDRAWELEMENTSINSTANCEDARBPROC>("glDrawElementsInstancedARB"))
	{
	}

GLARBDrawInstanced::~GLARBDrawInstanced(void)
	{
	}

const char* GLARBDrawInstanced::getExtensionName(void) const
	{
	return "GL_ARB_draw_instanced";
	}

void GLARBDrawInstanced::activate(void)
	{
	current=this;
	}

void GLARBDrawInstanced::deactivate(void)
	{
	current=0;
	}

bool GLARBDrawInstanced::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported("GL_ARB_draw_instanced");
	}

void GLARBDrawInstanced::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered("GL_ARB_draw_instanced"))
		{
		/* Create a new extension object: */
		GLARBDrawInstanced* newExtension=new GLARBDrawInstanced;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
