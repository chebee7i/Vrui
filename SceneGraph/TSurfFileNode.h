/***********************************************************************
TSurfFileNode - Class for triangle meshes read from GoCAD TSurf files.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef SCENEGRAPH_TSURFFILENODE_INCLUDED
#define SCENEGRAPH_TSURFFILENODE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLGeometryVertex.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GeometryNode.h>

namespace SceneGraph {

class TSurfFileNode:public GeometryNode,public GLObject
	{
	/* Embedded classes: */
	protected:
	typedef GLGeometry::Vertex<void,0,void,0,float,float,3> Vertex; // Type for mesh vertices
	typedef unsigned int Card; // Type for indices
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferObjectId; // ID of vertex buffer object containing the vertices, if supported
		GLuint indexBufferObjectId; // ID of index buffer object containing the vertex indices, if supported
		unsigned int version; // Version number of triangle mesh in buffers
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Fields: */
	public:
	MFString url; //  Name of the TSurf input file
	
	/* Derived elements: */
	protected:
	std::vector<Vertex> vertices; // List of mesh vertices
	std::vector<Card> indices; // List of mesh vertex indices
	unsigned int version; // Version number of triangle mesh
	
	/* Constructors and destructors: */
	public:
	TSurfFileNode(void); // Creates a default line set
	
	/* Methods from Node: */
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GeometryNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

}

#endif
