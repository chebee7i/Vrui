/***********************************************************************
ArrayKdTree - Class to store k-dimensional points in a kd-tree. Version
for fixed sets of points using index-based storage for added performance
and smaller memory footprint.
Copyright (c) 2003-2005 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef GEOMETRY_ARRAYKDTREE_INCLUDED
#define GEOMETRY_ARRAYKDTREE_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/ClosePointSet.h>

namespace Geometry {

template <class StoredPointParam>
class ArrayKdTree
	{
	/* Embedded classes: */
	public:
	typedef StoredPointParam StoredPoint; // Type of points stored in kd-tree (typically with some associated value)
	typedef typename StoredPoint::Point Point; // Type for positions
	typedef typename Point::Scalar Scalar; // Scalar type used by points
	static const int dimension=Point::dimension; // Dimension of points and kd-tree
	typedef Geometry::ClosePointSet<StoredPoint> ClosePointSet; // Type for nearest neighbours query results
	
	/* Elements: */
	private:
	int numNodes; // Total number of nodes in kd-tree
	StoredPoint* nodes; // Array of nodes
	
	/* Private methods: */
	void createTree(int left,int right,int splitDimension); // Creates sub-kd-tree
	void checkTree(int left,int right,int splitDimension,Scalar bbMin[],Scalar bbMax[]) const; // Checks if kd-tree has correct structure
	template <class TraversalFunctionParam>
	void traverseTree(int left,int right,TraversalFunctionParam& traversalFunction) const // Traverses sub-kd-tree in prefix order and calls traversal function for each node
		{
		/* Call traversal function: */
		int mid=(left+right)>>1;
		traversalFunction(nodes[mid]);
		
		/* Traverse left subtree: */
		if(left<mid)
			traverseTree(left,mid-1,traversalFunction);
		
		/* Traverse right subtree: */
		if(right>mid)
			traverseTree(mid+1,right,traversalFunction);
		}
	template <class TraversalFunctionParam>
	void traverseTreeDirected(int left,int right,int splitDimension,TraversalFunctionParam& traversalFunction) const; // Traverses sub-kd-tree in directed order and calls traversal function for each node
 	void findClosestPoint(int left,int right,int splitDimension,const Point& queryPosition,const StoredPoint*& closestPoint,Scalar& minDist2) const; // Recursively finds closest point in kd-tree
	void findClosestPoints(int left,int right,int splitDimension,const Point& queryPosition,ClosePointSet& closestPoints) const; // Recursively finds closest points in kd-tree
	
	/* Constructors and destructors: */
	public:
	ArrayKdTree(void) // Creates empty kd-tree
		:numNodes(0),nodes(0)
		{
		}
	ArrayKdTree(int sNumNodes) // Creates kd-tree for numNodes points, without initializing the point data
		:numNodes(sNumNodes),nodes(new StoredPoint[numNodes])
		{
		}
	ArrayKdTree(int sNumNodes,const StoredPoint sNodes[]); // Creates balanced kd-tree from point array
	~ArrayKdTree(void)
		{
		delete[] nodes;
		}
	
	/* Methods: */
	StoredPoint* createTree(int newNumNodes); // Creates a tree for the given number of points and returns a pointer to point array
	int getNumNodes(void) const // Returns the number of nodes in the tree
		{
		return numNodes;
		}
	const StoredPoint* accessPoints(void) const // Returns pointer to point array for one-by-one inspection
		{
		return nodes;
		}
	StoredPoint* accessPoints(void) // Returns pointer to point array for one-by-one updates
		{
		return nodes;
		}
	void releasePoints(void) // Releases access to point array; creates balanced kd-tree
		{
		/* Create new tree: */
		createTree(0,numNodes-1,0);
		}
	void setPoints(int newNumNodes,const StoredPoint newNodes[]); // Creates balanced kd-tree from point array
	void donatePoints(int newNumNodes,StoredPoint* newNodes); // Creates balanced kd-tree from point array; adopts point array as own
	const StoredPoint& getNode(int nodeIndex) const // Returns one of the octree's nodes
		{
		return nodes[nodeIndex];
		}
	void checkTree(void) const; // Checks the tree for consistency
	template <class TraversalFunctionParam>
	void traverseTree(TraversalFunctionParam& traversalFunction) const // Traverses tree in prefix order and calls traversal function for each node
		{
		traverseTree(0,numNodes-1,traversalFunction);
		}
	template <class TraversalFunctionParam>
	void traverseTreeDirected(TraversalFunctionParam& traversalFunction) const // Traverses tree in directed order and calls traversal function for each node
		{
		traverseTreeDirected(0,numNodes-1,0,traversalFunction);
		}
	const StoredPoint& findClosePoint(const Point& queryPosition) const; // Returns a stored point that is close to the query position
	const StoredPoint& findClosestPoint(const Point& queryPosition) const; // Returns the stored point closest to the query position
	ClosePointSet& findClosestPoints(const Point& queryPosition,ClosePointSet& closestPoints) const; // Returns a set of closest points
	};

}

/***********************************************************************
Creating standard versions of ArrayKdTree does not make much sense. We
abuse the well-established non-standard template mechanism to create
required instantiations on-the-fly.
***********************************************************************/

#if !defined(GEOMETRY_ARRAYKDTREE_IMPLEMENTATION)
#include <Geometry/ArrayKdTree.cpp>
#endif

#endif
