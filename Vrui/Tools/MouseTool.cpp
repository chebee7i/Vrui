/***********************************************************************
MouseTool - Class to map regular 2D mice into VR environments by
representing them as virtual input devices sliding along the screen
planes.
Copyright (c) 2005-2008 Oliver Kreylos

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

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/VRScreen.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>

#include <Vrui/Tools/MouseTool.h>

namespace Vrui {

/*********************************
Methods of class MouseToolFactory:
*********************************/

MouseToolFactory::MouseToolFactory(ToolManager& toolManager)
	:ToolFactory("MouseTool",toolManager),
	 crosshairSize(0),
	 buttonToggleFlag(false)
	{
	/* Insert class into class hierarchy: */
	TransformToolFactory* transformToolFactory=dynamic_cast<TransformToolFactory*>(toolManager.loadClass("TransformTool"));
	transformToolFactory->addChildClass(this);
	addParentClass(transformToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	crosshairSize=cfs.retrieveValue<Scalar>("./crosshairSize",crosshairSize);
	buttonToggleFlag=cfs.retrieveValue<bool>("./buttonToggleFlag",buttonToggleFlag);
	
	/* Initialize tool layout: */
	layout.setNumDevices(1);
	layout.setNumButtons(0,transformToolFactory->getNumButtons());
	layout.setNumValuators(0,transformToolFactory->getNumValuators());
	
	/* Set tool class' factory pointer: */
	MouseTool::factory=this;
	}

MouseToolFactory::~MouseToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	MouseTool::factory=0;
	}

Tool* MouseToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new MouseTool(this,inputAssignment);
	}

void MouseToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveMouseToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("TransformTool");
	}

extern "C" ToolFactory* createMouseToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	MouseToolFactory* mouseToolFactory=new MouseToolFactory(*toolManager);
	
	/* Return factory object: */
	return mouseToolFactory;
	}

extern "C" void destroyMouseToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**********************************
Static elements of class MouseTool:
**********************************/

MouseToolFactory* MouseTool::factory=0;

/**************************
Methods of class MouseTool:
**************************/

MouseTool::MouseTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:TransformTool(factory,inputAssignment)
	{
	}

MouseTool::~MouseTool(void)
	{
	}

void MouseTool::initialize(void)
	{
	/* Initialize the base tool: */
	TransformTool::initialize();
	
	/* Disable the transformed device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	}

const ToolFactory* MouseTool::getFactory(void) const
	{
	return factory;
	}

void MouseTool::frame(void)
	{
	if(transformEnabled)
		{
		/* Calculate the ray equation: */
		Ray ray=getDeviceRay(0);
		
		/* Find the closest intersection with any screen: */
		std::pair<VRScreen*,Scalar> si=findScreen(ray);
		
		if(si.first!=0)
			{
			/* Set the virtual input device's transformation: */
			TrackerState ts=TrackerState::translateFromOriginTo(ray(si.second));
			
			/* Update the virtual input device's transformation: */
			transformedDevice->setTransformation(ts);
			transformedDevice->setDeviceRayDirection(Geometry::normalize(ray.getDirection()));
			}
		}
	else
		{
		InputDevice* device=input.getDevice(0);
		transformedDevice->setTransformation(device->getTransformation());
		transformedDevice->setDeviceRayDirection(device->getDeviceRayDirection());
		}
	}

void MouseTool::display(GLContextData& contextData) const
	{
	if(transformEnabled&&factory->crosshairSize>Scalar(0))
		{
		/* Draw crosshairs at the virtual device's current position: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glPushMatrix();
		glMultMatrix(transformedDevice->getTransformation());
		glLineWidth(3.0f);
		Color lineCol=getBackgroundColor();
		glColor(lineCol);
		glBegin(GL_LINES);
		glVertex(-factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex( factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex(Scalar(0),Scalar(0),-factory->crosshairSize);
		glVertex(Scalar(0),Scalar(0), factory->crosshairSize);
		glEnd();
		glLineWidth(1.0f);
		for(int i=0;i<3;++i)
			lineCol[i]=1.0f-lineCol[i];
		glColor(lineCol);
		glBegin(GL_LINES);
		glVertex(-factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex( factory->crosshairSize,Scalar(0),Scalar(0));
		glVertex(Scalar(0),Scalar(0),-factory->crosshairSize);
		glVertex(Scalar(0),Scalar(0), factory->crosshairSize);
		glEnd();
		glPopMatrix();
		glPopAttrib();
		}
	}

}
