#include <iostream>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <SceneGraph/GLRenderState.h>
#include <SceneGraph/GroupNode.h>
#include <SceneGraph/TransformNode.h>
#include <SceneGraph/AppearanceNode.h>
#include <SceneGraph/MaterialNode.h>
#include <SceneGraph/BoxNode.h>
#include <SceneGraph/ShapeNode.h>
#include <SceneGraph/NodeCreator.h>
#include <SceneGraph/VRMLFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/OpenFile.h>
#include <Vrui/Application.h>

class VruiSceneGraphDemo:public Vrui::Application
	{
	/* Elements: */
	SceneGraph::GroupNodePointer root; // Root of the scene graph
	
	/* Constructors and destructors: */
	public:
	VruiSceneGraphDemo(int& argc,char**& argv,char**& appDefaults);
	
	/* Methods: */
	virtual void display(GLContextData& contextData) const;
	};

/***********************************
Methods of class VruiSceneGraphDemo:
***********************************/

VruiSceneGraphDemo::VruiSceneGraphDemo(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults)
	{
	if(argc>1)
		{
		/*************************************************
		Load scene graphs from one or more VRML 2.0 files:
		*************************************************/
		
		/* Create a node creator to parse the VRML files: */
		SceneGraph::NodeCreator nodeCreator;
		
		/* Create the scene graph's root node: */
		root=new SceneGraph::GroupNode;
		
		/* Load all VRML files from the command line: */
		for(int i=1;i<argc;++i)
			{
			try
				{
				IO::AutoFile inputFile(Vrui::openFile(argv[i]));
				SceneGraph::VRMLFile vrmlFile(argv[i],*inputFile,nodeCreator,Vrui::getMulticastPipeMultiplexer());
				vrmlFile.parse(root);
				}
			catch(std::runtime_error err)
				{
				/* Print an error message and try the next file: */
				std::cerr<<"Ignoring input file "<<argv[i]<<" due to exception "<<err.what()<<std::endl;
				}
			}
		}
	else
		{
		/* Create a small scene graph: */
		SceneGraph::TransformNode* transform=new SceneGraph::TransformNode;
		root=transform;
		
		SceneGraph::ShapeNode* shape=new SceneGraph::ShapeNode;
		root->children.appendValue(shape);
		
		SceneGraph::AppearanceNode* appearance=new SceneGraph::AppearanceNode;
		shape->appearance.setValue(appearance);
		
		SceneGraph::MaterialNode* material=new SceneGraph::MaterialNode;
		appearance->material.setValue(material);
		material->ambientIntensity.setValue(1.0f);
		material->diffuseColor.setValue(SceneGraph::Color(1.0f,0.0f,0.0f));
		material->specularColor.setValue(SceneGraph::Color(1.0f,1.0f,1.0f));
		material->shininess.setValue(0.2f);
		material->update();
		
		appearance->update();
		
		SceneGraph::BoxNode* box=new SceneGraph::BoxNode;
		shape->geometry.setValue(box);
		box->size.setValue(SceneGraph::Size(2,2,2));
		box->update();
		
		shape->update();
		
		root->update();
		}
	
	/* Set the navigation transformation: */
	SceneGraph::Box bbox=root->calcBoundingBox();
	Vrui::setNavigationTransformation(Geometry::mid(bbox.min,bbox.max),Geometry::dist(bbox.min,bbox.max));
	}

void VruiSceneGraphDemo::display(GLContextData& contextData) const
	{
	/* Create a GL render state object: */
	SceneGraph::GLRenderState glRenderState(contextData,Vrui::getHeadPosition(),Vrui::getNavigationTransformation().inverseTransform(Vrui::getUpDirection()));
	
	/* Render the scene graph: */
	root->glRenderAction(glRenderState);
	}

int main(int argc,char* argv[])
	{
	/* Create an application object: */
	char** appDefaults=0;
	VruiSceneGraphDemo app(argc,argv,appDefaults);
	
	/* Run the Vrui main loop: */
	app.run();
	
	/* Exit to OS: */
	return 0;
	}
