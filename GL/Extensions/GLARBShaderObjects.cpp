/***********************************************************************
GLARBShaderObjects - OpenGL extension class for the
GL_ARB_shader_objects extension.
Copyright (c) 2007-2008 Oliver Kreylos

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

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

#include <GL/Extensions/GLARBShaderObjects.h>

/*******************************************
Static elements of class GLARBShaderObjects:
*******************************************/

GL_THREAD_LOCAL(GLARBShaderObjects*) GLARBShaderObjects::current=0;

/***********************************
Methods of class GLARBShaderObjects:
***********************************/

GLARBShaderObjects::GLARBShaderObjects(void)
	:glDeleteObjectARBProc(GLExtensionManager::getFunction<PFNGLDELETEOBJECTARBPROC>("glDeleteObjectARB")),
	 glGetHandleARBProc(GLExtensionManager::getFunction<PFNGLGETHANDLEARBPROC>("glGetHandleARB")),
	 glDetachObjectARBProc(GLExtensionManager::getFunction<PFNGLDETACHOBJECTARBPROC>("glDetachObjectARB")),
	 glCreateShaderObjectARBProc(GLExtensionManager::getFunction<PFNGLCREATESHADEROBJECTARBPROC>("glCreateShaderObjectARB")),
	 glShaderSourceARBProc(GLExtensionManager::getFunction<PFNGLSHADERSOURCEARBPROC>("glShaderSourceARB")),
	 glCompileShaderARBProc(GLExtensionManager::getFunction<PFNGLCOMPILESHADERARBPROC>("glCompileShaderARB")),
	 glCreateProgramObjectARBProc(GLExtensionManager::getFunction<PFNGLCREATEPROGRAMOBJECTARBPROC>("glCreateProgramObjectARB")),
	 glAttachObjectARBProc(GLExtensionManager::getFunction<PFNGLATTACHOBJECTARBPROC>("glAttachObjectARB")),
	 glLinkProgramARBProc(GLExtensionManager::getFunction<PFNGLLINKPROGRAMARBPROC>("glLinkProgramARB")),
	 glUseProgramObjectARBProc(GLExtensionManager::getFunction<PFNGLUSEPROGRAMOBJECTARBPROC>("glUseProgramObjectARB")),
	 glValidateProgramARBProc(GLExtensionManager::getFunction<PFNGLVALIDATEPROGRAMARBPROC>("glValidateProgramARB")),
	 glUniform1fARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM1FARBPROC>("glUniform1fARB")),
	 glUniform2fARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM2FARBPROC>("glUniform2fARB")),
	 glUniform3fARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM3FARBPROC>("glUniform3fARB")),
	 glUniform4fARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM4FARBPROC>("glUniform4fARB")),
	 glUniform1iARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM1IARBPROC>("glUniform1iARB")),
	 glUniform2iARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM2IARBPROC>("glUniform2iARB")),
	 glUniform3iARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM3IARBPROC>("glUniform3iARB")),
	 glUniform4iARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM4IARBPROC>("glUniform4iARB")),
	 glUniform1fvARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM1FVARBPROC>("glUniform1fvARB")),
	 glUniform2fvARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM2FVARBPROC>("glUniform2fvARB")),
	 glUniform3fvARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM3FVARBPROC>("glUniform3fvARB")),
	 glUniform4fvARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM4FVARBPROC>("glUniform4fvARB")),
	 glUniform1ivARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM1IVARBPROC>("glUniform1ivARB")),
	 glUniform2ivARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM2IVARBPROC>("glUniform2ivARB")),
	 glUniform3ivARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM3IVARBPROC>("glUniform3ivARB")),
	 glUniform4ivARBProc(GLExtensionManager::getFunction<PFNGLUNIFORM4IVARBPROC>("glUniform4ivARB")),
	 glUniformMatrix2fvARBProc(GLExtensionManager::getFunction<PFNGLUNIFORMMATRIX2FVARBPROC>("glUniformMatrix2fvARB")),
	 glUniformMatrix3fvARBProc(GLExtensionManager::getFunction<PFNGLUNIFORMMATRIX3FVARBPROC>("glUniformMatrix3fvARB")),
	 glUniformMatrix4fvARBProc(GLExtensionManager::getFunction<PFNGLUNIFORMMATRIX4FVARBPROC>("glUniformMatrix4fvARB")),
	 glGetObjectParameterfvARBProc(GLExtensionManager::getFunction<PFNGLGETOBJECTPARAMETERFVARBPROC>("glGetObjectParameterfvARB")),
	 glGetObjectParameterivARBProc(GLExtensionManager::getFunction<PFNGLGETOBJECTPARAMETERIVARBPROC>("glGetObjectParameterivARB")),
	 glGetInfoLogARBProc(GLExtensionManager::getFunction<PFNGLGETINFOLOGARBPROC>("glGetInfoLogARB")),
	 glGetAttachedObjectsARBProc(GLExtensionManager::getFunction<PFNGLGETATTACHEDOBJECTSARBPROC>("glGetAttachedObjectsARB")),
	 glGetUniformLocationARBProc(GLExtensionManager::getFunction<PFNGLGETUNIFORMLOCATIONARBPROC>("glGetUniformLocationARB")),
	 glGetActiveUniformARBProc(GLExtensionManager::getFunction<PFNGLGETACTIVEUNIFORMARBPROC>("glGetActiveUniformARB")),
	 glGetUniformfvARBProc(GLExtensionManager::getFunction<PFNGLGETUNIFORMFVARBPROC>("glGetUniformfvARB")),
	 glGetUniformivARBProc(GLExtensionManager::getFunction<PFNGLGETUNIFORMIVARBPROC>("glGetUniformivARB")),
	 glGetShaderSourceARBProc(GLExtensionManager::getFunction<PFNGLGETSHADERSOURCEARBPROC>("glGetShaderSourceARB"))
	{
	}

GLARBShaderObjects::~GLARBShaderObjects(void)
	{
	}

const char* GLARBShaderObjects::getExtensionName(void) const
	{
	return "GL_ARB_shader_objects";
	}

void GLARBShaderObjects::activate(void)
	{
	current=this;
	}

void GLARBShaderObjects::deactivate(void)
	{
	current=0;
	}

bool GLARBShaderObjects::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported("GL_ARB_shader_objects");
	}

void GLARBShaderObjects::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered("GL_ARB_shader_objects"))
		{
		/* Create a new extension object: */
		GLARBShaderObjects* newExtension=new GLARBShaderObjects;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}

void glCompileShaderFromString(GLhandleARB shaderObject,const char* shaderSource)
	{
	/* Determine the length of the source string: */
	GLint shaderSourceLength=GLint(strlen(shaderSource));
	
	/* Upload the shader source into the shader object: */
	const GLcharARB* ss=reinterpret_cast<const GLcharARB*>(shaderSource);
	glShaderSourceARB(shaderObject,1,&ss,&shaderSourceLength);
	
	/* Compile the shader source: */
	glCompileShaderARB(shaderObject);
	
	/* Check if the shader compiled successfully: */
	GLint compileStatus;
	glGetObjectParameterivARB(shaderObject,GL_OBJECT_COMPILE_STATUS_ARB,&compileStatus);
	if(!compileStatus)
		{
		/* Get some more detailed information: */
		GLcharARB compileLogBuffer[2048];
		GLsizei compileLogSize;
		glGetInfoLogARB(shaderObject,sizeof(compileLogBuffer),&compileLogSize,compileLogBuffer);
		
		/* Signal an error: */
		Misc::throwStdErr("glCompileShaderFromString: Error \"%s\" while compiling shader",compileLogBuffer);
		}
	}

void glCompileShaderFromFile(GLhandleARB shaderObject,const char* shaderSourceFileName)
	{
	/* Open the source file: */
	Misc::File shaderSourceFile(shaderSourceFileName,"rt");
	
	/* Determine the length of the source file: */
	shaderSourceFile.seekEnd(0);
	GLint shaderSourceLength=GLint(shaderSourceFile.tell());
	shaderSourceFile.seekSet(0);
	
	/* Read the shader source: */
	GLcharARB* shaderSource=new GLcharARB[shaderSourceLength];
	shaderSourceFile.read<GLcharARB>(shaderSource,shaderSourceLength);
	
	/* Upload the shader source into the shader object: */
	const GLcharARB* ss=shaderSource;
	glShaderSourceARB(shaderObject,1,&ss,&shaderSourceLength);
	delete[] shaderSource;
	
	/* Compile the shader source: */
	glCompileShaderARB(shaderObject);
	
	/* Check if the shader compiled successfully: */
	GLint compileStatus;
	glGetObjectParameterivARB(shaderObject,GL_OBJECT_COMPILE_STATUS_ARB,&compileStatus);
	if(!compileStatus)
		{
		/* Get some more detailed information: */
		GLcharARB compileLogBuffer[2048];
		GLsizei compileLogSize;
		glGetInfoLogARB(shaderObject,sizeof(compileLogBuffer),&compileLogSize,compileLogBuffer);
		
		/* Signal an error: */
		Misc::throwStdErr("glCompileShaderFromFile: Error \"%s\" while compiling shader %s",compileLogBuffer,shaderSourceFileName);
		}
	}

GLhandleARB glLinkShader(const std::vector<GLhandleARB>& shaderObjects)
	{
	/* Create the program object: */
	GLhandleARB programObject=glCreateProgramObjectARB();
	
	/* Attach all shader objects to the shader program: */
	for(std::vector<GLhandleARB>::const_iterator soIt=shaderObjects.begin();soIt!=shaderObjects.end();++soIt)
		glAttachObjectARB(programObject,*soIt);
	
	/* Link the program: */
	glLinkProgramARB(programObject);
	
	/* Check if the program linked successfully: */
	GLint linkStatus;
	glGetObjectParameterivARB(programObject,GL_OBJECT_LINK_STATUS_ARB,&linkStatus);
	if(!linkStatus)
		{
		/* Get some more detailed information: */
		GLcharARB linkLogBuffer[2048];
		GLsizei linkLogSize;
		glGetInfoLogARB(programObject,sizeof(linkLogBuffer),&linkLogSize,linkLogBuffer);
		
		/* Signal an error: */
		Misc::throwStdErr("glLinkShader: Error \"%s\" while linking shader program",linkLogBuffer);
		}
	
	return programObject;
	}

GLhandleARB glLinkShader(GLhandleARB vertexShaderObject,GLhandleARB fragmentShaderObject)
	{
	/* Create the program object: */
	GLhandleARB programObject=glCreateProgramObjectARB();
	
	/* Attach all shader objects to the shader program: */
	glAttachObjectARB(programObject,vertexShaderObject);
	glAttachObjectARB(programObject,fragmentShaderObject);
	
	/* Link the program: */
	glLinkProgramARB(programObject);
	
	/* Check if the program linked successfully: */
	GLint linkStatus;
	glGetObjectParameterivARB(programObject,GL_OBJECT_LINK_STATUS_ARB,&linkStatus);
	if(!linkStatus)
		{
		/* Get some more detailed information: */
		GLcharARB linkLogBuffer[2048];
		GLsizei linkLogSize;
		glGetInfoLogARB(programObject,sizeof(linkLogBuffer),&linkLogSize,linkLogBuffer);
		
		/* Signal an error: */
		Misc::throwStdErr("glLinkShader: Error \"%s\" while linking shader program",linkLogBuffer);
		}
	
	return programObject;
	}
