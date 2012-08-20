/***********************************************************************
VRWindow - Class for OpenGL windows that are used to map one or two eyes
of a viewer onto a VR screen.
Copyright (c) 2004-2012 Oliver Kreylos
ZMap stereo mode additions copyright (c) 2011 Matthias Deller.

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

#define GLX_GLXEXT_PROTOTYPES 1
#define SAVE_SCREENSHOT_PROJECTION 0
#define RENDERFRAMETIMES 0

#include <Vrui/VRWindow.h>

#include <Images/Config.h>

#include <iostream>
#include <X11/keysym.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/CallbackData.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/ConfigurationFile.h>
#if SAVE_SCREENSHOT_PROJECTION
#include <IO/File.h>
#endif
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/AffineTransformation.h>
#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
// #include <GL/glxext.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLPrintError.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/Extensions/GLEXTFramebufferObject.h>
#include <GL/GLShader.h>
#include <GL/GLContextData.h>
#include <GL/GLFont.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/WidgetManager.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>
#include <Images/WriteImageFile.h>
#include <Vrui/Vrui.h>
#if SAVE_SCREENSHOT_PROJECTION
#include <Vrui/OpenFile.h>
#endif
#include <Vrui/InputDevice.h>
#include <Vrui/Internal/InputDeviceAdapterMouse.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/WindowProperties.h>
#include <Vrui/ViewSpecification.h>
#include <Vrui/Tool.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Internal/ToolKillZone.h>
#include <Vrui/Internal/MovieSaver.h>
#include <Vrui/Internal/Vrui.h>

#if SAVE_SCREENSHOT_PROJECTION
#include <Misc/File.h>
#include <GL/GLTransformationWrappers.h>
#endif

namespace Misc {

/***********************************
Helper class to decode window types:
***********************************/

template <>
class ValueCoder<Vrui::VRWindow::WindowType>
	{
	/* Methods: */
	public:
	static std::string encode(const Vrui::VRWindow::WindowType& value)
		{
		switch(value)
			{
			case Vrui::VRWindow::MONO:
				return "Mono";
			
			case Vrui::VRWindow::LEFT:
				return "LeftEye";
			
			case Vrui::VRWindow::RIGHT:
				return "RightEye";
			
			case Vrui::VRWindow::QUADBUFFER_STEREO:
				return "QuadbufferStereo";
			
			case Vrui::VRWindow::ANAGLYPHIC_STEREO:
				return "AnaglyphicStereo";
			
			case Vrui::VRWindow::SPLITVIEWPORT_STEREO:
				return "SplitViewportStereo";
			
			case Vrui::VRWindow::INTERLEAVEDVIEWPORT_STEREO:
				return "InterleavedViewportStereo";
			
			case Vrui::VRWindow::AUTOSTEREOSCOPIC_STEREO:
				return "AutoStereoscopicStereo";
			}
		}
	static Vrui::VRWindow::WindowType decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		if(end-start>=4&&strncasecmp(start,"Mono",4)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+4;
			return Vrui::VRWindow::MONO;
			}
		else if(end-start>=7&&strncasecmp(start,"LeftEye",7)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+7;
			return Vrui::VRWindow::LEFT;
			}
		else if(end-start>=8&&strncasecmp(start,"RightEye",8)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+8;
			return Vrui::VRWindow::RIGHT;
			}
		else if(end-start>=16&&strncasecmp(start,"QuadbufferStereo",16)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+16;
			return Vrui::VRWindow::QUADBUFFER_STEREO;
			}
		else if(end-start>=16&&strncasecmp(start,"AnaglyphicStereo",16)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+16;
			return Vrui::VRWindow::ANAGLYPHIC_STEREO;
			}
		else if(end-start>=18&&strncasecmp(start,"SplitViewportStereo",18)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+18;
			return Vrui::VRWindow::SPLITVIEWPORT_STEREO;
			}
		else if(end-start>=25&&strncasecmp(start,"InterleavedViewportStereo",25)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+25;
			return Vrui::VRWindow::INTERLEAVEDVIEWPORT_STEREO;
			}
		else if(end-start>=22&&strncasecmp(start,"AutoStereoscopicStereo",22)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+22;
			return Vrui::VRWindow::AUTOSTEREOSCOPIC_STEREO;
			}
		else
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to VRWindow::WindowType"));
		}
	};

/*************************************************
Helper class to decode window positions and sizes:
*************************************************/

template <>
class ValueCoder<GLWindow::WindowPos>
	{
	/* Methods: */
	public:
	static std::string encode(const GLWindow::WindowPos& value)
		{
		std::string result;
		
		result.append(FixedArrayValueCoder<int>::encode(value.origin,2));
		result.push_back(',');
		result.push_back(' ');
		result.append(FixedArrayValueCoder<int>::encode(value.size,2));
		return result;
		}
	static GLWindow::WindowPos decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		try
			{
			GLWindow::WindowPos result;
			const char* cPtr=start;
			
			/* Parse the origin: */
			FixedArrayValueCoder<int>::decode(result.origin,2,cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			
			/* Check for separating comma: */
			cPtr=checkSeparator(',',cPtr,end);
			
			/* Parse the size: */
			FixedArrayValueCoder<int>::decode(result.size,2,cPtr,end,&cPtr);
			
			if(decodeEnd!=0)
				*decodeEnd=cPtr;
			return result;
			}
		catch(std::runtime_error err)
			{
			throw DecodingError(std::string("Unable to convert ")+std::string(start,end)+std::string(" to GLWindow::WindowPos due to ")+err.what());
			}
		}
	};

}

namespace Vrui {

#if RENDERFRAMETIMES
const int numFrameTimes=800;
extern double frameTimes[];
extern int frameTimeIndex;
#endif

/*************************
Methods of class VRWindow:
*************************/

std::string VRWindow::getDisplayName(const Misc::ConfigurationFileSection& configFileSection)
	{
	const char* defaultDisplay=getenv("DISPLAY");
	if(defaultDisplay==0)
		defaultDisplay=":0.0";
	return configFileSection.retrieveString("./display",defaultDisplay);
	}

int* VRWindow::getVisualProperties(const WindowProperties& properties,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Create a list of desired visual properties: */
	static int visualPropertyList[256];
	int numProperties=0;
	
	/* Add standard properties first: */
	visualPropertyList[numProperties++]=GLX_RGBA;
	visualPropertyList[numProperties++]=GLX_DOUBLEBUFFER;
	
	/* Ask for the requested main buffer channel sizes: */
	visualPropertyList[numProperties++]=GLX_RED_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[0];
	visualPropertyList[numProperties++]=GLX_GREEN_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[1];
	visualPropertyList[numProperties++]=GLX_BLUE_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[2];
	visualPropertyList[numProperties++]=GLX_ALPHA_SIZE;
	visualPropertyList[numProperties++]=properties.colorBufferSize[3];
	
	/* Ask for the requested depth buffer size: */
	visualPropertyList[numProperties++]=GLX_DEPTH_SIZE;
	visualPropertyList[numProperties++]=properties.depthBufferSize;
	
	if(properties.numAuxBuffers>0)
		{
		/* Ask for auxiliary buffers: */
		visualPropertyList[numProperties++]=GLX_AUX_BUFFERS;
		visualPropertyList[numProperties++]=properties.numAuxBuffers;
		}
	
	if(properties.stencilBufferSize>0)
		{
		/* Ask for a stencil buffer: */
		visualPropertyList[numProperties++]=GLX_STENCIL_SIZE;
		visualPropertyList[numProperties++]=properties.stencilBufferSize;
		}
	
	if(properties.accumBufferSize[0]>0||properties.accumBufferSize[1]>0||properties.accumBufferSize[2]>0||properties.accumBufferSize[3]>0)
		{
		/* Ask for an accumulation buffer of the requested channel sizes: */
		visualPropertyList[numProperties++]=GLX_ACCUM_RED_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[0];
		visualPropertyList[numProperties++]=GLX_ACCUM_GREEN_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[1];
		visualPropertyList[numProperties++]=GLX_ACCUM_BLUE_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[2];
		visualPropertyList[numProperties++]=GLX_ACCUM_ALPHA_SIZE;
		visualPropertyList[numProperties++]=properties.accumBufferSize[3];
		}
	
	/* Check for multisample requests: */
	int multisamplingLevel=configFileSection.retrieveValue<int>("./multisamplingLevel",1);
	if(multisamplingLevel>1)
		{
		visualPropertyList[numProperties++]=GLX_SAMPLE_BUFFERS_ARB;
		visualPropertyList[numProperties++]=1;
		visualPropertyList[numProperties++]=GLX_SAMPLES_ARB;
		visualPropertyList[numProperties++]=multisamplingLevel;
		}
	
	/* Check for quad buffering (active stereo) requests: */
	if(configFileSection.retrieveValue<WindowType>("./windowType")==QUADBUFFER_STEREO)
		{
		visualPropertyList[numProperties++]=GLX_STEREO;
		}
	
	/* Finish and return the property list: */
	visualPropertyList[numProperties]=None;
	return visualPropertyList;
	}

void VRWindow::render(const GLWindow::WindowPos& viewportPos,int screenIndex,const Point& eye)
	{
	/*********************************************************************
	First step: Re-initialize OpenGL state and clear all buffers.
	*********************************************************************/
	
	/* Initialize standard OpenGL settings: */
	glDisable(GL_ALPHA_TEST);
	glAlphaFunc(GL_ALWAYS,0.0f);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
		{
		glDisable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS,0,~0x0U);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		glStencilMask(~0x0U);
		}
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	
	/* Clear all relevant buffers: */
	glClearColor(getBackgroundColor());
	glClearDepth(1.0); // Clear depth is "infinity"
	if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
		glClearStencil(0x0);
	if(clearBufferMask&GL_ACCUM_BUFFER_BIT)
		glClearAccum(0.0f,0.0f,0.0f,0.0f);
	glClear(clearBufferMask);
	
	/*********************************************************************
	Second step: Set up the projection and modelview matrices to project
	from the given eye onto the given screen.
	*********************************************************************/
	
	/* Get the inverse of the current screen transformation: */
	ONTransform invScreenT=screens[screenIndex]->getScreenTransformation();
	invScreenT.doInvert();
	
	/* Transform the eye position to screen coordinates: */
	Point screenEyePos=invScreenT.transform(eye);
	
	/* Set up projection matrix: */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	/* Check if the screen is projected off-axis: */
	if(screens[screenIndex]->isOffAxis())
		{
		/* Apply the screen's off-axis correction homography: */
		glMultMatrix(screens[screenIndex]->getInverseClipHomography());
		}
	
	/* Apply the screen's frustum transformation: */
	double near=getFrontplaneDist();
	double far=getBackplaneDist();
	double left=(viewports[screenIndex][0]-screenEyePos[0])/screenEyePos[2]*near;
	double right=(viewports[screenIndex][1]-screenEyePos[0])/screenEyePos[2]*near;
	double bottom=(viewports[screenIndex][2]-screenEyePos[1])/screenEyePos[2]*near;
	double top=(viewports[screenIndex][3]-screenEyePos[1])/screenEyePos[2]*near;
	glFrustum(left,right,bottom,top,near,far);
	
	/* Calculate the base modelview matrix: */
	OGTransform modelview=OGTransform::translateToOriginFrom(screenEyePos);
	modelview*=OGTransform(invScreenT);
	
	/*********************************************************************
	Third step: Render Vrui state.
	*********************************************************************/
	
	/* Update the window's display state object: */
	displayState->resized=resizeViewport;
	displayState->eyePosition=eye;
	displayState->screen=screens[screenIndex];
	
	/* Store the physical and navigational modelview matrices: */
	displayState->modelviewPhysical=modelview;
	modelview*=getNavigationTransformation();
	modelview.renormalize();
	displayState->modelviewNavigational=modelview;
	
	vruiState->display(displayState,*contextData);
	
	/*********************************************************************
	Fourth step: Render screen protectors and fps counter.
	*********************************************************************/
	
	if(protectScreens&&vruiState->numProtectors>0)
		{
		/* Check if any monitored input device is dangerously close to the screen: */
		bool renderProtection=false;
		for(int i=0;i<vruiState->numProtectors;++i)
			{
			VruiState::ScreenProtector& sp=vruiState->protectors[i];
			
			/* Transform device protection sphere center to screen coordinates: */
			Point p=sp.inputDevice->getTransformation().transform(sp.center);
			p=invScreenT.transform(p);
			if(p[2]>-sp.radius&&p[2]<sp.radius)
				if(p[0]>-sp.radius&&p[0]<screens[screenIndex]->getWidth()+sp.radius&&p[1]>-sp.radius&&p[1]<screens[screenIndex]->getHeight()+sp.radius)
					{
					renderProtection=true;
					break;
					}
			}
		
		if(renderProtection)
			{
			/* Set OpenGL matrices to pixel-based: */
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0,viewportPos.size[0],0,viewportPos.size[1],0,1);
			
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			
			/* Render grid onto screen: */
			glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
			glDisable(GL_LIGHTING);
			glLineWidth(1.0f);
			glColor3f(0.0f,1.0f,0.0f);
			glBegin(GL_LINES);
			for(int x=0;x<=10;++x)
				{
				int pos=x*(viewportPos.size[0]-1)/10;
				glVertex2i(pos,0);
				glVertex2i(pos,viewportPos.size[1]);
				}
			for(int y=0;y<=10;++y)
				{
				int pos=y*(viewportPos.size[1]-1)/10;
				glVertex2i(0,pos);
				glVertex2i(viewportPos.size[0],pos);
				}
			glEnd();
			glPopAttrib();
			
			/* Reset the OpenGL matrices: */
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			}
		}
	
	if(showFps&&burnMode)
		{
		/* Set OpenGL matrices to pixel-based: */
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0,viewportPos.size[0],0,viewportPos.size[1],0,1);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		
		#if RENDERFRAMETIMES
		/* Render EKG of recent frame rates: */
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(0.0f,1.0f,0.0f);
		for(int i=0;i<numFrameTimes;++i)
			if(i!=frameTimeIndex)
				{
				glVertex2i(i,0);
				glVertex2i(i,int(floor(frameTimes[i]*1000.0+0.5)));
				}
		glColor3f(1.0f,0.0f,0.0f);
		glVertex2i(frameTimeIndex,0);
		glVertex2i(frameTimeIndex,int(floor(frameTimes[frameTimeIndex]*1000.0+0.5)));
		glEnd();
		glEnable(GL_LIGHTING);
		#else
		/* Print the current frame time: */
		unsigned int fps=(unsigned int)(10.0/vruiState->currentFrameTime+0.5);
		char buffer[20];
		char* bufPtr=buffer+15;
		*(--bufPtr)=char(fps%10+'0');
		fps/=10;
		*(--bufPtr)='.';
		do
			{
			*(--bufPtr)=char(fps%10+'0');
			fps/=10;
			}
		while(bufPtr>buffer&&fps!=0);
		buffer[15]=' ';
		buffer[16]='f';
		buffer[17]='p';
		buffer[18]='s';
		buffer[19]='\0';
		glDisable(GL_LIGHTING);
		showFpsFont->drawString(GLFont::Vector(showFpsFont->getCharacterWidth()*9.5f+2.0f,2.0f,0.0f),bufPtr);
		glEnable(GL_LIGHTING);
		#endif
		
		/* Reset the OpenGL matrices: */
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		}
	}

bool VRWindow::calcMousePos(int x,int y,Scalar mousePos[2]) const
	{
	if(windowType==SPLITVIEWPORT_STEREO)
		{
		/* Find viewport containing mouse: */
		for(int i=0;i<2;++i)
			{
			int vx=x-splitViewportPos[i].origin[0];
			int vy=(getWindowHeight()-1-y)-splitViewportPos[i].origin[1];
			if(vx>=0&&vx<splitViewportPos[i].size[0]&&vy>=0&&vy<splitViewportPos[i].size[1])
				{
				/* Calculate mouse position based on found viewport: */
				mousePos[0]=(Scalar(vx)+Scalar(0.5))*screens[i]->getWidth()/Scalar(splitViewportPos[i].size[0]);
				mousePos[1]=(Scalar(vy)+Scalar(0.5))*screens[i]->getHeight()/Scalar(splitViewportPos[i].size[1]);
				return true;
				}
			}
		return false;
		}
	else
		{
		/* Calculate mouse position based on entire window: */
		if(panningViewport)
			{
			mousePos[0]=(Scalar(getWindowOrigin()[0]+x)+Scalar(0.5))*screens[0]->getWidth()/Scalar(displaySize[0]);
			mousePos[1]=(Scalar(displaySize[1]-getWindowOrigin()[1]-y)-Scalar(0.5))*screens[0]->getHeight()/Scalar(displaySize[1]);
			}
		else
			{
			mousePos[0]=(Scalar(x)+Scalar(0.5))*screens[0]->getWidth()/getWindowWidth();
			mousePos[1]=(getWindowHeight()-Scalar(y)-Scalar(0.5))*screens[0]->getHeight()/getWindowHeight();
			}
		return true;
		}
	}

VRWindow::VRWindow(const char* windowName,const Misc::ConfigurationFileSection& configFileSection,VruiState* sVruiState,InputDeviceAdapterMouse* sMouseAdapter)
	:GLWindow(getDisplayName(configFileSection).c_str(),
	          windowName,
	          configFileSection.retrieveValue<GLWindow::WindowPos>("./windowPos",GLWindow::WindowPos(800,600)),
	          configFileSection.retrieveValue<bool>("./decorate",true),
	          getVisualProperties(sVruiState->windowProperties,configFileSection)),
	 vruiState(sVruiState),
	 mouseAdapter(sMouseAdapter),
	 clearBufferMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT),
	 extensionManager(0),
	 contextData(0),displayState(0),
	 viewer(findViewer(configFileSection.retrieveString("./viewerName").c_str())),
	 windowType(configFileSection.retrieveValue<WindowType>("./windowType")),
	 multisamplingLevel(configFileSection.retrieveValue<int>("./multisamplingLevel",1)),
	 panningViewport(configFileSection.retrieveValue<bool>("./panningViewport",false)),
	 navigate(configFileSection.retrieveValue<bool>("./navigate",false)),
	 movePrimaryWidgets(configFileSection.retrieveValue<bool>("./movePrimaryWidgets",false)),
	 hasFramebufferObjectExtension(false),
	 ivEyeIndexOffset(0),
	 ivRightViewportTextureID(0),ivRightDepthbufferObjectID(0),ivRightFramebufferObjectID(0),
	 asNumViewZones(0),asViewZoneOffset(0),
	 asViewMapTextureID(0),asViewZoneTextureID(0),
	 asDepthBufferObjectID(0),asFrameBufferObjectID(0),
	 asInterzigShader(0),asQuadSizeUniformIndex(-1),
	 showFpsFont(0),
	 showFps(configFileSection.retrieveValue<bool>("./showFps",false)),burnMode(false),
	 protectScreens(configFileSection.retrieveValue<bool>("./protectScreens",true)),
	 trackToolKillZone(false),
	 dirty(true),
	 resizeViewport(true),
	 saveScreenshot(false),
	 movieSaver(0)
	{
	/* Initialize the window mouse position: */
	windowMousePos[0]=windowMousePos[1]=0;
	
	/* Update the clear buffer bit mask: */
	if(vruiState->windowProperties.stencilBufferSize>0)
		clearBufferMask|=GL_STENCIL_BUFFER_BIT;
	if(vruiState->windowProperties.accumBufferSize[0]>0||vruiState->windowProperties.accumBufferSize[1]>0||vruiState->windowProperties.accumBufferSize[2]>0||vruiState->windowProperties.accumBufferSize[3]>0)
		clearBufferMask|=GL_ACCUM_BUFFER_BIT;
	
	/* Get the screen(s) this window projects onto: */
	screens[0]=findScreen(configFileSection.retrieveString("./leftScreenName","").c_str());
	screens[1]=findScreen(configFileSection.retrieveString("./rightScreenName","").c_str());
	if(screens[0]==0||screens[1]==0)
		{
		/* Get the common screen: */
		screens[0]=screens[1]=findScreen(configFileSection.retrieveString("./screenName").c_str());
		}
	if(screens[0]==0||screens[1]==0)
		Misc::throwStdErr("VRWindow::VRWindow: No screen(s) provided");
	
	/* Get the size of the entire display in pixels: */
	WindowPos rootWindowPos=getRootWindowPos();
	for(int i=0;i<2;++i)
		displaySize[i]=rootWindowPos.size[i];
	
	/* Initialize other window state: */
	for(int i=0;i<2;++i)
		{
		ivTextureSize[i]=0;
		ivTexCoord[i]=0.0f;
		asNumTiles[i]=0;
		asTextureSize[i]=0;
		}
	for(int i=0;i<4;++i)
		ivRightStipplePatterns[i]=0;
	
	/* Check if the window has a viewer: */
	if(viewer==0)
		Misc::throwStdErr("VRWindow::VRWindow: No viewer provided");
	
	/* Check if the window's screen size should be defined based on the X display's real size: */
	if(configFileSection.retrieveValue<bool>("./autoScreenSize",false))
		{
		/* Query the size of the X11 screen in physical units: */
		Scalar w=Scalar(getScreenWidthMM())*getInchFactor()/Scalar(25.4);
		Scalar h=Scalar(getScreenHeightMM())*getInchFactor()/Scalar(25.4);
		
		/* Query the screen's configured size (use mean of both screens, assuming they're the same): */
		Scalar oldSize=Scalar(1);
		for(int i=0;i<2;++i)
			oldSize*=Math::sqrt(Math::sqr(screens[0]->getWidth())+Math::sqr(screens[1]->getWidth()));
		oldSize=Math::sqrt(oldSize);
		
		/* Adjust the size of the screen used by this window: */
		screens[0]->setSize(w,h);
		if(screens[1]!=screens[0])
			screens[1]->setSize(w,h);
		Scalar newSize=Math::sqrt(Math::sqr(w)+Math::sqr(h));
		
		/* Adjust the size of the display environment: */
		setDisplayCenter(getDisplayCenter(),getDisplaySize()*newSize/oldSize);
		
		/* Try activating a fake navigation tool: */
		if(activateNavigationTool(reinterpret_cast<Tool*>(this)))
			{
			/* Adjust the navigation transformation to the new display size: */
			NavTransform nav=NavTransform::translateFromOriginTo(getDisplayCenter());
			nav*=NavTransform::scale(newSize/oldSize);
			nav*=NavTransform::translateToOriginFrom(getDisplayCenter());
			concatenateNavigationTransformationLeft(nav);
			
			/* Deactivate the fake navigation tool: */
			deactivateNavigationTool(reinterpret_cast<Tool*>(this));
			}
		}
	
	/* Make the window full screen if requested: */
	if(configFileSection.retrieveValue<bool>("./windowFullscreen",false))
		makeFullscreen();
	
	if(windowType==SPLITVIEWPORT_STEREO)
		{
		/* Read split viewport positions: */
		splitViewportPos[0]=configFileSection.retrieveValue<GLWindow::WindowPos>("./leftViewportPos");
		splitViewportPos[1]=configFileSection.retrieveValue<GLWindow::WindowPos>("./rightViewportPos");
		}
	
	/* Initialize the window's panning viewport state: */
	if(panningViewport)
		{
		/* Adapt the viewports to the size of the window in relation to the size of the display: */
		for(int i=0;i<2;++i)
			{
			viewports[i][0]=Scalar(getWindowOrigin()[0])*screens[i]->getWidth()/Scalar(displaySize[0]);
			viewports[i][1]=Scalar(getWindowOrigin()[0]+getWindowWidth())*screens[i]->getWidth()/Scalar(displaySize[0]);
			viewports[i][2]=Scalar(displaySize[1]-getWindowOrigin()[1]-getWindowHeight())*screens[i]->getHeight()/Scalar(displaySize[1]);
			viewports[i][3]=Scalar(displaySize[1]-getWindowOrigin()[1])*screens[i]->getHeight()/Scalar(displaySize[1]);
			}
		
		if(navigate)
			{
			/* Navigate such that the previous transformation, which assumed a full-screen window, fits into the actual window: */
			Point screenCenter(Math::div2(screens[0]->getWidth()),Math::div2(screens[0]->getHeight()),Scalar(0));
			Scalar screenSize=Math::sqrt(Math::sqr(screens[0]->getWidth())+Math::sqr(screens[0]->getHeight()));
			Point windowCenter;
			Scalar windowSize(0);
			for(int i=0;i<2;++i)
				{
				windowCenter[i]=Math::mid(viewports[0][i*2+0],viewports[0][i*2+1]);
				windowSize+=Math::sqr(viewports[0][i*2+1]-viewports[0][i*2+0]);
				}
			windowCenter[2]=Scalar(0);
			windowSize=Math::sqrt(windowSize);
			
			ONTransform screenT=screens[0]->getScreenTransformation();
			screenCenter=screenT.transform(screenCenter);
			windowCenter=screenT.transform(windowCenter);
			
			/* Try activating a fake navigation tool: */
			if(activateNavigationTool(reinterpret_cast<Tool*>(this)))
				{
				/* Scale to fit the old viewport into the new viewport: */
				NavTransform nav=NavTransform::translateFromOriginTo(windowCenter);
				nav*=NavTransform::scale(windowSize/screenSize);
				nav*=NavTransform::translateToOriginFrom(windowCenter);
				
				/* Translate to move to the new viewport center: */
				nav*=NavTransform::translate(windowCenter-screenCenter);
				
				/* Apply the transformation: */
				concatenateNavigationTransformationLeft(nav);
				
				/* Deactivate the fake navigation tool: */
				deactivateNavigationTool(reinterpret_cast<Tool*>(this));
				}
			
			/* Update the display center and size: */
			setDisplayCenter(windowCenter,getDisplaySize()*windowSize/screenSize);
			}
		}
	else
		{
		/* Set the viewport dimensions to the full screen: */
		for(int i=0;i<2;++i)
			screens[i]->getViewport(viewports[i]);
		}
	
	/* Check if the window is supposed to track the tool manager's tool kill zone: */
	try
		{
		/* Parse the tool kill zone's relative window position: */
		std::string toolKillZonePosValue=configFileSection.retrieveString("./toolKillZonePos");
		const char* start=toolKillZonePosValue.data();
		const char* end=start+toolKillZonePosValue.length();
		Misc::FixedArrayValueCoder<Scalar>::decode(toolKillZonePos,2,start,end,0);
		trackToolKillZone=true;
		
		/* Move the tool kill zone to the desired position: */
		ToolKillZone* toolKillZone=getToolManager()->getToolKillZone();
		Vector toolKillZoneSize=screens[0]->getScreenTransformation().inverseTransform(Vector(toolKillZone->getSize()));
		Point screenPos;
		for(int i=0;i<2;++i)
			{
			Scalar min=viewports[0][2*i+0]+toolKillZoneSize[i]*Scalar(0.5);
			Scalar max=viewports[0][2*i+1]-toolKillZoneSize[i]*Scalar(0.5);
			screenPos[i]=min+(max-min)*toolKillZonePos[i];
			}
		screenPos[2]=Scalar(0);
		toolKillZone->setCenter(screens[0]->getScreenTransformation().transform(screenPos));
		vruiState->navigationTransformationChangedMask|=0x4;
		}
	catch(...)
		{
		}
	
	/* Hide mouse cursor and ignore mouse events if the mouse is not used as an input device: */
	if(mouseAdapter==0||!mouseAdapter->needMouseCursor())
		{
		hideCursor();
		if(mouseAdapter==0)
			disableMouseEvents();
		}
	
	/* Initialize the window's OpenGL context: */
	makeCurrent();
	extensionManager=new GLExtensionManager;
	GLExtensionManager::makeCurrent(extensionManager);
	contextData=new GLContextData(101);
	displayState=vruiState->registerContext(*contextData);
	displayState->window=this;
	displayState->viewer=viewer;
	displayState->eyeIndex=0;
	glViewport(0,0,getWindowWidth(),getWindowHeight());
	glClearColor(getBackgroundColor());
	glClearDepth(1.0);
	if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
		glClearStencil(0x0);
	if(clearBufferMask&GL_ACCUM_BUFFER_BIT)
		glClearAccum(0.0f,0.0f,0.0f,0.0f);
	if(multisamplingLevel>1)
		glEnable(GL_MULTISAMPLE_ARB);
	
	if(windowType==INTERLEAVEDVIEWPORT_STEREO)
		{
		/* Create the viewport buffer texture for the right viewport rendering pass: */
		for(int i=0;i<2;++i)
			{
			for(ivTextureSize[i]=1;ivTextureSize[i]<GLWindow::getWindowSize()[i];ivTextureSize[i]<<=1)
				;
			ivTexCoord[i]=float(GLWindow::getWindowSize()[i])/float(ivTextureSize[i]);
			}
		ivEyeIndexOffset=(GLWindow::getWindowOrigin()[1]%2)*2+(GLWindow::getWindowOrigin()[0]%2);
		glGenTextures(1,&ivRightViewportTextureID);
		glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,ivTextureSize[0],ivTextureSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Check if the local OpenGL supports frame buffer objects: */
		hasFramebufferObjectExtension=GLEXTFramebufferObject::isSupported();
		if(hasFramebufferObjectExtension)
			{
			/* Initialize the extension: */
			GLEXTFramebufferObject::initExtension();
			
			/* Create a depthbuffer object for the right viewport rendering pass: */
			glGenRenderbuffersEXT(1,&ivRightDepthbufferObjectID);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,ivRightDepthbufferObjectID);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT,ivTextureSize[0],ivTextureSize[1]);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
			
			/* Create a framebuffer object for the right viewport rendering pass: */
			glGenFramebuffersEXT(1,&ivRightFramebufferObjectID);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,ivRightFramebufferObjectID);
			
			/* Attach the viewport texture and the depthbuffer to the framebuffer: */
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,ivRightViewportTextureID,0);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,ivRightDepthbufferObjectID);
			
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
			}
		
		/* Read the target display's interleave pattern: */
		std::string pattern=configFileSection.retrieveValue<std::string>("./interleavePattern","LRRL");
		bool patRight[2][2];
		bool patternValid=pattern.size()==4;
		for(int y=0;patternValid&&y<2;++y)
			for(int x=0;patternValid&&x<2;++x)
				{
				char pat=toupper(pattern[y*2+x]);
				patternValid=pat=='L'||pat=='R';
				patRight[y][x]=pat=='R';
				}
		if(!patternValid)
			Misc::throwStdErr("VRWindow::VRWindow: Invalid interleave pattern %s ",pattern.c_str());
		
		/* Initialize the interleave stipple patterns: */
		for(int yoff=0;yoff<2;++yoff)
			for(int xoff=0;xoff<2;++xoff)
				{
				GLubyte* stipPat=ivRightStipplePatterns[yoff*2+xoff]=new GLubyte[128];
				for(int i=0;i<128;++i)
					stipPat[i]=GLubyte(0);
				for(int y=0;y<32;++y)
					for(int x=0;x<32;++x)
						if(patRight[(y+yoff)%2][(x+xoff)%2])
							stipPat[y*4+x/8]|=0x1U<<(x%8);
				}
		}
	else if(windowType==AUTOSTEREOSCOPIC_STEREO)
		{
		/* Check if the local OpenGL supports the required extensions: */
		if(!GLARBMultitexture::isSupported())
			Misc::throwStdErr("VRWindow::VRWindow: Local OpenGL does not support multitexturing");
		if(!GLShader::isSupported())
			Misc::throwStdErr("VRWindow::VRWindow: Local OpenGL does not support GLSL shaders");
		
		/* Initialize the required OpenGL extensions: */
		GLARBMultitexture::initExtension();
		
		/* Read the number of view zones and the view zone offset: */
		asNumViewZones=configFileSection.retrieveValue<int>("./autostereoNumViewZones");
		asViewZoneOffset=configFileSection.retrieveValue<Scalar>("./autostereoViewZoneOffset");
		
		/* Get the number of view zone tile columns: */
		asNumTiles[0]=configFileSection.retrieveValue<int>("./autostereoNumTileColumns");
		asNumTiles[1]=(asNumViewZones+asNumTiles[0]-1)/asNumTiles[0];
		
		/* Determine the texture size required to cover the entire screen: */
		WindowPos rootPos=getRootWindowPos();
		for(int i=0;i<2;++i)
			for(asTextureSize[i]=1;asTextureSize[i]<rootPos.size[i];asTextureSize[i]<<=1)
				;
		
		/* Get the name of the view map image: */
		std::string viewMapImageName=configFileSection.retrieveValue<std::string>("./autostereoViewMapImageName");
		
		/* Go to the standard directory if none specified: */
		if(viewMapImageName[0]!='/')
			viewMapImageName=std::string(AUTOSTEREODIRECTORY)+"/"+viewMapImageName;
		
		/* Load the view map: */
		Images::RGBImage viewMap=Images::readImageFile(viewMapImageName.c_str());
		if(int(viewMap.getSize(0))!=rootPos.size[0]||int(viewMap.getSize(1))!=rootPos.size[1])
			Misc::throwStdErr("VRWindow::VRWindow: View map image size does not match display size");
		
		/* Upload the view map texture (pad to power of two size): */
		glGenTextures(1,&asViewMapTextureID);
		glBindTexture(GL_TEXTURE_2D,asViewMapTextureID);
		viewMap.glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,true);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		
		/* Create the view zone texture: */
		glGenTextures(1,&asViewZoneTextureID);
		glBindTexture(GL_TEXTURE_2D,asViewZoneTextureID);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,asTextureSize[0],asTextureSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Check if the local OpenGL supports frame buffer objects: */
		hasFramebufferObjectExtension=GLEXTFramebufferObject::isSupported();
		if(hasFramebufferObjectExtension)
			{
			/* Initialize the extension: */
			GLEXTFramebufferObject::initExtension();
			
			/* Generate a depth buffer object for the view zone rendering pass: */
			glGenRenderbuffersEXT(1,&asDepthBufferObjectID);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,asDepthBufferObjectID);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT,asTextureSize[0],asTextureSize[1]);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
			
			/* Generate a frame buffer object for the view zone rendering pass: */
			glGenFramebuffersEXT(1,&asFrameBufferObjectID);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,asFrameBufferObjectID);
			
			/* Attach the view zone texture and the depth buffer to the frame buffer: */
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,asViewZoneTextureID,0);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,asDepthBufferObjectID);
			
			/* Unbind the frame buffer object for now: */
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
			}
		
		/* Initialize the interzigging shader: */
		asInterzigShader=new GLShader;
		std::string asVertexShaderName=AUTOSTEREODIRECTORY;
		asVertexShaderName+="/InterzigShader.vs";
		asInterzigShader->compileVertexShader(asVertexShaderName.c_str());
		std::string asFragmentShaderName=AUTOSTEREODIRECTORY;
		asFragmentShaderName+="/InterzigShader.fs";
		asInterzigShader->compileFragmentShader(asFragmentShaderName.c_str());
		asInterzigShader->linkShader();
		asQuadSizeUniformIndex=asInterzigShader->getUniformLocation("quadSize");
		if(asQuadSizeUniformIndex<0)
			Misc::throwStdErr("VRWindow::VRWindow: Interzigging shader does not define quadSize variable");
		}
	
	if(showFps)
		{
		/* Load font: */
		showFpsFont=loadFont(configFileSection.retrieveString("./showFpsFontName","HelveticaMediumUpright").c_str());
		GLfloat textHeight=showFpsFont->getTextPixelHeight()-1.0f;
		if(textHeight>16.0f)
			textHeight=16.0f;
		showFpsFont->setTextHeight(textHeight);
		GLFont::Color bg=getBackgroundColor();
		showFpsFont->setBackgroundColor(bg);
		GLFont::Color fg;
		for(int i=0;i<3;++i)
			fg[i]=1.0f-bg[i];
		fg[3]=bg[3];
		showFpsFont->setForegroundColor(fg);
		showFpsFont->setHAlignment(GLFont::Right);
		showFpsFont->setVAlignment(GLFont::Bottom);
		showFpsFont->setAntialiasing(false);
		// showFpsFont->createCharTextures(*contextData);
		}
	
	#ifdef VRWINDOW_USE_SWAPGROUPS
	/* Join a swap group if requested: */
	if(configFileSection.retrieveValue<bool>("./joinSwapGroup",false))
		{
		GLuint maxSwapGroupName,maxSwapBarrierName;
		glXQueryMaxSwapGroupsNV(getDisplay(),getScreen(),&maxSwapGroupName,&maxSwapBarrierName);
		GLuint swapGroupName=configFileSection.retrieveValue<GLuint>("./swapGroupName",0);
		if(swapGroupName>maxSwapGroupName)
			Misc::throwStdErr("VRWindow::VRWindow: Swap group name %u larger than maximum %u",swapGroupName,maxSwapGroupName);
		GLuint swapBarrierName=configFileSection.retrieveValue<GLuint>("./swapBarrierName",0);
		if(swapBarrierName>maxSwapBarrierName)
			Misc::throwStdErr("VRWindow::VRWindow: Swap barrier name %u larger than maximum %u",swapBarrierName,maxSwapBarrierName);
		if(!glXJoinSwapGroupNV(getDisplay(),getWindow(),swapGroupName))
			Misc::throwStdErr("VRWindow::VRWindow: Unable to join swap group %u",swapGroupName);
		if(!glXBindSwapBarrierNV(getDisplay(),swapGroupName,swapBarrierName))
			Misc::throwStdErr("VRWindow::VRWindow: Unable to bind swap barrier %u",swapBarrierName);
		}
	#endif
	
	/* Check if the window is supposed to save a movie: */
	if(configFileSection.retrieveValue<bool>("./saveMovie",false))
		{
		/* Create a movie saver object: */
		movieSaver=MovieSaver::createMovieSaver(configFileSection);
		}
	}

VRWindow::~VRWindow(void)
	{
	delete movieSaver;
	
	makeCurrent();
	if(windowType==INTERLEAVEDVIEWPORT_STEREO)
		{
		if(hasFramebufferObjectExtension)
			{
			glDeleteFramebuffersEXT(1,&ivRightFramebufferObjectID);
			glDeleteRenderbuffersEXT(1,&ivRightDepthbufferObjectID);
			}
		glDeleteTextures(1,&ivRightViewportTextureID);
		for(int i=0;i<4;++i)
			delete[] ivRightStipplePatterns[i];
		}
	else if(windowType==AUTOSTEREOSCOPIC_STEREO)
		{
		delete asInterzigShader;
		if(hasFramebufferObjectExtension)
			{
			glDeleteFramebuffersEXT(1,&asFrameBufferObjectID);
			glDeleteRenderbuffersEXT(1,&asDepthBufferObjectID);
			}
		glDeleteTextures(1,&asViewZoneTextureID);
		glDeleteTextures(1,&asViewMapTextureID);
		}
	delete showFpsFont;
	GLContextData::makeCurrent(0);
	delete contextData;
	GLExtensionManager::makeCurrent(0);
	delete extensionManager;
	}

void VRWindow::setVRScreen(VRScreen* newScreen)
	{
	/* Set both screens to the given screen: */
	screens[0]=screens[1]=newScreen;
	}

void VRWindow::setScreenViewport(const Scalar newViewport[4])
	{
	/* Update both viewports: */
	for(int i=0;i<4;++i)
		viewports[0][i]=viewports[1][i]=newViewport[i];
	}

void VRWindow::setViewer(Viewer* newViewer)
	{
	/* Set the viewer to the given viewer: */
	viewer=newViewer;
	}

int VRWindow::getNumEyes(void) const
	{
	switch(windowType)
		{
		case MONO:
		case LEFT:
		case RIGHT:
			return 1;
		
		case AUTOSTEREOSCOPIC_STEREO:
			return asNumViewZones;
		
		default:
			return 2;
		}
	}

Point VRWindow::getEyePosition(int eyeIndex) const
	{
	switch(windowType)
		{
		case MONO:
			return viewer->getEyePosition(Viewer::MONO);
		
		case LEFT:
			return viewer->getEyePosition(Viewer::LEFT);
		
		case RIGHT:
			return viewer->getEyePosition(Viewer::RIGHT);
		
		case AUTOSTEREOSCOPIC_STEREO:
			{
			Point asEye=viewer->getEyePosition(Viewer::MONO);
			Vector asViewZoneOffsetVector=screens[0]->getScreenTransformation().inverseTransform(Vector(asViewZoneOffset,0,0));
			asEye+=asViewZoneOffsetVector*(Scalar(eyeIndex)-Math::div2(Scalar(asNumViewZones-1)));
			return asEye;
			}
		
		default:
			if(eyeIndex==0)
				return viewer->getEyePosition(Viewer::LEFT);
			else
				return viewer->getEyePosition(Viewer::RIGHT);
		}
	}

Ray VRWindow::reprojectWindowPos(const Scalar windowPos[2]) const
	{
	/* Get the current screen transformation: */
	ONTransform screenT=screens[0]->getScreenTransformation();
	
	/* Transform the eye position to screen coordinates: */
	Point eyePos=viewer->getEyePosition(Viewer::MONO);
	Point screenEyePos=screenT.inverseTransform(eyePos);
	
	/* Check if the screen is projected off-axis: */
	Point nearPoint;
	if(screens[0]->isOffAxis())
		{
		/* Transform the window position from rectified screen coordinates to projected screen coordinates: */
		VRScreen::PTransform2::Point wp(windowPos);
		wp=screens[0]->getScreenHomography().transform(wp);
		
		/* Calculate point on near plane: */
		Scalar near=getFrontplaneDist();
		nearPoint[0]=(wp[0]-screenEyePos[0])/screenEyePos[2]*near+screenEyePos[0];
		nearPoint[1]=(wp[1]-screenEyePos[1])/screenEyePos[2]*near+screenEyePos[1];
		nearPoint[2]=screenEyePos[2]-near;
		}
	else
		{
		/* Calculate point on near plane: */
		Scalar near=getFrontplaneDist();
		nearPoint[0]=(windowPos[0]-screenEyePos[0])/screenEyePos[2]*near+screenEyePos[0];
		nearPoint[1]=(windowPos[1]-screenEyePos[1])/screenEyePos[2]*near+screenEyePos[1];
		nearPoint[2]=screenEyePos[2]-near;
		}
	
	/* Transform near point to world coordinates: */
	nearPoint=screenT.transform(nearPoint);
	
	/* Return result ray: */
	return Ray(nearPoint,nearPoint-eyePos);
	}

ViewSpecification VRWindow::calcViewSpec(int eyeIndex) const
	{
	/* Prepare the result viewing specification: */
	ViewSpecification result;
	
	/* Set the viewport size: */
	result.setViewportSize(getViewportSize());
	
	/* Get the screen's coordinate system: */
	ATransform screenT=screens[eyeIndex]->getScreenTransformation();
	
	/* Calculate helper vectors/points: */
	Scalar l=viewports[eyeIndex][0];
	Scalar r=viewports[eyeIndex][1];
	Scalar b=viewports[eyeIndex][2];
	Scalar t=viewports[eyeIndex][3];
	Scalar lr=Math::mid(l,r);
	Scalar bt=Math::mid(b,t);
	Vector screenX=screenT.getDirection(0);
	Vector screenY=screenT.getDirection(1);
	Vector screenZ=screenT.getDirection(2);
	Point left=screenT.transform(Point(l,bt,0));
	Point right=screenT.transform(Point(r,bt,0));
	Point bottom=screenT.transform(Point(lr,b,0));
	Point top=screenT.transform(Point(lr,t,0));
	
	/* Set the screen plane: */
	result.setScreenPlane(Plane(screenZ,screenT.getOrigin()));
	
	/* Set the screen size: */
	Scalar screenSize[2];
	screenSize[0]=r-l;
	screenSize[1]=t-b;
	result.setScreenSize(screenSize);
	
	/* Get the eye position in physical coordinates: */
	Point eye=getEyePosition(eyeIndex);
	result.setEye(eye);
	
	/* Get the z coordinate of the eye in screen space: */
	Scalar eyeZ=(eye-left)*screenZ;
	result.setEyeScreenDistance(eyeZ);
	
	/* Calculate the six frustum face planes: */
	result.setFrustumPlane(0,Plane(Geometry::cross(screenY,eye-left),left));
	result.setFrustumPlane(1,Plane(Geometry::cross(eye-right,screenY),right));
	result.setFrustumPlane(2,Plane(Geometry::cross(eye-bottom,screenX),bottom));
	result.setFrustumPlane(3,Plane(Geometry::cross(screenX,eye-top),top));
	result.setFrustumPlane(4,Plane(-screenZ,eye-screenZ*getFrontplaneDist()));
	result.setFrustumPlane(5,Plane(screenZ,eye-screenZ*getBackplaneDist()));
	
	/* Calculate the eight frustum corner vertices: */
	Point vertex0=screenT.transform(Point(l,b,0));
	Point vertex1=screenT.transform(Point(r,b,0));
	Point vertex2=screenT.transform(Point(l,t,0));
	Point vertex3=screenT.transform(Point(r,t,0));
	Scalar frontLambda=getFrontplaneDist()/eyeZ;
	result.setFrustumVertex(0,Geometry::affineCombination(eye,vertex0,frontLambda));
	result.setFrustumVertex(1,Geometry::affineCombination(eye,vertex1,frontLambda));
	result.setFrustumVertex(2,Geometry::affineCombination(eye,vertex2,frontLambda));
	result.setFrustumVertex(3,Geometry::affineCombination(eye,vertex3,frontLambda));
	Scalar backLambda=getBackplaneDist()/eyeZ;
	result.setFrustumVertex(4,Geometry::affineCombination(eye,vertex0,backLambda));
	result.setFrustumVertex(5,Geometry::affineCombination(eye,vertex1,backLambda));
	result.setFrustumVertex(6,Geometry::affineCombination(eye,vertex2,backLambda));
	result.setFrustumVertex(7,Geometry::affineCombination(eye,vertex3,backLambda));
	
	return result;
	}

void VRWindow::setCursorPos(const Scalar newCursorPos[2])
	{
	/* Convert from screen coordinates to window coordinates: */
	if(windowType!=SPLITVIEWPORT_STEREO)
		{
		/* Calculate mouse position based on entire window: */
		if(panningViewport)
			{
			windowMousePos[0]=int(Math::floor(newCursorPos[0]*Scalar(displaySize[0])/screens[0]->getWidth()))-getWindowOrigin()[0];
			windowMousePos[1]=displaySize[1]-1-int(Math::floor(newCursorPos[1]*Scalar(displaySize[1])/screens[0]->getHeight()))-getWindowOrigin()[1];
			}
		else
			{
			windowMousePos[0]=int(Math::floor(newCursorPos[0]*Scalar(getWindowWidth())/screens[0]->getWidth()));
			windowMousePos[1]=getWindowHeight()-1-int(Math::floor(newCursorPos[1]*Scalar(getWindowHeight())/screens[0]->getHeight()));
			}
		}
	else
		{
		/* Find out which viewport contains the current mouse position: */
		for(int i=0;i<2;++i)
			{
			int vx=windowMousePos[0]-splitViewportPos[i].origin[0];
			int vy=(getWindowHeight()-1-windowMousePos[1])-splitViewportPos[i].origin[1];
			if(vx>=0&&vx<splitViewportPos[i].size[0]&&vy>=0&&vy<splitViewportPos[i].size[1])
				{
				/* Calculate mouse position based on found viewport: */
				windowMousePos[0]=int(Math::floor(newCursorPos[0]*Scalar(splitViewportPos[i].size[0])/screens[i]->getWidth()))+splitViewportPos[i].origin[0];
				windowMousePos[1]=getWindowHeight()-1-int(Math::floor(newCursorPos[1]*Scalar(splitViewportPos[i].size[1])/screens[i]->getHeight()))-splitViewportPos[i].origin[1];
				
				break;
				}
			}
		}
	
	/* Set the cursor position in the window: */
	GLWindow::setCursorPos(windowMousePos[0],windowMousePos[1]);
	}

void VRWindow::setCursorPosWithAdjust(Scalar newCursorPos[2])
	{
	/* Convert from screen coordinates to window coordinates: */
	if(windowType!=SPLITVIEWPORT_STEREO)
		{
		/* Calculate mouse position based on entire window: */
		if(panningViewport)
			{
			windowMousePos[0]=int(Math::floor(newCursorPos[0]*Scalar(displaySize[0])/screens[0]->getWidth()))-getWindowOrigin()[0];
			windowMousePos[1]=displaySize[1]-int(Math::floor(newCursorPos[1]*Scalar(displaySize[1])/screens[0]->getHeight()))-getWindowOrigin()[1];
			}
		else
			{
			windowMousePos[0]=int(Math::floor(newCursorPos[0]*Scalar(getWindowWidth())/screens[0]->getWidth()));
			windowMousePos[1]=getWindowHeight()-int(Math::floor(newCursorPos[1]*Scalar(getWindowHeight())/screens[0]->getHeight()));
			}
		}
	else
		{
		/* Find out which viewport contains the current mouse position: */
		for(int i=0;i<2;++i)
			{
			int vx=windowMousePos[0]-splitViewportPos[i].origin[0];
			int vy=(getWindowHeight()-1-windowMousePos[1])-splitViewportPos[i].origin[1];
			if(vx>=0&&vx<splitViewportPos[i].size[0]&&vy>=0&&vy<splitViewportPos[i].size[1])
				{
				/* Calculate mouse position based on found viewport: */
				windowMousePos[0]=int(Math::floor(newCursorPos[0]*Scalar(splitViewportPos[i].size[0])/screens[i]->getWidth()))+splitViewportPos[i].origin[0];
				windowMousePos[1]=getWindowHeight()-1-int(Math::floor(newCursorPos[1]*Scalar(splitViewportPos[i].size[1])/screens[i]->getHeight()))-splitViewportPos[i].origin[1];
				
				break;
				}
			}
		}
	
	/* Set the cursor position in the window: */
	GLWindow::setCursorPos(windowMousePos[0],windowMousePos[1]);
	
	/* Adjust the given cursor position: */
	calcMousePos(windowMousePos[0],windowMousePos[1],newCursorPos);
	}

void VRWindow::makeCurrent(void)
	{
	/* Call the base class method: */
	GLWindow::makeCurrent();
	
	/* Install this window's GL extension manager: */
	GLExtensionManager::makeCurrent(extensionManager);
	
	/* Install the window's GL context data manager: */
	GLContextData::makeCurrent(contextData);
	}

bool VRWindow::processEvent(const XEvent& event)
	{
	bool stopProcessing=false;
	
	switch(event.type)
		{
		case Expose:
		case GraphicsExpose:
			/* Window must be redrawn: */
			dirty=true;
			break;
		
		case ConfigureNotify:
			{
			/* Call the base class' event handler: */
			GLWindow::processEvent(event);
			
			if(panningViewport)
				{
				/* Compute the old viewport center and size: */
				Vector translate;
				Scalar oldSize(0);
				for(int i=0;i<2;++i)
					{
					translate[i]=-Math::mid(viewports[0][i*2+0],viewports[0][i*2+1]);
					oldSize+=Math::sqr(viewports[0][i*2+1]-viewports[0][i*2+0]);
					}
				translate[2]=Scalar(0);
				oldSize=Math::sqrt(oldSize);
				
				/* Update the window's viewport: */
				for(int i=0;i<2;++i)
					{
					viewports[i][0]=Scalar(getWindowOrigin()[0])*screens[i]->getWidth()/Scalar(displaySize[0]);
					viewports[i][1]=Scalar(getWindowOrigin()[0]+getWindowWidth())*screens[i]->getWidth()/Scalar(displaySize[0]);
					viewports[i][2]=Scalar(displaySize[1]-getWindowOrigin()[1]-getWindowHeight())*screens[i]->getHeight()/Scalar(displaySize[1]);
					viewports[i][3]=Scalar(displaySize[1]-getWindowOrigin()[1])*screens[i]->getHeight()/Scalar(displaySize[1]);
					}
				
				/* Compute the new viewport center and size: */
				ONTransform screenT=screens[0]->getScreenTransformation();
				Point newCenter;
				Scalar newSize(0);
				for(int i=0;i<2;++i)
					{
					newCenter[i]=Math::mid(viewports[0][i*2+0],viewports[0][i*2+1]);
					translate[i]+=newCenter[i];
					newSize+=Math::sqr(viewports[0][i*2+1]-viewports[0][i*2+0]);
					}
				newCenter[2]=Scalar(0);
				newCenter=screenT.transform(newCenter);
				translate=screenT.transform(translate);
				newSize=Math::sqrt(newSize);
				
				if(navigate)
					{
					/* Try activating a fake navigation tool: */
					if(activateNavigationTool(reinterpret_cast<Tool*>(this)))
						{
						/* Scale to fit the old viewport into the new viewport: */
						NavTransform nav=NavTransform::translateFromOriginTo(newCenter);
						nav*=NavTransform::scale(newSize/oldSize);
						nav*=NavTransform::translateToOriginFrom(newCenter);
						
						/* Translate navigation coordinates to move the display with the window: */
						nav*=NavTransform::translate(translate);
						
						/* Apply the transformation: */
						concatenateNavigationTransformationLeft(nav);
						
						/* Deactivate the fake navigation tool: */
						deactivateNavigationTool(reinterpret_cast<Tool*>(this));
						}
					}
				
				if(movePrimaryWidgets)
					{
					/* Move all primary popped-up widgets with the window: */
					GLMotif::WidgetManager* wm=getWidgetManager();
					GLMotif::WidgetManager::Transformation wt=GLMotif::WidgetManager::Transformation::translate(translate);
					for(GLMotif::WidgetManager::PoppedWidgetIterator wIt=wm->beginPrimaryWidgets();wIt!=wm->endPrimaryWidgets();++wIt)
						wIt.setWidgetToWorld(wt*wIt.getWidgetToWorld());
					}
				
				/* Update the display center and size: */
				setDisplayCenter(newCenter,getDisplaySize()*newSize/oldSize);
				
				requestUpdate();
				}
			
			/* Remember to resize the window's viewport on the next draw() call: */
			resizeViewport=true;
			
			if(trackToolKillZone)
				{
				/* Move the tool kill zone to its intended position in relative window coordinates: */
				ToolKillZone* toolKillZone=getToolManager()->getToolKillZone();
				Vector toolKillZoneSize=screens[0]->getScreenTransformation().inverseTransform(Vector(toolKillZone->getSize()));
				Point screenPos;
				for(int i=0;i<2;++i)
					{
					Scalar min=viewports[0][2*i+0]+toolKillZoneSize[i]*Scalar(0.5);
					Scalar max=viewports[0][2*i+1]-toolKillZoneSize[i]*Scalar(0.5);
					screenPos[i]=min+(max-min)*toolKillZonePos[i];
					}
				screenPos[2]=Scalar(0);
				toolKillZone->setCenter(screens[0]->getScreenTransformation().transform(screenPos));
				vruiState->navigationTransformationChangedMask|=0x4;
				}
			
			if(windowType==INTERLEAVEDVIEWPORT_STEREO)
				{
				/* Reallocate the off-screen buffers: */
				bool mustReallocate=false;
				for(int i=0;i<2;++i)
					{
					int newTextureSize;
					for(newTextureSize=1;newTextureSize<GLWindow::getWindowSize()[i];newTextureSize<<=1)
						;
					ivTexCoord[i]=float(GLWindow::getWindowSize()[i])/float(newTextureSize);
					if(ivTextureSize[i]!=newTextureSize)
						mustReallocate=true;
					ivTextureSize[i]=newTextureSize;
					}
				ivEyeIndexOffset=(GLWindow::getWindowOrigin()[1]%2)*2+(GLWindow::getWindowOrigin()[0]%2);
				
				if(mustReallocate)
					{
					/* Reallocate the right viewport texture: */
					glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
					glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,ivTextureSize[0],ivTextureSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
					glBindTexture(GL_TEXTURE_2D,0);
					
					if(hasFramebufferObjectExtension)
						{
						/* Reallocate the depthbuffer: */
						glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,ivRightDepthbufferObjectID);
						glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT,ivTextureSize[0],ivTextureSize[1]);
						glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
						}
					}
				}
			break;
			}
		
		case MotionNotify:
			if(mouseAdapter!=0)
				{
				/* Remember window-coordinate mouse position: */
				windowMousePos[0]=event.xmotion.x;
				windowMousePos[1]=event.xmotion.y;
				
				/* Set mouse position in input device adapter: */
				Scalar mousePos[2];
				if(calcMousePos(event.xmotion.x,event.xmotion.y,mousePos))
					mouseAdapter->setMousePosition(this,mousePos);
				}
			break;
		
		case ButtonPress:
		case ButtonRelease:
			if(mouseAdapter!=0)
				{
				/* Remember window-coordinate mouse position: */
				windowMousePos[0]=event.xbutton.x;
				windowMousePos[1]=event.xbutton.y;
				
				/* Set mouse position in input device adapter: */
				Scalar mousePos[2];
				if(calcMousePos(event.xbutton.x,event.xbutton.y,mousePos))
					mouseAdapter->setMousePosition(this,mousePos);
				
				/* Set the state of the appropriate button in the input device adapter: */
				bool newState=event.type==ButtonPress;
				if(event.xbutton.button<4)
					stopProcessing=mouseAdapter->setButtonState(event.xbutton.button-1,newState);
				else if(event.xbutton.button==4)
					{
					if(newState)
						mouseAdapter->incMouseWheelTicks();
					}
				else if(event.xbutton.button==5)
					{
					if(newState)
						mouseAdapter->decMouseWheelTicks();
					}
				else if(event.xbutton.button>5)
					stopProcessing=mouseAdapter->setButtonState(event.xbutton.button-3,newState);
				}
			break;
		
		case KeyPress:
		case KeyRelease:
			{
			/* Convert event key index to keysym: */
			char keyString[20];
			KeySym keySym;
			XComposeStatus compose;
			XKeyEvent keyEvent=event.xkey;
			
			/* Use string lookup method to get proper key value for text events: */
			int keyStringLen=XLookupString(&keyEvent,keyString,sizeof(keyString),&keySym,&compose);
			keyString[keyStringLen]='\0';
			
			/* Use keysym lookup a second time to get raw key code ignoring modifier keys: */
			keySym=XLookupKeysym(&keyEvent,0);
			
			if(event.type==KeyPress)
				{
				/* Handle Vrui application keys: */
				switch(keySym)
					{
					case XK_Print:
						if((keyEvent.state&0xedU)==0x40U)
							{
							saveScreenshot=true;
							char numberedFileName[256];
							#if IMAGES_CONFIG_HAVE_PNG
							/* Save the screenshot as a PNG file: */
							screenshotImageFileName=Misc::createNumberedFileName("VruiScreenshot.png",4,numberedFileName);
							#else
							/* Save the screenshot as a PPM file: */
							screenshotImageFileName=Misc::createNumberedFileName("VruiScreenshot.ppm",4,numberedFileName);
							#endif
							
							/* Write a confirmation message: */
							std::cout<<"Saving window contents as "<<screenshotImageFileName<<std::endl;
							}
						break;
					
					case XK_Scroll_Lock:
						if((keyEvent.state&0xedU)==0x40U)
							{
							/* Enter or leave burn mode: */
							if(burnMode)
								{
								double burnModeTime=getApplicationTime()-burnModeStartTime;
								std::cout<<"Leaving burn mode: "<<burnModeNumFrames<<" frames in "<<burnModeTime*1000.0<<" ms, averaging "<<double(burnModeNumFrames)/burnModeTime<<" fps"<<std::endl;
								burnMode=false;
								}
							else
								{
								std::cout<<"Entering burn mode"<<std::endl;
								burnMode=true;
								burnModeNumFrames=~0U;
								}
							}
						break;
					
					case XK_Escape:
						if((keyEvent.state&0xedU)==0x00U)
							{
							/* Call the window close callbacks: */
							Misc::CallbackData cbData;
							getCloseCallbacks().call(&cbData);
							stopProcessing=true;
							}
						break;
					}
				
				if(mouseAdapter!=0)
					{
					/* Send key event to input device adapter: */
					stopProcessing=mouseAdapter->keyPressed(keySym,keyEvent.state,keyString);
					}
				}
			else
				{
				if(mouseAdapter!=0)
					{
					/* Send key event to input device adapter: */
					stopProcessing=mouseAdapter->keyReleased(keySym);
					}
				}
			break;
			}
		
		case EnterNotify:
			if(mouseAdapter!=0)
				{
				/* Create a fake XKeymap event: */
				XKeymapEvent keymapEvent;
				keymapEvent.type=KeymapNotify;
				keymapEvent.serial=event.xcrossing.serial;
				keymapEvent.send_event=event.xcrossing.send_event;
				keymapEvent.display=event.xcrossing.display;
				keymapEvent.window=event.xcrossing.window;
				
				/* Query the current key map: */
				XQueryKeymap(getDisplay(),keymapEvent.key_vector);
				
				/* Reset the input device adapter's key states: */
				mouseAdapter->resetKeys(keymapEvent);
				}
			break;
		
		default:
			/* Call base class method: */
			GLWindow::processEvent(event);
		}
	
	return stopProcessing;
	}

void VRWindow::requestScreenshot(const char* sScreenshotImageFileName)
	{
	/* Set the screenshot flag and remember the given image file name: */
	saveScreenshot=true;
	screenshotImageFileName=sScreenshotImageFileName;
	}

void VRWindow::draw(void)
	{
	/* Activate the window's OpenGL context: */
	makeCurrent();
	
	/* Check if the window's viewport needs to be resized: */
	if(resizeViewport)
		{
		/* Resize the GL viewport: */
		glViewport(0,0,getWindowWidth(),getWindowHeight());
		
		/* Clear the entire window (important if the usual rendering mode masks part of the frame buffer): */
		glDisable(GL_SCISSOR_TEST);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		glDepthMask(GL_TRUE);
		if(clearBufferMask&GL_STENCIL_BUFFER_BIT)
			glStencilMask(~0x0U);
		glClear(clearBufferMask);
		}
	
	/* Update things in the window's GL context data: */
	contextData->updateThings();
	
	/* Draw the window's contents: */
	switch(windowType)
		{
		case MONO:
			/* Render both-eyes view: */
			glDrawBuffer(GL_BACK);
			render(getWindowPos(),0,viewer->getEyePosition(Viewer::MONO));
			break;
		
		case LEFT:
			/* Render left-eye view: */
			glDrawBuffer(GL_BACK);
			render(getWindowPos(),0,viewer->getEyePosition(Viewer::LEFT));
			break;
		
		case RIGHT:
			/* Render right-eye view: */
			glDrawBuffer(GL_BACK);
			render(getWindowPos(),1,viewer->getEyePosition(Viewer::RIGHT));
			break;
		
		case QUADBUFFER_STEREO:
			/* Render left-eye view: */
			glDrawBuffer(GL_BACK_LEFT);
			displayState->eyeIndex=0;
			render(getWindowPos(),0,viewer->getEyePosition(Viewer::LEFT));
			
			/* Render right-eye view: */
			glDrawBuffer(GL_BACK_RIGHT);
			displayState->eyeIndex=1;
			render(getWindowPos(),1,viewer->getEyePosition(Viewer::RIGHT));
			break;
		
		case ANAGLYPHIC_STEREO:
			glDrawBuffer(GL_BACK);
			
			/* Render left-eye view: */
			glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
			displayState->eyeIndex=0;
			render(getWindowPos(),0,viewer->getEyePosition(Viewer::LEFT));
			
			/* Render right-eye view: */
			glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_FALSE);
			displayState->eyeIndex=1;
			render(getWindowPos(),1,viewer->getEyePosition(Viewer::RIGHT));
			break;
		
		case SPLITVIEWPORT_STEREO:
			{
			glDrawBuffer(GL_BACK);
			
			/* Render both views into the split viewport: */
			glEnable(GL_SCISSOR_TEST);
			for(int eye=0;eye<2;++eye)
				{
				glViewport(splitViewportPos[eye].origin[0],splitViewportPos[eye].origin[1],
				           splitViewportPos[eye].size[0],splitViewportPos[eye].size[1]);
				glScissor(splitViewportPos[eye].origin[0],splitViewportPos[eye].origin[1],
				          splitViewportPos[eye].size[0],splitViewportPos[eye].size[1]);
				displayState->eyeIndex=eye;
				render(splitViewportPos[eye],eye,viewer->getEyePosition(eye==0?Viewer::LEFT:Viewer::RIGHT));
				}
			glDisable(GL_SCISSOR_TEST);
			break;
			}
		
		case INTERLEAVEDVIEWPORT_STEREO:
			glDrawBuffer(GL_BACK);
			
			if(hasFramebufferObjectExtension)
				{
				/* Render the left-eye view into the window's default framebuffer: */
				displayState->eyeIndex=0;
				render(getWindowPos(),0,viewer->getEyePosition(Viewer::LEFT));
				
				/* Render the right-eye view into the right viewport framebuffer: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,ivRightFramebufferObjectID);
				displayState->eyeIndex=1;
				render(getWindowPos(),1,viewer->getEyePosition(Viewer::RIGHT));
				
				/* Re-bind the default framebuffer to get access to the right viewport image as a texture: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
				}
			else
				{
				/* Render the right-eye view into the window's default framebuffer: */
				displayState->eyeIndex=1;
				render(getWindowPos(),1,viewer->getEyePosition(Viewer::RIGHT));
				
				/* Copy the rendered view into the viewport texture: */
				glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
				glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,GLWindow::getWindowSize()[0],GLWindow::getWindowSize()[1]);
				glBindTexture(GL_TEXTURE_2D,0);
				
				/* Render the left-eye view into the window's default framebuffer: */
				displayState->eyeIndex=0;
				render(getWindowPos(),0,viewer->getEyePosition(Viewer::LEFT));
				}
			
			/* Set up matrices to render a full-screen quad: */
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glDisable(GL_DEPTH_TEST);
			
			/* Set up polygon stippling: */
			glEnable(GL_POLYGON_STIPPLE);
			
			/* Bind the right viewport texture: */
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,ivRightViewportTextureID);
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			
			/* Set the polygon stippling pattern: */
			glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			glPolygonStipple(ivRightStipplePatterns[ivEyeIndexOffset]);
				
			/* Render the quad: */
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);
			glVertex2f(-1.0f,-1.0f);
			
			glTexCoord2f(ivTexCoord[0],0.0f);
			glVertex2f(1.0f,-1.0f);
			
			glTexCoord2f(ivTexCoord[0],ivTexCoord[1]);
			glVertex2f(1.0f,1.0f);
			
			glTexCoord2f(0.0f,ivTexCoord[1]);
			glVertex2f(-1.0f,1.0f);
			glEnd();
			
			/* Reset OpenGL state: */
			glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_POLYGON_STIPPLE);
			glEnable(GL_DEPTH_TEST);
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			break;
		
		case AUTOSTEREOSCOPIC_STEREO:
			{
			/* Set up the view zone mapping: */
			int asTileSize[2];
			float asTileTexCoord[2];
			int asQuadSize[2];
			for(int i=0;i<2;++i)
				{
				asTileSize[i]=GLWindow::getWindowSize()[i]/asNumTiles[i];
				asTileTexCoord[i]=float(asTileSize[i])/float(asTextureSize[i]);
				asQuadSize[i]=asTileSize[i]*asNumTiles[i];
				}
			
			if(hasFramebufferObjectExtension)
				{
				/* Bind the framebuffer for view zone rendering: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,asFrameBufferObjectID);
				}
			
			/* Calculate the central eye position and the view zone offset vector: */
			Point asEye=viewer->getEyePosition(Viewer::MONO);
			Vector asViewZoneOffsetVector=screens[0]->getScreenTransformation().inverseTransform(Vector(asViewZoneOffset,0,0));
			
			/* Render the view zones: */
			glEnable(GL_SCISSOR_TEST);
			for(int zoneIndex=0;zoneIndex<asNumViewZones;++zoneIndex)
				{
				int row=zoneIndex/asNumTiles[0];
				int col=zoneIndex%asNumTiles[0];
				glViewport(asTileSize[0]*col,asTileSize[1]*row,asTileSize[0],asTileSize[1]);
				glScissor(asTileSize[0]*col,asTileSize[1]*row,asTileSize[0],asTileSize[1]);
				Point eyePos=asEye;
				eyePos+=asViewZoneOffsetVector*(Scalar(zoneIndex)-Math::div2(Scalar(asNumViewZones-1)));
				displayState->eyeIndex=zoneIndex;
				render(GLWindow::WindowPos(asTileSize[0],asTileSize[1]),0,eyePos);
				}
			glDisable(GL_SCISSOR_TEST);
			
			/* Read the view zone image into a texture: */
			if(hasFramebufferObjectExtension)
				{
				/* Unbind the frame buffer to get access to the texture image: */
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
				
				/* Bind the view zone texture to texture unit 0: */
				glActiveTextureARB(GL_TEXTURE0_ARB);
				glBindTexture(GL_TEXTURE_2D,asViewZoneTextureID);
				}
			else
				{
				/* Copy the view zone image from the back buffer into the texture image and bind it to texture 0: */
				glActiveTextureARB(GL_TEXTURE0_ARB);
				glBindTexture(GL_TEXTURE_2D,asViewZoneTextureID);
				glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,GLWindow::getWindowSize()[0],GLWindow::getWindowSize()[1]);
				}
			
			/* Bind the view map image to texture unit 1: */
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D,asViewMapTextureID);
			
			/* Enable the interzigging shader: */
			asInterzigShader->useProgram();
			glUniformARB(asInterzigShader->getUniformLocation("viewZonesTexture"),0);
			glUniformARB(asInterzigShader->getUniformLocation("viewMapTexture"),1);
			glUniformARB<2>(asQuadSizeUniformIndex,1,asTileTexCoord);
			
			/* Set up matrices to render a full-screen quad: */
			glViewport(0,0,asQuadSize[0],asQuadSize[1]);
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0,GLdouble(asQuadSize[0]),0.0,GLdouble(asQuadSize[1]),-1.0,1.0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			
			/* Render the quad: */
			glBegin(GL_QUADS);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0f,0.0f);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.0f,0.0f);
			glVertex2i(0,0);
			
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,asTileTexCoord[0],0.0f);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,asTileTexCoord[0]*3.0f,0.0f);
			glVertex2i(asQuadSize[0],0);
			
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,asTileTexCoord[0],asTileTexCoord[1]);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,asTileTexCoord[0]*3.0f,asTileTexCoord[1]*3.0f);
			glVertex2i(asQuadSize[0],asQuadSize[1]);
			
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,0.0f,asTileTexCoord[1]);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB,0.0f,asTileTexCoord[1]*3.0f);
			glVertex2i(0,asQuadSize[1]);
			glEnd();
			
			/* Reset OpenGL state: */
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			GLShader::disablePrograms();
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D,0);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glBindTexture(GL_TEXTURE_2D,0);
			break;
			}
		}
	
	/* Check for OpenGL errors: */
	glPrintError(std::cerr);
	
	/* Take a screen shot if requested: */
	if(saveScreenshot)
		{
		/* Wait for the OpenGL pipeline to finish: */
		glFinish();
		
		/* Create an RGB image of the same size as the window: */
		Images::RGBImage image(getWindowWidth(),getWindowHeight());
		
		/* Read the window contents into an RGB image: */
		image.glReadPixels(0,0);
		
		/* Save the image buffer to the given image file: */
		Images::writeImageFile(image,screenshotImageFileName.c_str());
		
		#if SAVE_SCREENSHOT_PROJECTION
		
		/* Temporarily load the navigation-space modelview matrix: */
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glMultMatrix(displayState->modelviewNavigational);
		
		/* Query the current projection and modelview matrices: */
		GLdouble proj[16],mv[16];
		glGetDoublev(GL_PROJECTION_MATRIX,proj);
		glGetDoublev(GL_MODELVIEW_MATRIX,mv);
		
		glPopMatrix();
		
		/* Write the matrices to a projection file: */
		{
		IO::AutoFile projFile(Vrui::openFile((screenshotImageFileName+".proj").c_str(),Misc::BufferedFile::WriteOnly));
		projFile->setEndianness(IO::File::LittleEndian);
		projFile->write(proj,16);
		projFile->write(mv,16);
		}
		
		#endif
		
		saveScreenshot=false;
		}
	
	/* Check if the window is supposed to save a movie: */
	if(movieSaver!=0)
		{
		/* Get a fresh frame buffer: */
		MovieSaver::FrameBuffer& frameBuffer=movieSaver->startNewFrame();
		
		/* Update the frame buffer's size and prepare it for writing: */
		frameBuffer.setFrameSize(getWindowWidth(),getWindowHeight());
		frameBuffer.prepareWrite();
		
		/* Wait for the OpenGL pipeline to finish: */
		glFinish();
		
		/* Read the window contents into the movie saver's frame buffer: */
		glPixelStorei(GL_PACK_ALIGNMENT,1);
		glPixelStorei(GL_PACK_SKIP_PIXELS,0);
		glPixelStorei(GL_PACK_ROW_LENGTH,0);
		glPixelStorei(GL_PACK_SKIP_ROWS,0);
		glReadPixels(0,0,getWindowWidth(),getWindowHeight(),GL_RGB,GL_UNSIGNED_BYTE,frameBuffer.getBuffer());
		
		/* Post the new frame: */
		movieSaver->postNewFrame();
		}
	
	/* Window is now up-to-date: */
	resizeViewport=false;
	dirty=false;
	
	if(burnMode)
		{
		if(++burnModeNumFrames==0)
			burnModeStartTime=getApplicationTime();
		
		/* Request another Vrui frame immediately: */
		requestUpdate();
		}
	}

}
