/***********************************************************************
AffinePointTransformNode - Point transformation class to transform
points by arbitrary affine transformations.
Copyright (c) 2011-2013 Oliver Kreylos

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

#ifndef SCENEGRAPH_AFFINEPOINTTRANSFORMNODE_INCLUDED
#define SCENEGRAPH_AFFINEPOINTTRANSFORMNODE_INCLUDED

#include <Geometry/AffineTransformation.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/PointTransformNode.h>

namespace SceneGraph {

class AffinePointTransformNode:public PointTransformNode
	{
	/* Embedded classes: */
	private:
	typedef Geometry::AffineTransformation<TScalar,3> ATransform; // Type for affine transformations
	
	/* Elements: */
	
	/* Fields: */
	public:
	MFTScalar matrix;
	
	/* Derived state: */
	protected:
	ATransform transform; // A transformation created from the row-major matrix
	ATransform inverseTransform; // The inverse transformation
	ATransform normalTransform; // The associated normal vector transformation
	
	/* Constructors and destructors: */
	public:
	AffinePointTransformNode(void); // Creates a default node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from PointTransformNode: */
	virtual TPoint transformPoint(const TPoint& point) const;
	virtual TPoint inverseTransformPoint(const TPoint& point) const;
	virtual TBox calcBoundingBox(const std::vector<Point>& points) const;
	virtual TBox transformBox(const TBox& box) const;
	virtual TVector transformNormal(const TPoint& basePoint,const TVector& normal) const;
	};

}

#endif
