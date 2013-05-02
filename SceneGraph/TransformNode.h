/***********************************************************************
TransformNode - Class for group nodes that apply an orthogonal
transformation to their children.
Copyright (c) 2009-2013 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_TRANSFORMNODE_INCLUDED
#define SCENEGRAPH_TRANSFORMNODE_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GroupNode.h>

namespace SceneGraph {

class TransformNode:public GroupNode
	{
	/* Elements: */
	
	/* Fields: */
	public:
	SFPoint center;
	SFRotation rotation;
	SFSize scale;
	SFRotation scaleOrientation;
	SFVector translation;
	
	/* Derived state: */
	protected:
	OGTransform transform; // The current transformation
	
	/* Constructors and destructors: */
	public:
	TransformNode(void); // Creates an empty transform node with an identity transformation
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual EventOut* getEventOut(const char* fieldName) const;
	virtual EventIn* getEventIn(const char* fieldName);
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GraphNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* New methods: */
	const OGTransform& getTransform(void) const // Returns the current derived transformation
		{
		return transform;
		}
	};

typedef Misc::Autopointer<TransformNode> TransformNodePointer;

}

#endif
