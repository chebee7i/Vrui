/***********************************************************************
GLEXTFramebufferObject - OpenGL extension class for the
GL_EXT_framebuffer_object extension.
Copyright (c) 2007-2012 Oliver Kreylos

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

#include <GL/Extensions/GLEXTFramebufferObject.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/***********************************************
Static elements of class GLEXTFramebufferObject:
***********************************************/

GL_THREAD_LOCAL(GLEXTFramebufferObject*) GLEXTFramebufferObject::current=0;

/***************************************
Methods of class GLEXTFramebufferObject:
***************************************/

GLEXTFramebufferObject::GLEXTFramebufferObject(void)
	{
	/* Get all function pointers: */
	glIsRenderbufferEXTProc=GLExtensionManager::getFunction<PFNGLISRENDERBUFFEREXTPROC>("glIsRenderbufferEXT");
	glBindRenderbufferEXTProc=GLExtensionManager::getFunction<PFNGLBINDRENDERBUFFEREXTPROC>("glBindRenderbufferEXT");
	glDeleteRenderbuffersEXTProc=GLExtensionManager::getFunction<PFNGLDELETERENDERBUFFERSEXTPROC>("glDeleteRenderbuffersEXT");
	glGenRenderbuffersEXTProc=GLExtensionManager::getFunction<PFNGLGENRENDERBUFFERSEXTPROC>("glGenRenderbuffersEXT");
	glRenderbufferStorageEXTProc=GLExtensionManager::getFunction<PFNGLRENDERBUFFERSTORAGEEXTPROC>("glRenderbufferStorageEXT");
	glGetRenderbufferParameterivEXTProc=GLExtensionManager::getFunction<PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC>("glGetRenderbufferParameterivEXT");
	glIsFramebufferEXTProc=GLExtensionManager::getFunction<PFNGLISFRAMEBUFFEREXTPROC>("glIsFramebufferEXT");
	glBindFramebufferEXTProc=GLExtensionManager::getFunction<PFNGLBINDFRAMEBUFFEREXTPROC>("glBindFramebufferEXT");
	glDeleteFramebuffersEXTProc=GLExtensionManager::getFunction<PFNGLDELETEFRAMEBUFFERSEXTPROC>("glDeleteFramebuffersEXT");
	glGenFramebuffersEXTProc=GLExtensionManager::getFunction<PFNGLGENFRAMEBUFFERSEXTPROC>("glGenFramebuffersEXT");
	glCheckFramebufferStatusEXTProc=GLExtensionManager::getFunction<PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC>("glCheckFramebufferStatusEXT");
	glFramebufferTexture1DEXTProc=GLExtensionManager::getFunction<PFNGLFRAMEBUFFERTEXTURE1DEXTPROC>("glFramebufferTexture1DEXT");
	glFramebufferTexture2DEXTProc=GLExtensionManager::getFunction<PFNGLFRAMEBUFFERTEXTURE2DEXTPROC>("glFramebufferTexture2DEXT");
	glFramebufferTexture3DEXTProc=GLExtensionManager::getFunction<PFNGLFRAMEBUFFERTEXTURE3DEXTPROC>("glFramebufferTexture3DEXT");
	glFramebufferRenderbufferEXTProc=GLExtensionManager::getFunction<PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC>("glFramebufferRenderbufferEXT");
	glGetFramebufferAttachmentParameterivEXTProc=GLExtensionManager::getFunction<PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC>("glGetFramebufferAttachmentParameterivEXT");
	glGenerateMipmapEXTProc=GLExtensionManager::getFunction<PFNGLGENERATEMIPMAPEXTPROC>("glGenerateMipmapEXT");
	}

GLEXTFramebufferObject::~GLEXTFramebufferObject(void)
	{
	}

const char* GLEXTFramebufferObject::getExtensionName(void) const
	{
	return "GL_EXT_framebuffer_object";
	}

void GLEXTFramebufferObject::activate(void)
	{
	current=this;
	}

void GLEXTFramebufferObject::deactivate(void)
	{
	current=0;
	}

bool GLEXTFramebufferObject::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported("GL_EXT_framebuffer_object");
	}

void GLEXTFramebufferObject::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered("GL_EXT_framebuffer_object"))
		{
		/* Create a new extension object: */
		GLEXTFramebufferObject* newExtension=new GLEXTFramebufferObject;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}

void glPrintFramebufferStatusEXT(std::ostream& stream,const char* tag)
	{
	GLenum status=glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			stream<<tag<<" frame buffer has an incomplete attachment"<<std::endl;
			break;
		
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			stream<<tag<<" frame buffer has no attachments"<<std::endl;
			break;
		
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			stream<<tag<<" frame buffer has attachments with mismatching sizes"<<std::endl;
			break;
		
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			stream<<tag<<" frame buffer has an attachment with an invalid format"<<std::endl;
			break;
		
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			stream<<tag<<" frame buffer is missing a draw buffer attachment"<<std::endl;
			break;
		
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			stream<<tag<<" frame buffer is missing a read buffer attachment"<<std::endl;
			break;
		
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			stream<<tag<<" frame buffer configuration is unsupported by local OpenGL"<<std::endl;
			break;
		
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			break;
		
		default:
			stream<<tag<<" frame buffer is incomplete for unknown reasons"<<std::endl;
		}
	}
