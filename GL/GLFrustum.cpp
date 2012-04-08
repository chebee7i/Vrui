/***********************************************************************
GLFrustum - Class describing an OpenGL view frustum in model coordinates
to perform software-based frustum culling and LOD calculation.
Copyright (c) 2007 Oliver Kreylos

This file is part of the OpenGL Wrapper Library for the Templatized
Geometry Library (GLGeometry).

The OpenGL Wrapper Library for the Templatized Geometry Library is free
software; you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

The OpenGL Wrapper Library for the Templatized Geometry Library is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Wrapper Library for the Templatized Geometry Library; if
not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite
330, Boston, MA 02111-1307 USA
***********************************************************************/

#define GLFRUSTUM_IMPLEMENTATION

#ifndef METHODPREFIX
	#ifdef NONSTANDARD_TEMPLATES
		#define METHODPREFIX inline
	#else
		#define METHODPREFIX
	#endif
#endif

#include <Math/Math.h>
#include <Geometry/HVector.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>

#include <GL/GLFrustum.h>

/**************************
Methods of class GLFrustum:
**************************/

template <class ScalarParam>
METHODPREFIX
void
GLFrustum<ScalarParam>::setScreenEye(
	const typename GLFrustum<ScalarParam>::Plane& newScreenPlane,
	const typename GLFrustum<ScalarParam>::HVector& newEye)
	{
	/* Set the screen plane and the eye point: */
	screenPlane=newScreenPlane;
	eye=newEye;
	
	/* Calculate the inverse eye-screen distance: */
	eyeScreenDist=eye[3]/(screenPlane.getNormal()[0]*eye[0]+screenPlane.getNormal()[1]*eye[1]+screenPlane.getNormal()[2]*eye[2]-screenPlane.getOffset()*eye[3]);
	}

template <class ScalarParam>
METHODPREFIX
void
GLFrustum<ScalarParam>::setFromGL(
	void)
	{
	typedef Geometry::ProjectiveTransformation<double,3> PTransform;
	
	/* Read projection and modelview matrices from OpenGL: */
	PTransform pmv=glGetProjectionMatrix<double>();
	pmv*=glGetModelviewMatrix<double>();
	pmv.doInvert();
	
	/* Calculate the eight frustum vertices: */
	frustumVertices[0]=Point(pmv.transform(Geometry::Point<double,3>(-1,-1,-1)));
	frustumVertices[1]=Point(pmv.transform(Geometry::Point<double,3>( 1,-1,-1)));
	frustumVertices[2]=Point(pmv.transform(Geometry::Point<double,3>(-1, 1,-1)));
	frustumVertices[3]=Point(pmv.transform(Geometry::Point<double,3>( 1, 1,-1)));
	frustumVertices[4]=Point(pmv.transform(Geometry::Point<double,3>(-1,-1, 1)));
	frustumVertices[5]=Point(pmv.transform(Geometry::Point<double,3>( 1,-1, 1)));
	frustumVertices[6]=Point(pmv.transform(Geometry::Point<double,3>(-1, 1, 1)));
	frustumVertices[7]=Point(pmv.transform(Geometry::Point<double,3>( 1, 1, 1)));
	
	/* Calculate the six frustum face planes: */
	Vector fv10=frustumVertices[1]-frustumVertices[0];
	Vector fv20=frustumVertices[2]-frustumVertices[0];
	Vector fv40=frustumVertices[4]-frustumVertices[0];
	Vector fv67=frustumVertices[6]-frustumVertices[7];
	Vector fv57=frustumVertices[5]-frustumVertices[7];
	Vector fv37=frustumVertices[3]-frustumVertices[7];
	frustumPlanes[0]=Plane(Geometry::cross(fv40,fv20),frustumVertices[0]);
	frustumPlanes[1]=Plane(Geometry::cross(fv57,fv37),frustumVertices[7]);
	frustumPlanes[2]=Plane(Geometry::cross(fv10,fv40),frustumVertices[0]);
	frustumPlanes[3]=Plane(Geometry::cross(fv37,fv67),frustumVertices[7]);
	frustumPlanes[4]=Plane(Geometry::cross(fv20,fv10),frustumVertices[0]);
	Scalar screenArea=Geometry::mag(frustumPlanes[4].getNormal());
	frustumPlanes[5]=Plane(Geometry::cross(fv67,fv57),frustumVertices[7]);
	for(int i=0;i<6;++i)
		frustumPlanes[i].normalize();
	
	/* Use the frustum near plane as the screen plane: */
	screenPlane=frustumPlanes[4];
	
	/* Calculate the eye point: */
	eye=HVector(pmv.transform(Geometry::HVector<double,3>(0,0,1,0)));
	
	/* Calculate the inverse eye-screen distance: */
	eyeScreenDist=eye[3]/(screenPlane.getNormal()[0]*eye[0]+screenPlane.getNormal()[1]*eye[1]+screenPlane.getNormal()[2]*eye[2]-screenPlane.getOffset()*eye[3]);
	
	/* Get viewport size from OpenGL: */
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	
	/* Calculate the inverse pixel size: */
	pixelSize=Math::sqrt((Scalar(viewport[2])*Scalar(viewport[3]))/screenArea);
	}

#if !defined(NONSTANDARD_TEMPLATES)

/*****************************************************
Force instantiation of all standard GLFrustum classes:
*****************************************************/

template class GLFrustum<GLfloat>;
template class GLFrustum<GLdouble>;

#endif
