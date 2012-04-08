########################################################################
# Makefile for Vrui toolkit and required basic libraries.
# Copyright (c) 1998-2008 Oliver Kreylos
#
# This file is part of the WhyTools Build Environment.
# 
# The WhyTools Build Environment is free software; you can redistribute
# it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# The WhyTools Build Environment is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the WhyTools Build Environment; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
########################################################################

# Define the root of the toolkit source tree
PACKAGEROOT := $(shell pwd)

# Include definitions for the system environment
include $(PACKAGEROOT)/BuildRoot/SystemDefinitions
include $(PACKAGEROOT)/BuildRoot/Packages

########################################################################
# Some settings that might need adjustment
# The settings below are conservative; see the README file for what they
# mean, and how they depend on capabilities of the host system.
########################################################################

# Root directory for the software installation
# Note: This must a different directory than the one that contains this
# makefile!
INSTALLDIR = $(HOME)/Vrui-1.0

# Set this to 1 if Vrui executables and shared libraries shall contain
# links to any shared libraries they link to. This will relieve a user
# from having to set up a dynamic library search path.
USE_RPATH = 1

# Set the following flag to 1 if the GL support library shall contain
# support for multithreaded rendering to several windows. This is
# required for shared-memory multi-pipe rendering systems such as SGI
# Onyx or Prism, but somewhat reduces performance when accessing per-
# window data elements or OpenGL extensions. If the compiler and run-
# time environment do not support a direct mechanism for thread-local
# storage (such as gcc's __thread extension), this uses the even slower
# POSIX thread-local storage mechanism. If setting this to 1 causes
# compilation errors, the SYSTEM_HAVE_TLS variable in
# BuildRoot/SystemDefinitions needs to be set to 0.
GLSUPPORT_USE_TLS = 0

# Set the following two flags to 1 if the image handling library shall
# contain support for reading/writing PNG and JPEG images, respectively.
# This requires that libpng and libjpeg are installed on the host
# computer, and that the paths to their respective header files /
# libraries are set properly in $(PACKAGEROOT)/BuildRoot/Packages
# For now, the following code tries to automatically determine whether
# PNG and/or JPEG are supported. This might or might not work.
ifneq ($(strip $(PNG_BASEDIR)),)
  IMAGES_USE_PNG = 1
else
  IMAGES_USE_PNG = 0
endif
ifneq ($(strip $(JPEG_BASEDIR)),)
  IMAGES_USE_JPEG = 1
else
  IMAGES_USE_JPEG = 0
endif

# Set this to 1 if Vrui shall contain support for saving screenshots in
# PNG (Portable Network Graphics) format. If this is set to 0, Vrui will
# save screenshots in binary (raw) PPM format instead. PNG support
# requires that the image handling library is compiled with PNG support
# as well (see the respective setting for further requirements). The
# default setting is to use PNG image files if the Images library is
# built with PNG support.
VRUI_USE_PNG = $(IMAGES_USE_PNG)

# Set this to 1 if the VRWindow class shall be compiled with support for
# swap locks and swap groups (NVidia extension). If this is set to 1 and
# the NVidia extension is not there, VRWindow.cpp will generate compiler
# errors.
VRWINDOW_CPP_USE_SWAPGROUPS = 0

# Set this to 1 if Vrui shall be able to record sound while capturing a
# session using an input device data saver, and be able to play back
# sound while playing back a recorded session. Under Linux, this
# requires that the ALSA sound library development packages are
# installed on the host computer, and that the path to the ALSA header
# files / libraries is set properly in
# $(PACKAGEROOT)/BuildRoot/Packages.
# For now, the following code tries to automatically determine whether
# ALSA is supported. This might or might not work.
ifeq ($(SYSTEM),DARWIN)
  VRUI_USE_SOUND = 1
else
  ifneq ($(strip $(ALSA_BASEDIR)),)
    VRUI_USE_SOUND = 1
  else
    VRUI_USE_SOUND = 0
  endif
endif

# Set this to 1 if Vrui shall contain support for spatial audio using
# the OpenAL API. This requires that OpenAL is installed on the host
# computer, and that the path to the OpenAL header files / libraries is
# set properly in $(PACKAGEROOT)/BuildRoot/Packages.
# For now, the following code tries to automatically determine whether
# OpenAL is supported. This might or might not work.
ifneq ($(strip $(OPENAL_BASEDIR)),)
  VRUI_USE_OPENAL = 1
else
  VRUI_USE_OPENAL = 0
endif

# Set this to 1 if the operating system supports the input abstraction
# layer. If this is set to 1 and the input abstraction is not supported,
# Joystick.cpp will generate compiler errors.
VRDEVICES_USE_INPUT_ABSTRACTION = 0

# Set this to 1 if the Linux input.h header file has the required
# structure definitions (usually on newer Linux versions). If this is
# set wrongly, HIDDevice.cpp will generate compiler errors.
# This setting is ignored on MacOS X.
HIDDEVICE_CPP_INPUT_H_HAS_STRUCTS = 1

# Set this to 1 if the Vrui VR device driver shall support Nintendo Wii
# controllers using the bluez user-level Bluetooth library. This
# requires that bluez is installed on the host computer, and that the
# path to the bluez header files / libraries is set properly in
# $(PACKAGEROOT)/BuildRoot/Packages.
# For now, the following code tries to automatically determine whether
# bluez is supported. This might or might not work.
ifneq ($(strip $(BLUETOOTH_BASEDIR)),)
  VRDEVICES_USE_BLUETOOTH = 1
else
  VRDEVICES_USE_BLUETOOTH = 0
endif

########################################################################
# Everything below here should not have to be changed
########################################################################

# Specify default optimization/debug level
ifdef DEBUG
  DEFAULTDEBUGLEVEL = 3
  DEFAULTOPTLEVEL = 0
else
  DEFAULTDEBUGLEVEL = 0
  DEFAULTOPTLEVEL = 3
endif

# Set destination directory for libraries
ifdef DEBUG
  LIBDESTDIR = $(PACKAGEROOT)/$(LIBEXT)/debug
else
  LIBDESTDIR = $(PACKAGEROOT)/$(LIBEXT)
endif

# Specify version of created dynamic shared libraries
MAJORLIBVERSION = 1
MINORLIBVERSION = 1

# Directories for installation components
HEADERINSTALLDIR = $(INSTALLDIR)/$(INCLUDEEXT)
ifdef DEBUG
  LIBINSTALLDIR = $(INSTALLDIR)/$(LIBEXT)/debug
  EXECUTABLEINSTALLDIR = $(INSTALLDIR)/$(BINEXT)/debug
  PLUGININSTALLDIR = $(INSTALLDIR)/$(LIBEXT)/debug
else
  LIBINSTALLDIR = $(INSTALLDIR)/$(LIBEXT)
  EXECUTABLEINSTALLDIR = $(INSTALLDIR)/$(BINEXT)
  PLUGININSTALLDIR = $(INSTALLDIR)/$(LIBEXT)
endif
ETCINSTALLDIR = $(INSTALLDIR)/etc
SHAREINSTALLDIR = $(INSTALLDIR)/share

########################################################################
# Specify additional compiler and linker flags
########################################################################

# Set flags to distinguish between static and shared libraries
ifdef STATIC_LINK
  LIBRARYNAME = $(LIBDESTDIR)/$(1).$(LDEXT).a
  OBJDIREXT = Static
else
  CFLAGS += $(CDSOFLAGS)
  LIBRARYNAME=$(LIBDESTDIR)/$(call FULLDSONAME,$(1))
endif

########################################################################
# List packages used by this project
# (Supported packages can be found in ./BuildRoot/Packages)
########################################################################

PACKAGES = 

########################################################################
# Specify all final targets
########################################################################

LIBRARIES = 
PLUGINS = 
EXECUTABLES = 

#
# The basic libraries:
#

LIBRARY_NAMES = libMisc \
                libPlugins \
                libRealtime \
                libComm \
                libMath \
                libGeometry \
                libGLWrappers \
                libGLSupport \
                libGLXSupport \
                libGLGeometry \
                libGLMotif \
                libImages \
                libSound \
                libALSupport \
                libVrui

LIBRARIES += $(LIBRARY_NAMES:%=$(call LIBRARYNAME,%))

#
# The Vrui VR tool plug-in hierarchy:
#

VRTOOLSDIR = $(LIBDESTDIR)/VRTools

VRTOOLS_SOURCES = Vrui/Tools/SixDofLocatorTool.cpp \
                  Vrui/Tools/ScreenLocatorTool.cpp \
                  Vrui/Tools/WaldoLocatorTool.cpp \
                  Vrui/Tools/SixDofDraggingTool.cpp \
                  Vrui/Tools/WaldoDraggingTool.cpp \
                  Vrui/Tools/SixDofNavigationTool.cpp \
                  Vrui/Tools/SixDofWithScaleNavigationTool.cpp \
                  Vrui/Tools/TrackballNavigationTool.cpp \
                  Vrui/Tools/FlyNavigationTool.cpp \
                  Vrui/Tools/ValuatorFlyNavigationTool.cpp \
                  Vrui/Tools/ValuatorTurnNavigationTool.cpp \
                  Vrui/Tools/ValuatorFlyTurnNavigationTool.cpp \
                  Vrui/Tools/WandNavigationTool.cpp \
                  Vrui/Tools/WalkNavigationTool.cpp \
                  Vrui/Tools/ValuatorScalingNavigationTool.cpp \
                  Vrui/Tools/TwoHandedNavigationTool.cpp \
                  Vrui/Tools/MultiDeviceNavigationTool.cpp \
                  Vrui/Tools/MouseNavigationTool.cpp \
                  Vrui/Tools/MouseDialogNavigationTool.cpp \
                  Vrui/Tools/FPSNavigationTool.cpp \
                  Vrui/Tools/HelicopterNavigationTool.cpp \
                  Vrui/Tools/DesktopDeviceNavigationTool.cpp \
                  Vrui/Tools/ViewpointFileNavigationTool.cpp \
                  Vrui/Tools/ComeHitherNavigationTool.cpp \
                  Vrui/Tools/MouseTool.cpp \
                  Vrui/Tools/EyeRayTool.cpp \
                  Vrui/Tools/OffsetTool.cpp \
                  Vrui/Tools/WaldoTool.cpp \
                  Vrui/Tools/ClutchTool.cpp \
                  Vrui/Tools/RevolverTool.cpp \
                  Vrui/Tools/RayMenuTool.cpp \
                  Vrui/Tools/RayScreenMenuTool.cpp \
                  Vrui/Tools/PanelMenuTool.cpp \
                  Vrui/Tools/ToolManagementTool.cpp \
                  Vrui/Tools/InputDeviceTool.cpp \
                  Vrui/Tools/SixDofInputDeviceTool.cpp \
                  Vrui/Tools/RayInputDeviceTool.cpp \
                  Vrui/Tools/ButtonInputDeviceTool.cpp \
                  Vrui/Tools/WidgetTool.cpp \
                  Vrui/Tools/SpaceBallTool.cpp \
                  Vrui/Tools/LaserpointerTool.cpp \
                  Vrui/Tools/JediTool.cpp \
                  Vrui/Tools/FlashlightTool.cpp \
                  Vrui/Tools/MeasurementTool.cpp \
                  Vrui/Tools/ScreenshotTool.cpp \
                  Vrui/Tools/ViewpointSaverTool.cpp \
                  Vrui/Tools/CurveEditorTool.cpp

VRTOOLS = $(VRTOOLS_SOURCES:Vrui/Tools/%.cpp=$(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT))
PLUGINS += $(VRTOOLS)

#
# The Vrui vislet plug-in hierarchy:
#

VRVISLETSDIR = $(LIBDESTDIR)/Vislets

VRVISLETS_SOURCES = Vrui/Vislets/CAVERenderer.cpp

VRVISLETS = $(VRVISLETS_SOURCES:Vrui/Vislets/%.cpp=$(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT))
PLUGINS += $(VRVISLETS)

#
# The Vrui device driver test program:
#

EXECUTABLES += $(EXEDIR)/DeviceTest

#
# The input device data file dumping program:
#

EXECUTABLES += $(EXEDIR)/PrintInputDeviceDataFile

#
# The VR device driver daemon:
#

EXECUTABLES += $(EXEDIR)/VRDeviceDaemon

#
# The VR device driver plug-ins:
#

VRDEVICESDIR = $(LIBDESTDIR)/VRDevices

VRDEVICES_SOURCES = VRDeviceDaemon/AscensionFlockOfBirds.cpp \
                    VRDeviceDaemon/PolhemusFastrak.cpp \
                    VRDeviceDaemon/InterSense.cpp \
                    VRDeviceDaemon/FakespacePinchGlove.cpp \
                    VRDeviceDaemon/ArtDTrack.cpp \
                    VRDeviceDaemon/ViconTarsus.cpp \
                    VRDeviceDaemon/ViconTarsusRaw.cpp \
                    VRDeviceDaemon/PCTracker.cpp \
                    VRDeviceDaemon/PCWand.cpp \
                    VRDeviceDaemon/MouseButtons.cpp \
                    VRDeviceDaemon/SpaceBallRaw.cpp \
                    VRDeviceDaemon/SpaceBall.cpp \
                    VRDeviceDaemon/HIDDevice.cpp \
                    VRDeviceDaemon/VRPNClient.cpp \
                    VRDeviceDaemon/RemoteDevice.cpp \
                    VRDeviceDaemon/DummyDevice.cpp

ifneq ($(VRDEVICES_USE_INPUT_ABSTRACTION),0)
  VRDEVICES_SOURCES += VRDeviceDaemon/Joystick.cpp
endif

ifneq ($(VRDEVICES_USE_BLUETOOTH),0)
  VRDEVICES_SOURCES += VRDeviceDaemon/WiimoteTracker.cpp
endif

VRDEVICES = $(VRDEVICES_SOURCES:VRDeviceDaemon/%.cpp=$(VRDEVICESDIR)/lib%.$(PLUGINFILEEXT))
PLUGINS += $(VRDEVICES)

#
# The VR tracker calibrator plug-ins:
#

VRCALIBRATORSDIR = $(LIBDESTDIR)/VRCalibrators

VRCALIBRATORS_SOURCES = VRDeviceDaemon/TransformCalibrator.cpp \
                        VRDeviceDaemon/GridCalibrator.cpp

VRCALIBRATORS = $(VRCALIBRATORS_SOURCES:VRDeviceDaemon/%.cpp=$(VRCALIBRATORSDIR)/lib%.$(PLUGINFILEEXT))
PLUGINS += $(VRCALIBRATORS)

# Set the name of the makefile fragment:
ifdef DEBUG
  MAKEFILEFRAGMENT = Share/Vrui.debug.makeinclude
else
  MAKEFILEFRAGMENT = Share/Vrui.makeinclude
endif

ALL = config $(LIBRARIES) $(EXECUTABLES) $(PLUGINS) $(MAKEFILEFRAGMENT)

.PHONY: all
all: $(ALL)

########################################################################
# Pseudo-target to print configuration options
########################################################################

.PHONY: config
config:
	@echo "---- Configured Vrui options: ----"
	@echo "Installation directory: $(INSTALLDIR)"
ifneq ($(GLSUPPORT_USE_TLS),0)
	@echo "Multithreaded rendering enabled"
else
	@echo "Multithreaded rendering disabled"
endif
ifneq ($(IMAGES_USE_PNG),0)
	@echo "PNG image file format enabled"
else
	@echo "PNG image file format disabled"
endif
ifneq ($(IMAGES_USE_JPEG),0)
	@echo "JPG image file format enabled"
else
	@echo "JPG image file format disabled"
endif
ifneq ($(VRUI_USE_PNG),0)
	@echo "Screenshots will be saved in PNG format"
else
	@echo "Screenshots will be saved in PPM format"
endif
ifneq ($(VRUI_USE_SOUND),0)
	@echo "Sound recording and playback enabled"
else
	@echo "Sound recording and playback disabled"
endif
ifneq ($(VRUI_USE_OPENAL),0)
	@echo "Spatial sound using OpenAL library enabled"
else
	@echo "Spatial sound using OpenAL library disabled"
endif
ifneq ($(VRDEVICES_USE_BLUETOOTH),0)
	@echo "Bluetooth support (for Nintendo Wii controller) enabled"
else
	@echo "Bluetooth support (for Nintendo Wii controller) disabled"
endif
	@echo "--------"

########################################################################
# Specify other actions to be performed on a `make clean'
########################################################################

.PHONY: extraclean
extraclean:
	-rm -f $(LIBRARY_NAMES:%=$(LIBDESTDIR)/$(call DSONAME,%))
	-rm -f $(LIBRARY_NAMES:%=$(LIBDESTDIR)/$(call LINKDSONAME,%))

.PHONY: extrasqueakyclean
extrasqueakyclean:
	-rm -f $(ALL)
	-rm -rf $(PACKAGEROOT)/$(LIBEXT)
	-rm -f Share/Vrui.makeinclude Share/Vrui.debug.makeinclude

# Include basic makefile
include $(PACKAGEROOT)/BuildRoot/BasicMakefile

########################################################################
# Specify build rules for dynamic shared objects
########################################################################

#
# Function to get the build dependencies of a package
#

DEPENDENCIES = $(patsubst -l%.$(LDEXT),$(call LIBRARYNAME,lib%),$(foreach PACKAGENAME,$(filter MY%,$($(1)_DEPENDS)),$($(PACKAGENAME)_LIBS)))

#
# The Miscellaneous Support Library (Misc)
#

MISC_HEADERS = Misc/Utility.h \
               Misc/SelfDestructPointer.h \
               Misc/RefCounted.h \
               Misc/Autopointer.h \
               Misc/ArrayIndex.h \
               Misc/Array.h \
               Misc/ChunkedArray.h \
               Misc/ChunkedQueue.h \
               Misc/PriorityHeap.h \
               Misc/PoolAllocator.h \
               Misc/StandardHashFunction.h \
               Misc/OrderedTuple.h \
               Misc/UnorderedTuple.h \
               Misc/HashTable.h \
               Misc/OneTimeQueue.h \
               Misc/ThrowStdErr.h \
               Misc/Time.h \
               Misc/Timer.h \
               Misc/CallbackData.h \
               Misc/CallbackList.h \
               Misc/TimerEventScheduler.h \
               Misc/Endianness.h \
               Misc/File.h \
               Misc/LargeFile.h \
               Misc/MemMappedFile.h \
               Misc/StringMarshaller.h \
               Misc/CreateNumberedFileName.h \
               Misc/ValueCoder.h \
               Misc/StandardValueCoders.h \
               Misc/ArrayValueCoders.h Misc/ArrayValueCoders.cpp \
               Misc/CompoundValueCoders.h Misc/CompoundValueCoders.cpp \
               Misc/ConfigurationFile.h Misc/ConfigurationFile.icpp \
               Misc/FileLocator.h

MISC_SOURCES = Misc/ThrowStdErr.cpp \
               Misc/Timer.cpp \
               Misc/CallbackList.cpp \
               Misc/TimerEventScheduler.cpp \
               Misc/CreateNumberedFileName.cpp \
               Misc/ValueCoder.cpp \
               Misc/StandardValueCoders.cpp \
               Misc/ArrayValueCoders.cpp \
               Misc/CompoundValueCoders.cpp \
               Misc/ConfigurationFile.cpp \
               Misc/FileLocator.cpp

$(call LIBRARYNAME,libMisc): PACKAGES += $(MYMISC_DEPENDS)
$(call LIBRARYNAME,libMisc): EXTRACINCLUDEFLAGS += $(MYMISC_INCLUDE)
$(call LIBRARYNAME,libMisc): $(call DEPENDENCIES,MYMISC)
$(call LIBRARYNAME,libMisc): $(MISC_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libMisc
libMisc: $(call LIBRARYNAME,libMisc)

#
# The Plugin Handling Library (Plugins):
#

PLUGINS_HEADERS = Plugins/Factory.h \
                  Plugins/FactoryManager.h Plugins/FactoryManager.cpp

PLUGINS_SOURCES = Plugins/Factory.cpp \
                  Plugins/FactoryManager.cpp

$(call LIBRARYNAME,libPlugins): PACKAGES += $(MYPLUGINS_DEPENDS)
$(call LIBRARYNAME,libPlugins): EXTRACINCLUDEFLAGS += $(MYPLUGINS_INCLUDE)
$(call LIBRARYNAME,libPlugins): $(call DEPENDENCIES,MYPLUGINS)
$(call LIBRARYNAME,libPlugins): $(PLUGINS_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libPlugins
libPlugins: $(call LIBRARYNAME,libPlugins)

#
# The Realtime Processing Library (Realtime):
#

REALTIME_HEADERS = Realtime/AlarmTimer.h

REALTIME_SOURCES = Realtime/AlarmTimer.cpp

$(call LIBRARYNAME,libRealtime): PACKAGES += $(MYREALTIME_DEPENDS)
$(call LIBRARYNAME,libRealtime): EXTRACINCLUDEFLAGS += $(MYREALTIME_INCLUDE)
$(call LIBRARYNAME,libRealtime): CFLAGS += $(MYREALTIME_CFLAGS)
$(call LIBRARYNAME,libRealtime): $(call DEPENDENCIES,MYREALTIME)
$(call LIBRARYNAME,libRealtime): $(REALTIME_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libRealtime
libRealtime: $(call LIBRARYNAME,libRealtime)

#
# The Portable Threading Library (Threads)
#

THREADS_HEADERS = Threads/Thread.h \
                  Threads/Mutex.h \
                  Threads/Cond.h \
                  Threads/MutexCond.h \
                  Threads/Barrier.h \
                  Threads/Local.h \
                  Threads/RefCounted.h \
                  Threads/TripleBuffer.h

#
# The Portable Communications Library (Comm)
#

COMM_HEADERS = Comm/FdSet.h \
               Comm/SerialPort.h \
               Comm/UDPSocket.h \
               Comm/TCPSocket.h \
               Comm/TCPPipe.h \
               Comm/MulticastPacket.h \
               Comm/MulticastPipe.h \
               Comm/MulticastPipeMultiplexer.h \
               Comm/ClusterPipe.h

COMM_SOURCES = Comm/FdSet.cpp \
               Comm/SerialPort.cpp \
               Comm/UDPSocket.cpp \
               Comm/TCPSocket.cpp \
               Comm/TCPPipe.cpp \
               Comm/MulticastPipe.cpp \
               Comm/MulticastPipeMultiplexer.cpp \
               Comm/ClusterPipe.cpp

$(call LIBRARYNAME,libComm): PACKAGES += $(MYCOMM_DEPENDS)
$(call LIBRARYNAME,libComm): EXTRACINCLUDEFLAGS += $(MYCOMM_INCLUDE)
$(call LIBRARYNAME,libComm): $(call DEPENDENCIES,MYCOMM)
$(call LIBRARYNAME,libComm): $(COMM_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libComm
libComm: $(call LIBRARYNAME,libComm)

#
# The Templatized Math Library (Math)
#

MATH_HEADERS = Math/Math.h \
               Math/Constants.h \
               Math/Interval.h Math/Interval.cpp \
               Math/Random.h

MATH_SOURCES = Math/Constants.cpp \
               Math/Interval.cpp \
               Math/Random.cpp

$(call LIBRARYNAME,libMath): PACKAGES += $(MYMATH_DEPENDS)
$(call LIBRARYNAME,libMath): EXTRACINCLUDEFLAGS += $(MYMATH_INCLUDE)
$(call LIBRARYNAME,libMath): $(call DEPENDENCIES,MYMATH)
$(call LIBRARYNAME,libMath): $(MATH_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libMath
libMath: $(call LIBRARYNAME,libMath)

#
# The Templatized Geometry Library (Geometry)
#

GEOMETRY_HEADERS = Geometry/ComponentArray.h Geometry/ComponentArray.cpp \
                   Geometry/Vector.h Geometry/Vector.cpp \
                   Geometry/Point.h Geometry/Point.cpp \
                   Geometry/AffineCombiner.h \
                   Geometry/HVector.h Geometry/HVector.cpp \
                   Geometry/MatrixHelperFunctions.h \
                   Geometry/Matrix.h Geometry/Matrix.cpp \
                   Geometry/Rotation.h Geometry/Rotation.cpp \
                   Geometry/TranslationTransformation.h Geometry/TranslationTransformation.cpp \
                   Geometry/RotationTransformation.h Geometry/RotationTransformation.cpp \
                   Geometry/OrthonormalTransformation.h Geometry/OrthonormalTransformation.cpp \
                   Geometry/UniformScalingTransformation.h Geometry/UniformScalingTransformation.cpp \
                   Geometry/OrthogonalTransformation.h Geometry/OrthogonalTransformation.cpp \
                   Geometry/ScalingTransformation.h Geometry/ScalingTransformation.cpp \
                   Geometry/AffineTransformation.h Geometry/AffineTransformation.cpp\
                   Geometry/ProjectiveTransformation.h Geometry/ProjectiveTransformation.cpp \
                   Geometry/Ray.h \
                   Geometry/HitResult.h \
                   Geometry/SolidHitResult.h \
                   Geometry/Plane.h \
                   Geometry/Sphere.h \
                   Geometry/Cylinder.h \
                   Geometry/Cone.h \
                   Geometry/Box.h Geometry/Box.cpp \
                   Geometry/Polygon.h Geometry/Polygon.cpp \
                   Geometry/SplineCurve.h Geometry/SplineCurve.cpp \
                   Geometry/SplinePatch.h Geometry/SplinePatch.cpp \
                   Geometry/ValuedPoint.h \
                   Geometry/ClosePointSet.h \
                   Geometry/PointOctree.h Geometry/PointOctree.cpp \
                   Geometry/PointTwoNTree.h Geometry/PointTwoNTree.cpp \
                   Geometry/PointKdTree.h Geometry/PointKdTree.cpp \
                   Geometry/ArrayKdTree.h Geometry/ArrayKdTree.cpp \
                   Geometry/PolygonMesh.h Geometry/PolygonMesh.cpp \
                   Geometry/Random.h Geometry/Random.cpp \
                   Geometry/Endianness.h \
                   Geometry/GeometryValueCoders.h Geometry/GeometryValueCoders.cpp

GEOMETRY_SOURCES = Geometry/ComponentArray.cpp \
                   Geometry/Vector.cpp \
                   Geometry/Point.cpp \
                   Geometry/HVector.cpp \
                   Geometry/Matrix.cpp \
                   Geometry/Rotation.cpp \
                   Geometry/TranslationTransformation.cpp \
                   Geometry/RotationTransformation.cpp \
                   Geometry/OrthonormalTransformation.cpp \
                   Geometry/UniformScalingTransformation.cpp \
                   Geometry/OrthogonalTransformation.cpp \
                   Geometry/ScalingTransformation.cpp \
                   Geometry/AffineTransformation.cpp \
                   Geometry/ProjectiveTransformation.cpp \
                   Geometry/Box.cpp \
                   Geometry/Polygon.cpp \
                   Geometry/SplineCurve.cpp \
                   Geometry/SplinePatch.cpp \
                   Geometry/PointOctree.cpp \
                   Geometry/PointTwoNTree.cpp \
                   Geometry/PointKdTree.cpp \
                   Geometry/ArrayKdTree.cpp \
                   Geometry/PolygonMesh.cpp \
                   Geometry/Random.cpp \
                   Geometry/GeometryValueCoders.cpp

$(call LIBRARYNAME,libGeometry): PACKAGES += $(MYGEOMETRY_DEPENDS)
$(call LIBRARYNAME,libGeometry): EXTRACINCLUDEFLAGS += $(MYGEOMETRY_INCLUDE)
$(call LIBRARYNAME,libGeometry): $(call DEPENDENCIES,MYGEOMETRY)
$(call LIBRARYNAME,libGeometry): $(GEOMETRY_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libGeometry
libGeometry: $(call LIBRARYNAME,libGeometry)

#
# The OpenGL C++ Wrapper Library (GLWrappers)
#

GLWRAPPERS_HEADERS = GL/GLTexCoordTemplates.h \
                     GL/GLMultiTexCoordTemplates.h \
                     GL/GLColorTemplates.h \
                     GL/GLSecondaryColorTemplates.h \
                     GL/GLNormalTemplates.h \
                     GL/GLFogCoordTemplates.h \
                     GL/GLVertexTemplates.h \
                     GL/GLIndexTemplates.h \
                     GL/GLPrimitiveTemplates.h \
                     GL/GLScalarLimits.h \
                     GL/GLScalarConverter.h \
                     GL/GLColor.h \
                     GL/GLColorOperations.h \
                     GL/GLVector.h \
                     GL/GLBox.h \
                     GL/GLVertexArrayTemplates.h \
                     GL/GLVertexArrayParts.h \
                     GL/GLVertex.h GL/GLVertex.cpp \
                     GL/GLFogEnums.h \
                     GL/GLFogTemplates.h \
                     GL/GLFog.h \
                     GL/GLLightEnums.h \
                     GL/GLLightTemplates.h \
                     GL/GLLight.h \
                     GL/GLLightModelEnums.h \
                     GL/GLLightModelTemplates.h \
                     GL/GLMaterialEnums.h \
                     GL/GLMaterialTemplates.h \
                     GL/GLMaterial.h \
                     GL/GLTexEnvEnums.h \
                     GL/GLTexEnvTemplates.h \
                     GL/GLMatrixEnums.h \
                     GL/GLMatrixTemplates.h \
                     GL/GLMiscTemplates.h \
                     GL/GLGetTemplates.h \
                     GL/GLGetPrimitiveTemplates.h \
                     GL/GLGetFogTemplates.h \
                     GL/GLGetLightTemplates.h \
                     GL/GLGetMaterialTemplates.h \
                     GL/GLGetTexEnvTemplates.h \
                     GL/GLGetMatrixTemplates.h \
                     GL/GLGetMiscTemplates.h

GLWRAPPERS_SOURCES = GL/GLScalarLimits.cpp \
                     GL/GLColor.cpp \
                     GL/GLVertex.cpp \
                     GL/GLFog.cpp \
                     GL/GLLight.cpp \
                     GL/GLMaterial.cpp

$(call LIBRARYNAME,libGLWrappers): PACKAGES += $(MYGLWRAPPERS_DEPENDS)
$(call LIBRARYNAME,libGLWrappers): EXTRACINCLUDEFLAGS += $(MYGLWRAPPERS_INCLUDE)
$(call LIBRARYNAME,libGLWrappers): CFLAGS += $(MYGLWRAPPERS_CFLAGS)
$(call LIBRARYNAME,libGLWrappers): $(call DEPENDENCIES,MYGLWRAPPERS)
$(call LIBRARYNAME,libGLWrappers): $(GLWRAPPERS_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libGLWrappers
libGLWrappers: $(call LIBRARYNAME,libGLWrappers)

#
# The OpenGL Support Library (GLSupport)
#

GLSUPPORT_HEADERS = GL/GLValueCoders.h \
                    GL/TLSHelper.h \
                    GL/GLObject.h \
                    GL/GLContextData.h \
                    GL/GLExtensions.h \
                    GL/GLExtensionManager.h \
                    GL/GLShader.h \
                    GL/GLColorMap.h \
                    GL/GLFont.h \
                    GL/GLLineIlluminator.h \
                    GL/GLModels.h

GLSUPPORTEXTENSION_HEADERS = GL/Extensions/GLExtension.h \
                             GL/Extensions/GLARBDepthTexture.h \
                             GL/Extensions/GLARBFragmentProgram.h \
                             GL/Extensions/GLARBFragmentShader.h \
                             GL/Extensions/GLARBMultitexture.h \
                             GL/Extensions/GLARBPointParameters.h \
                             GL/Extensions/GLARBPointSprite.h \
                             GL/Extensions/GLARBShaderObjects.h \
                             GL/Extensions/GLARBShadow.h \
                             GL/Extensions/GLARBTextureCompression.h \
                             GL/Extensions/GLARBTextureNonPowerOfTwo.h \
                             GL/Extensions/GLARBVertexBufferObject.h \
                             GL/Extensions/GLARBVertexProgram.h \
                             GL/Extensions/GLARBVertexShader.h \
                             GL/Extensions/GLEXTFramebufferObject.h \
                             GL/Extensions/GLEXTPalettedTexture.h \
                             GL/Extensions/GLEXTTexture3D.h \
                             GL/Extensions/GLEXTTextureCompressionS3TC.h \
                             GL/Extensions/GLEXTTextureCubeMap.h \
                             GL/Extensions/GLNVFogDistance.h \
                             GL/Extensions/GLNVOcclusionQuery.h \
                             GL/Extensions/GLNVPointSprite.h \
                             GL/Extensions/GLNVTextureShader.h

GLSUPPORT_SOURCES = GL/GLValueCoders.cpp \
                    GL/GLObject.cpp \
                    GL/GLThingManager.cpp \
                    GL/GLContextData.cpp \
                    GL/GLExtensions.cpp \
                    GL/GLExtensionManager.cpp \
                    GL/Extensions/GLARBDepthTexture.cpp \
                    GL/Extensions/GLARBFragmentProgram.cpp \
                    GL/Extensions/GLARBFragmentShader.cpp \
                    GL/Extensions/GLARBMultitexture.cpp \
                    GL/Extensions/GLARBPointParameters.cpp \
                    GL/Extensions/GLARBPointSprite.cpp \
                    GL/Extensions/GLARBShaderObjects.cpp \
                    GL/Extensions/GLARBShadow.cpp \
                    GL/Extensions/GLARBTextureCompression.cpp \
                    GL/Extensions/GLARBTextureNonPowerOfTwo.cpp \
                    GL/Extensions/GLARBVertexBufferObject.cpp \
                    GL/Extensions/GLARBVertexProgram.cpp \
                    GL/Extensions/GLARBVertexShader.cpp \
                    GL/Extensions/GLEXTFramebufferObject.cpp \
                    GL/Extensions/GLEXTPalettedTexture.cpp \
                    GL/Extensions/GLEXTTexture3D.cpp \
                    GL/Extensions/GLEXTTextureCompressionS3TC.cpp \
                    GL/Extensions/GLEXTTextureCubeMap.cpp \
                    GL/Extensions/GLNVFogDistance.cpp \
                    GL/Extensions/GLNVOcclusionQuery.cpp \
                    GL/Extensions/GLNVPointSprite.cpp \
                    GL/Extensions/GLNVTextureShader.cpp \
                    GL/GLShader.cpp \
                    GL/GLColorMap.cpp \
                    GL/GLFont.cpp \
                    GL/GLLineIlluminator.cpp \
                    GL/GLModels.cpp

ifneq ($(SYSTEM_HAVE_GLXGETPROCADDRESS), 0)
  $(OBJDIR)/GL/GLExtensionManager.o: CFLAGS += -DHAVE_GLXGETPROCADDRESS
endif
$(OBJDIR)/GL/GLFont.o: CFLAGS += -DSYSGLFONTDIR='"$(SHAREINSTALLDIR)/GLFonts"'

$(call LIBRARYNAME,libGLSupport): PACKAGES += $(MYGLSUPPORT_DEPENDS)
$(call LIBRARYNAME,libGLSupport): EXTRACINCLUDEFLAGS += $(MYGLSUPPORT_INCLUDE)
$(call LIBRARYNAME,libGLSupport): CFLAGS += $(MYGLSUPPORT_CFLAGS)
$(call LIBRARYNAME,libGLSupport): $(call DEPENDENCIES,MYGLSUPPORT)
$(call LIBRARYNAME,libGLSupport): $(GLSUPPORT_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libGLSupport
libGLSupport: $(call LIBRARYNAME,libGLSupport)

#
# The OpenGL/GLX Support Library (GLXSupport)
#

GLXSUPPORT_HEADERS = GL/GLWindow.h

GLXSUPPORT_SOURCES = GL/GLWindow.cpp

$(call LIBRARYNAME,libGLXSupport): PACKAGES += $(MYGLXSUPPORT_DEPENDS)
$(call LIBRARYNAME,libGLXSupport): EXTRACINCLUDEFLAGS += $(MYGLXSUPPORT_INCLUDE)
$(call LIBRARYNAME,libGLXSupport): CFLAGS += $(MYGLXSUPPORT_CFLAGS)
$(call LIBRARYNAME,libGLXSupport): $(call DEPENDENCIES,MYGLXSUPPORT)
$(call LIBRARYNAME,libGLXSupport): $(GLXSUPPORT_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libGLXSupport
libGLXSupport: $(call LIBRARYNAME,libGLXSupport)

#
# The OpenGL Wrapper Library for the Templatized Geometry Library (GLGeometry)
#

GLGEOMETRY_HEADERS = GL/GLGeometryWrappers.h \
                     GL/GLTransformationWrappers.h GL/GLTransformationWrappers.cpp \
                     GL/GLFrustum.h \
                     GL/GLPolylineTube.h

GLGEOMETRY_SOURCES = GL/GLTransformationWrappers.cpp \
                     GL/GLFrustum.cpp \
                     GL/GLPolylineTube.cpp

$(call LIBRARYNAME,libGLGeometry): PACKAGES += $(MYGLGEOMETRY_DEPENDS)
$(call LIBRARYNAME,libGLGeometry): EXTRACINCLUDEFLAGS += $(MYGLGEOMETRY_INCLUDE)
$(call LIBRARYNAME,libGLGeometry): $(call DEPENDENCIES,MYGLGEOMETRY)
$(call LIBRARYNAME,libGLGeometry): $(GLGEOMETRY_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libGLGeometry
libGLGeometry: $(call LIBRARYNAME,libGLGeometry)

#
# The GLMotif 3D User Interface Component Library (GLMotif)
#

GLMOTIF_HEADERS = GLMotif/Types.h \
                  GLMotif/Alignment.h \
                  GLMotif/Event.h \
                  GLMotif/StyleSheet.h \
                  GLMotif/Widget.h \
                  GLMotif/WidgetManager.h \
                  GLMotif/Container.h \
                  GLMotif/SingleChildContainer.h \
                  GLMotif/Margin.h \
                  GLMotif/Popup.h \
                  GLMotif/Blind.h \
                  GLMotif/Separator.h \
                  GLMotif/Label.h \
                  GLMotif/TextField.h \
                  GLMotif/Button.h \
                  GLMotif/DecoratedButton.h \
                  GLMotif/Arrow.h \
                  GLMotif/ToggleButton.h \
                  GLMotif/CascadeButton.h \
                  GLMotif/DragWidget.h \
                  GLMotif/Slider.h \
                  GLMotif/ScrollBar.h \
                  GLMotif/ListBox.h \
                  GLMotif/ScrolledListBox.h \
                  GLMotif/RowColumn.h \
                  GLMotif/RadioBox.h \
                  GLMotif/DropdownBox.h \
                  GLMotif/Menu.h \
                  GLMotif/SubMenu.h \
                  GLMotif/PopupMenu.h \
                  GLMotif/TitleBar.h \
                  GLMotif/PopupWindow.h \
                  GLMotif/FileSelectionDialog.h \
                  GLMotif/WidgetAlgorithms.h GLMotif/WidgetAlgorithms.cpp

GLMOTIF_SOURCES = GLMotif/Event.cpp \
                  GLMotif/StyleSheet.cpp \
                  GLMotif/Widget.cpp \
                  GLMotif/WidgetManager.cpp \
                  GLMotif/Container.cpp \
                  GLMotif/SingleChildContainer.cpp \
                  GLMotif/Margin.cpp \
                  GLMotif/Popup.cpp \
                  GLMotif/Blind.cpp \
                  GLMotif/Separator.cpp \
                  GLMotif/Label.cpp \
                  GLMotif/TextField.cpp \
                  GLMotif/Button.cpp \
                  GLMotif/DecoratedButton.cpp \
                  GLMotif/Arrow.cpp \
                  GLMotif/ToggleButton.cpp \
                  GLMotif/CascadeButton.cpp \
                  GLMotif/DragWidget.cpp \
                  GLMotif/Slider.cpp \
                  GLMotif/ScrollBar.cpp \
                  GLMotif/ListBox.cpp \
                  GLMotif/ScrolledListBox.cpp \
                  GLMotif/RowColumn.cpp \
                  GLMotif/RadioBox.cpp \
                  GLMotif/DropdownBox.cpp \
                  GLMotif/Menu.cpp \
                  GLMotif/SubMenu.cpp \
                  GLMotif/PopupMenu.cpp \
                  GLMotif/TitleBar.cpp \
                  GLMotif/PopupWindow.cpp \
                  GLMotif/FileSelectionDialog.cpp \
                  GLMotif/WidgetAlgorithms.cpp

$(call LIBRARYNAME,libGLMotif): PACKAGES += $(MYGLMOTIF_DEPENDS)
$(call LIBRARYNAME,libGLMotif): EXTRACINCLUDEFLAGS += $(MYGLMOTIF_INCLUDE)
$(call LIBRARYNAME,libGLMotif): $(call DEPENDENCIES,MYGLMOTIF)
$(call LIBRARYNAME,libGLMotif): $(GLMOTIF_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libGLMotif
libGLMotif: $(call LIBRARYNAME,libGLMotif)

#
# The Image Handling Library (Images)
#

IMAGES_HEADERS = Images/Image.h \
                 Images/RGBImage.h \
                 Images/RGBAImage.h \
                 Images/GetImageFileSize.h \
                 Images/ReadImageFile.h \
                 Images/WriteImageFile.h

IMAGES_SOURCES = Images/Image.cpp \
                 Images/GetImageFileSize.cpp \
                 Images/ReadImageFile.cpp \
                 Images/WriteImageFile.cpp

$(call LIBRARYNAME,libImages): PACKAGES += $(MYIMAGES_DEPENDS)
$(call LIBRARYNAME,libImages): EXTRACINCLUDEFLAGS += $(MYIMAGES_INCLUDE)
ifneq ($(IMAGES_USE_PNG),0)
  $(call LIBRARYNAME,libImages): CFLAGS += -DIMAGES_USE_PNG
endif
ifneq ($(IMAGES_USE_JPEG),0)
  $(call LIBRARYNAME,libImages): CFLAGS += -DIMAGES_USE_JPEG
endif
$(call LIBRARYNAME,libImages): $(call DEPENDENCIES,MYIMAGES)
$(call LIBRARYNAME,libImages): $(IMAGES_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libImages
libImages: $(call LIBRARYNAME,libImages)

#
# The basic sound library (Sound)
#

SOUND_HEADERS = Sound/SoundDataFormat.h \
                Sound/SoundRecorder.h \
                Sound/SoundPlayer.h

SOUND_SOURCES = Sound/SoundDataFormat.cpp \
                Sound/SoundRecorder.cpp \
                Sound/SoundPlayer.cpp


$(call LIBRARYNAME,libSound): PACKAGES += $(MYSOUND_DEPENDS)
$(call LIBRARYNAME,libSound): EXTRACINCLUDEFLAGS += $(MYSOUND_INCLUDE)
ifeq ($(SYSTEM),LINUX)
  ifneq ($(VRUI_USE_SOUND),0)
    $(call LIBRARYNAME,libSound): CFLAGS += -DSOUND_USE_ALSA
  endif
endif
$(call LIBRARYNAME,libSound): $(call DEPENDENCIES,MYSOUND)
$(call LIBRARYNAME,libSound): $(SOUND_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libSound
libSound: $(call LIBRARYNAME,libSound)

#
# The OpenAL Support Library (ALSupport)
#

ALSUPPORT_HEADERS = AL/ALObject.h \
                    AL/ALContextData.h

ALSUPPORT_SOURCES = AL/ALObject.cpp \
                    AL/ALThingManager.cpp \
                    AL/ALContextData.cpp

$(call LIBRARYNAME,libALSupport): PACKAGES += $(MYALSUPPORT_DEPENDS)
$(call LIBRARYNAME,libALSupport): EXTRACINCLUDEFLAGS += $(MYALSUPPORT_INCLUDE)
$(call LIBRARYNAME,libALSupport): $(call DEPENDENCIES,MYALSUPPORT)
$(call LIBRARYNAME,libALSupport): $(ALSUPPORT_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libALSupport
libALSupport: $(call LIBRARYNAME,libALSupport)

#
# The Vrui Virtual Reality User Interface Library (Vrui)
#

VRUI_HEADERS = Vrui/Geometry.h \
               Vrui/TransparentObject.h \
               Vrui/GlyphRenderer.h \
               Vrui/InputDevice.h \
               Vrui/VirtualInputDevice.h \
               Vrui/InputGraphManager.h \
               Vrui/InputDeviceManager.h \
               Vrui/VRDeviceState.h \
               Vrui/VRDevicePipe.h \
               Vrui/VRDeviceClient.h \
               Vrui/MutexMenu.h \
               Vrui/Lightsource.h \
               Vrui/LightsourceManager.h \
               Vrui/CoordinateTransform.h \
               Vrui/OrthogonalCoordinateTransform.h \
               Vrui/GeodeticCoordinateTransform.h \
               Vrui/CoordinateManager.h \
               Vrui/Viewer.h \
               Vrui/VRScreen.h \
               Vrui/ViewSpecification.h \
               Vrui/VRWindow.h \
               Vrui/Listener.h \
               Vrui/SoundContext.h \
               Vrui/ToolInputLayout.h \
               Vrui/ToolInputAssignment.h \
               Vrui/LocatorToolAdapter.h \
               Vrui/DraggingToolAdapter.h \
               Vrui/ToolManager.h \
               Vrui/Vislet.h \
               Vrui/VisletManager.h \
               Vrui/Vrui.h \
               Vrui/ClusterSupport.h \
               Vrui/Application.h

VRUI_TOOLHEADERS = Vrui/Tools/Tool.h \
                   Vrui/Tools/GenericToolFactory.h \
                   Vrui/Tools/LocatorTool.h \
                   Vrui/Tools/DraggingTool.h \
                   Vrui/Tools/NavigationTool.h \
                   Vrui/Tools/TransformTool.h \
                   Vrui/Tools/UserInterfaceTool.h \
                   Vrui/Tools/MenuTool.h \
                   Vrui/Tools/UtilityTool.h

VRUI_SOURCES = Vrui/TransparentObject.cpp \
               Vrui/BoxRayDragger.cpp \
               Vrui/GlyphRenderer.cpp \
               Vrui/InputDevice.cpp \
               Vrui/VirtualInputDevice.cpp \
               Vrui/InputGraphManager.cpp \
               Vrui/InputDeviceManager.cpp \
               Vrui/InputDeviceAdapter.cpp \
               Vrui/InputDeviceAdapterMouse.cpp \
               Vrui/InputDeviceAdapterIndexMap.cpp \
               Vrui/VRDeviceClient.cpp \
               Vrui/InputDeviceAdapterDeviceDaemon.cpp \
               Vrui/InputDeviceAdapterVisBox.cpp \
               Vrui/InputDeviceAdapterPlayback.cpp \
               Vrui/MultipipeDispatcher.cpp \
               Vrui/MutexMenu.cpp \
               Vrui/Lightsource.cpp \
               Vrui/LightsourceManager.cpp \
               Vrui/CoordinateTransform.cpp \
               Vrui/OrthogonalCoordinateTransform.cpp \
               Vrui/GeodeticCoordinateTransform.cpp \
               Vrui/CoordinateManager.cpp \
               Vrui/Viewer.cpp \
               Vrui/VRScreen.cpp \
               Vrui/ViewSpecification.cpp \
               Vrui/VRWindow.cpp \
               Vrui/Listener.cpp \
               Vrui/SoundContext.cpp \
               Vrui/ToolInputLayout.cpp \
               Vrui/ToolInputAssignment.cpp \
               Vrui/Tools/Tool.cpp \
               Vrui/Tools/LocatorTool.cpp \
               Vrui/Tools/DraggingTool.cpp \
               Vrui/Tools/NavigationTool.cpp \
               Vrui/Tools/TransformTool.cpp \
               Vrui/Tools/UserInterfaceTool.cpp \
               Vrui/Tools/MenuTool.cpp \
               Vrui/Tools/UtilityTool.cpp \
               Vrui/LocatorToolAdapter.cpp \
               Vrui/DraggingToolAdapter.cpp \
               Vrui/ToolKillZone.cpp \
               Vrui/ToolKillZoneBox.cpp \
               Vrui/ToolKillZoneFrustum.cpp \
               Vrui/ToolManager.cpp \
               Vrui/Vislet.cpp \
               Vrui/VisletManager.cpp \
               Vrui/InputDeviceDataSaver.cpp \
               Vrui/Vrui.General.cpp \
               Vrui/Vrui.Workbench.cpp \
               Vrui/Application.cpp

$(OBJDIR)/Vrui/ToolManager.o: CFLAGS += -DSYSTOOLDSONAMETEMPLATE='"$(PLUGININSTALLDIR)/VRTools/lib%s.$(PLUGINFILEEXT)"'
$(OBJDIR)/Vrui/VisletManager.o: CFLAGS += -DSYSVISLETDSONAMETEMPLATE='"$(PLUGININSTALLDIR)/VRVislets/lib%s.$(PLUGINFILEEXT)"'
$(OBJDIR)/Vrui/VRWindow.o: CFLAGS += -DAUTOSTEREODIRECTORY='"$(SHAREINSTALLDIR)/Textures"'
ifneq ($(VRWINDOW_CPP_USE_SWAPGROUPS),0)
  $(OBJDIR)/Vrui/VRWindow.o: CFLAGS += -DVRWINDOW_USE_SWAPGROUPS
endif
$(OBJDIR)/Vrui/Vrui.Workbench.o: CFLAGS += -DSYSVRUICONFIGFILE='"$(ETCINSTALLDIR)/Vrui.cfg"' -DVRUIDEFAULTROOTSECTIONNAME='"Desktop"'

$(call LIBRARYNAME,libVrui): PACKAGES += $(MYVRUI_DEPENDS)
$(call LIBRARYNAME,libVrui): EXTRACINCLUDEFLAGS += $(MYVRUI_INCLUDE)
ifneq ($(VRUI_USE_OPENAL),0)
  $(call LIBRARYNAME,libVrui): CFLAGS += -DVRUI_USE_OPENAL
endif
ifneq ($(IMAGES_USE_PNG),0)
  ifneq ($(VRUI_USE_PNG),0)
    $(call LIBRARYNAME,libVrui): CFLAGS += -DVRUI_USE_PNG
  endif
endif
$(call LIBRARYNAME,libVrui): $(call DEPENDENCIES,MYVRUI)
$(call LIBRARYNAME,libVrui): $(VRUI_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: libVrui
libVrui: $(call LIBRARYNAME,libVrui)

#
# The Vrui VR tool plug-in hierarchy:
#

# Implicit rule for creating VR tool plug-ins:
$(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT): PACKAGES += MYVRUI
$(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT): CFLAGS += $(CPLUGINFLAGS)
ifneq ($(VRUI_USE_OPENAL),0)
  $(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT): CFLAGS += -DVRUI_USE_OPENAL
endif
ifneq ($(SYSTEM_HAVE_TRANSITIVE_PLUGINS),0)
  $(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT): PLUGINLINKFLAGS += -L$(VRTOOLSDIR) $(DEPENDENCIES:$(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT)=-l%)
  ifneq ($(SYSTEM_HAVE_RPATH),0)
    ifneq ($(USE_RPATH),0)
      $(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT): PLUGINLINKFLAGS += -Wl,-rpath=$(PLUGININSTALLDIR)/VRTools
    endif
  endif
endif
$(VRTOOLSDIR)/lib%.$(PLUGINFILEEXT): $(OBJDIR)/Vrui/Tools/%.o
	@mkdir -p $(VRTOOLSDIR)
ifdef SHOWCOMMAND
	$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
else
	@echo Linking $@...
	@$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
endif

# Dependencies between Vrui tools:
$(VRTOOLSDIR)/libSixDofInputDeviceTool.$(PLUGINFILEEXT): DEPENDENCIES = $(VRTOOLSDIR)/libInputDeviceTool.$(PLUGINFILEEXT)
$(VRTOOLSDIR)/libSixDofInputDeviceTool.$(PLUGINFILEEXT): $(DEPENDENCIES)
$(VRTOOLSDIR)/libRayInputDeviceTool.$(PLUGINFILEEXT): DEPENDENCIES = $(VRTOOLSDIR)/libInputDeviceTool.$(PLUGINFILEEXT)
$(VRTOOLSDIR)/libRayInputDeviceTool.$(PLUGINFILEEXT): $(DEPENDENCIES)

$(VRTOOLSDIR)/libViewpointFileNavigationTool.$(PLUGINFILEEXT): $(OBJDIR)/Vrui/Tools/DenseMatrix.o \
                                                               $(OBJDIR)/Vrui/Tools/ViewpointFileNavigationTool.o
$(VRTOOLSDIR)/libCurveEditorTool.$(PLUGINFILEEXT): $(OBJDIR)/Vrui/Tools/DenseMatrix.o \
                                                   $(OBJDIR)/Vrui/Tools/CurveEditorTool.o

# Vrui tool settings:
$(OBJDIR)/Vrui/Tools/MouseNavigationTool.o: CFLAGS += -DDEFAULTMOUSECURSORIMAGEFILENAME='"$(SHAREINSTALLDIR)/Textures/Cursor.Xcur"'
$(OBJDIR)/Vrui/Tools/MouseDialogNavigationTool.o: CFLAGS += -DDEFAULTMOUSECURSORIMAGEFILENAME='"$(SHAREINSTALLDIR)/Textures/Cursor.Xcur"'
$(OBJDIR)/Vrui/Tools/JediTool.o: CFLAGS += -DDEFAULTLIGHTSABERIMAGEFILENAME='"$(SHAREINSTALLDIR)/Textures/Lightsaber.png"'
ifneq ($(IMAGES_USE_PNG),0)
  ifneq ($(VRUI_USE_PNG),0)
    $(OBJDIR)/Vrui/Tools/ScreenshotTool.o: CFLAGS += -DVRUI_USE_PNG
  endif
endif

# Mark all VR tool object files as intermediate:
.SECONDARY: $(VRTOOLS_SOURCES:%.cpp=$(OBJDIR)/%.o)

.PHONY: VRTools
VRTools: $(VRTOOLS)

#
# The Vrui vislet plug-in hierarchy:
#

# Implicit rule for creating vislet plug-ins:
$(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT): PACKAGES += MYVRUI
$(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT): CFLAGS += $(CPLUGINFLAGS)
ifneq ($(VRUI_USE_OPENAL),0)
  $(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT): CFLAGS += -DVRUI_USE_OPENAL
endif
ifneq ($(SYSTEM_HAVE_TRANSITIVE_PLUGINS),0)
  $(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT): PLUGINLINKFLAGS += -L$(VRVISLETSDIR) $(DEPENDENCIES:$(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT)=-l%)
  ifneq ($(SYSTEM_HAVE_RPATH),0)
    ifneq ($(USE_RPATH),0)
      $(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT): PLUGINLINKFLAGS += -Wl,-rpath=$(PLUGININSTALLDIR)/VRVislets
    endif
  endif
endif
$(VRVISLETSDIR)/lib%.$(PLUGINFILEEXT): $(OBJDIR)/Vrui/Vislets/%.o
	@mkdir -p $(VRVISLETSDIR)
ifdef SHOWCOMMAND
	$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
else
	@echo Linking $@...
	@$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
endif

# Dependencies between Vrui vislets:

# Mark all vislet object files as intermediate:
.SECONDARY: $(VRVISLETS_SOURCES:%.cpp=$(OBJDIR)/%.o)

.PHONY: VRVislets
VRVislets: $(VRVISLETS)

# The VR Device Daemon Test program:
$(EXEDIR)/DeviceTest: PACKAGES += MYVRUI
$(EXEDIR)/DeviceTest: $(call LIBRARYNAME,libVrui)
$(EXEDIR)/DeviceTest: $(OBJDIR)/Vrui/DeviceTest.o
.PHONY: DeviceTest
DeviceTest: $(EXEDIR)/DeviceTest

# The Vrui input device data file printer:
$(EXEDIR)/PrintInputDeviceDataFile: PACKAGES += MYVRUI
$(EXEDIR)/PrintInputDeviceDataFile: $(call LIBRARYNAME,libVrui)
$(EXEDIR)/PrintInputDeviceDataFile: $(OBJDIR)/Vrui/PrintInputDeviceDataFile.o
.PHONY: PrintInputDeviceDataFile
PrintInputDeviceDataFile: $(EXEDIR)/PrintInputDeviceDataFile


#
# The VR device driver daemon:
#

VRDEVICEDAEMON_SOURCES = VRDeviceDaemon/VRFactory.cpp \
                         VRDeviceDaemon/VRFactoryManager.cpp \
                         VRDeviceDaemon/VRDevice.cpp \
                         VRDeviceDaemon/VRCalibrator.cpp \
                         VRDeviceDaemon/VRDeviceManager.cpp \
                         VRDeviceDaemon/VRDeviceServer.cpp \
                         VRDeviceDaemon/VRDeviceDaemon.cpp

$(OBJDIR)/VRDeviceDaemon/VRDeviceManager.o: CFLAGS += -DSYSVRDEVICEDIRECTORY='"$(PLUGININSTALLDIR)/VRDevices"' \
                                                      -DSYSVRCALIBRATORDIRECTORY='"$(PLUGININSTALLDIR)/VRCalibrators"'
$(OBJDIR)/VRDeviceDaemon/VRDeviceDaemon.o: CFLAGS += -DSYSVRDEVICEDAEMONCONFIGFILENAME='"$(ETCINSTALLDIR)/VRDevices.cfg"'

$(EXEDIR)/VRDeviceDaemon: PACKAGES += MYGEOMETRY MYCOMM MYTHREADS MYMISC DL
$(EXEDIR)/VRDeviceDaemon: EXTRACINCLUDEFLAGS += $(MYVRUI_INCLUDE)
$(EXEDIR)/VRDeviceDaemon: CFLAGS += -DVERBOSE -DSYSDSONAMETEMPLATE='"lib%s.$(PLUGINFILEEXT)"'
$(EXEDIR)/VRDeviceDaemon: LINKFLAGS += $(PLUGINHOSTLINKFLAGS)
$(EXEDIR)/VRDeviceDaemon: $(call LIBRARYNAME,libGeometry) $(call LIBRARYNAME,libComm) $(call LIBRARYNAME,libMisc)
$(EXEDIR)/VRDeviceDaemon: $(VRDEVICEDAEMON_SOURCES:%.cpp=$(OBJDIR)/%.o)
.PHONY: VRDeviceDaemon
VRDeviceDaemon: $(EXEDIR)/VRDeviceDaemon

#
# The VR device driver plug-ins:
#

ifneq ($(HIDDEVICE_CPP_INPUT_H_HAS_STRUCTS),0)
  $(OBJDIR)/VRDeviceDaemon/HIDDevice.o: CFLAGS += -DINPUT_H_HAS_STRUCTS
endif

ifeq ($(SYSTEM),DARWIN)
  $(VRDEVICESDIR)/libHIDDevice.$(PLUGINFILEEXT): PLUGINLINKFLAGS += -framework System -framework IOKit -framework CoreFoundation
endif

$(VRDEVICESDIR)/libVRPNClient.$(PLUGINFILEEXT): $(OBJDIR)/VRDeviceDaemon/VRPNConnection.o \
                                                $(OBJDIR)/VRDeviceDaemon/VRPNClient.o

$(VRDEVICESDIR)/libWiimoteTracker.$(PLUGINFILEEXT): PLUGINLINKFLAGS += $(BLUETOOTH_LIBDIR) $(BLUETOOTH_LIBS)
$(VRDEVICESDIR)/libWiimoteTracker.$(PLUGINFILEEXT): $(OBJDIR)/VRDeviceDaemon/Wiimote.o \
                                                    $(OBJDIR)/VRDeviceDaemon/WiimoteTracker.o

# Implicit rule for creating plugins:
$(VRDEVICESDIR)/lib%.$(PLUGINFILEEXT): PACKAGES += MYGEOMETRY MYCOMM MYTHREADS MYMISC
$(VRDEVICESDIR)/lib%.$(PLUGINFILEEXT): EXTRACINCLUDEFLAGS += $(MYVRUI_INCLUDE)
$(VRDEVICESDIR)/lib%.$(PLUGINFILEEXT): CFLAGS += $(CPLUGINFLAGS) -DVERBOSE -DSYSDSONAMETEMPLATE='"lib%s.$(PLUGINFILEEXT)"'
$(VRDEVICESDIR)/lib%.$(PLUGINFILEEXT): $(OBJDIR)/VRDeviceDaemon/%.o
	@mkdir -p $(VRDEVICESDIR)
ifdef SHOWCOMMAND
	$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
else
	@echo Linking $@...
	@$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
endif

# Mark all VR device driver object files as intermediate:
.SECONDARY: $(VRDEVICES_SOURCES:%.cpp=$(OBJDIR)/%.o)

#
# The VR tracker calibrator plug-ins:
#

# Implicit rule for creating plugins:
$(VRCALIBRATORSDIR)/lib%.$(PLUGINFILEEXT): PACKAGES += MYGEOMETRY MYCOMM MYTHREADS MYMISC
$(VRCALIBRATORSDIR)/lib%.$(PLUGINFILEEXT): EXTRACINCLUDEFLAGS += $(MYVRUI_INCLUDE)
$(VRCALIBRATORSDIR)/lib%.$(PLUGINFILEEXT): CFLAGS += $(CPLUGINFLAGS) -DSYSDSONAMETEMPLATE='"lib%s.$(PLUGINFILEEXT)"'
$(VRCALIBRATORSDIR)/lib%.$(PLUGINFILEEXT): $(OBJDIR)/VRDeviceDaemon/%.o
	@mkdir -p $(VRCALIBRATORSDIR)
ifdef SHOWCOMMAND
	$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
else
	@echo Linking $@...
	@$(CCOMP) $(PLUGINLINKFLAGS) -o $@ $^
endif

# Mark all VR calibrator object files as intermediate:
.SECONDARY: $(VRCALIBRATORS_SOURCES:%.cpp=$(OBJDIR)/%.o)

.PHONY: VRCalibrators
VRCalibrators: $(VRCALIBRATORS)

########################################################################
# Specify installation rules for header files, libraries, executables,
# configuration files, and shared files.
########################################################################

SYSTEMPACKAGES = $(sort $(patsubst MY%,,$(PACKAGES_RECEXPAND)))
VRUIAPP_CFLAGS = $(CSYSFLAGS)
VRUIAPP_CFLAGS += -I$(HEADERINSTALLDIR)
VRUIAPP_CFLAGS += $(sort $(foreach PACKAGENAME,$(SYSTEMPACKAGES),$($(PACKAGENAME)_INCLUDE)))
VRUIAPP_CFLAGS += $(sort $(foreach PACKAGENAME,$(PACKAGES_RECEXPAND),$($(PACKAGENAME)_CFLAGS)))
ifneq ($(IMAGES_USE_PNG),0)
  VRUIAPP_CFLAGS += -DIMAGES_HAVE_PNG
endif
ifneq ($(IMAGES_USE_JPEG),0)
  VRUIAPP_CFLAGS += -DIMAGES_HAVE_JPEG
endif
VRUIAPP_LDIRS = -L$(LIBINSTALLDIR)
VRUIAPP_LDIRS += $(sort $(foreach PACKAGENAME,$(SYSTEMPACKAGES),$($(PACKAGENAME)_LIBDIR)))
VRUIAPP_LIBS = $(patsubst lib%,-l%.$(LDEXT),$(LIBRARY_NAMES))
VRUIAPP_LIBS += $(foreach PACKAGENAME,$(SYSTEMPACKAGES),$($(PACKAGENAME)_LIBS))
ifneq ($(SYSTEM_HAVE_RPATH),0)
  ifneq ($(USE_RPATH),0)
    VRUIAPP_LFLAGS += -Wl,-rpath=$(LIBINSTALLDIR)
  endif
endif

# Pseudo-target to dump Vrui compiler and linker flags to a makefile fragment
$(MAKEFILEFRAGMENT): PACKAGES = MYVRUI
$(MAKEFILEFRAGMENT): BuildRoot/SystemDefinitions BuildRoot/Packages makefile
	@echo Creating application makefile fragment...
	@echo "# Makefile fragment for Vrui applications" > $(MAKEFILEFRAGMENT)
	@echo "# Autogenerated by Vrui installation on $(shell date)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_CFLAGS = $(VRUIAPP_CFLAGS)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_LINKFLAGS = $(VRUIAPP_LDIRS) $(VRUIAPP_LIBS) $(VRUIAPP_LFLAGS)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_PLUGINFILEEXT = $(PLUGINFILEEXT)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_PLUGINCFLAGS = $(CPLUGINFLAGS)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_PLUGINLINKFLAGS = $(PLUGINLINKFLAGS)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_PLUGINHOSTLINKFLAGS = $(PLUGINHOSTLINKFLAGS)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_TOOLSDIR = $(VRTOOLSDIR)" >> $(MAKEFILEFRAGMENT)
	@echo "VRUI_VISLETSDIR = $(VRVISLETSDIR)" >> $(MAKEFILEFRAGMENT)
ifeq ($(SYSTEM),DARWIN)
	@echo "export MACOSX_DEPLOYMENT_TARGET = $(SYSTEM_DARWIN_VERSION)" >> $(MAKEFILEFRAGMENT)
endif

# Sequence to create symlinks for dynamic libraries:
# First argument: library name
define CREATE_SYMLINK
@-rm -f $(LIBINSTALLDIR)/$(call DSONAME,$(1)) $(LIBINSTALLDIR)/$(call LINKDSONAME,$(1))
@ln -s $(LIBINSTALLDIR)/$(call FULLDSONAME,$(1)) $(LIBINSTALLDIR)/$(call DSONAME,$(1))
@ln -s $(LIBINSTALLDIR)/$(call FULLDSONAME,$(1)) $(LIBINSTALLDIR)/$(call LINKDSONAME,$(1))

endef

install: all $(MAKEFILEFRAGMENT)
# Install all header files in HEADERINSTALLDIR:
	@echo Installing header files...
	@install -d $(HEADERINSTALLDIR)/Misc
	@install -m u=rw,go=r $(MISC_HEADERS) $(HEADERINSTALLDIR)/Misc
	@install -d $(HEADERINSTALLDIR)/Plugins
	@install -m u=rw,go=r $(PLUGINS_HEADERS) $(HEADERINSTALLDIR)/Plugins
	@install -d $(HEADERINSTALLDIR)/Realtime
	@install -m u=rw,go=r $(REALTIME_HEADERS) $(HEADERINSTALLDIR)/Realtime
	@install -d $(HEADERINSTALLDIR)/Threads
	@install -m u=rw,go=r $(THREADS_HEADERS) $(HEADERINSTALLDIR)/Threads
	@install -d $(HEADERINSTALLDIR)/Comm
	@install -m u=rw,go=r $(COMM_HEADERS) $(HEADERINSTALLDIR)/Comm
	@install -d $(HEADERINSTALLDIR)/Math
	@install -m u=rw,go=r $(MATH_HEADERS) $(HEADERINSTALLDIR)/Math
	@install -d $(HEADERINSTALLDIR)/Geometry
	@install -m u=rw,go=r $(GEOMETRY_HEADERS) $(HEADERINSTALLDIR)/Geometry
	@install -d $(HEADERINSTALLDIR)/GL
	@install -m u=rw,go=r $(GLWRAPPERS_HEADERS) $(HEADERINSTALLDIR)/GL
	@install -m u=rw,go=r $(GLSUPPORT_HEADERS) $(HEADERINSTALLDIR)/GL
	@install -d $(HEADERINSTALLDIR)/GL/Extensions
	@install -m u=rw,go=r $(GLSUPPORTEXTENSION_HEADERS) $(HEADERINSTALLDIR)/GL/Extensions
	@install -m u=rw,go=r $(GLXSUPPORT_HEADERS) $(HEADERINSTALLDIR)/GL
	@install -m u=rw,go=r $(GLGEOMETRY_HEADERS) $(HEADERINSTALLDIR)/GL
	@install -d $(HEADERINSTALLDIR)/GLMotif
	@install -m u=rw,go=r $(GLMOTIF_HEADERS) $(HEADERINSTALLDIR)/GLMotif
	@install -d $(HEADERINSTALLDIR)/Images
	@install -m u=rw,go=r $(IMAGES_HEADERS) $(HEADERINSTALLDIR)/Images
ifneq ($(VRUI_USE_OPENAL),0)
	@install -d $(HEADERINSTALLDIR)/AL
	@install -m u=rw,go=r $(ALSUPPORT_HEADERS) $(HEADERINSTALLDIR)/AL
endif
	@install -d $(HEADERINSTALLDIR)/Vrui
	@install -m u=rw,go=r $(VRUI_HEADERS) $(HEADERINSTALLDIR)/Vrui
	@install -d $(HEADERINSTALLDIR)/Vrui/Tools
	@install -m u=rw,go=r $(VRUI_TOOLHEADERS) $(HEADERINSTALLDIR)/Vrui/Tools
# Install all library files in LIBINSTALLDIR:
	@echo Installing libraries...
	@install -d $(LIBINSTALLDIR)
	@install $(LIBRARIES) $(LIBINSTALLDIR)
	@echo Configuring run-time linker...
	$(foreach LIBNAME,$(LIBRARY_NAMES),$(call CREATE_SYMLINK,$(LIBNAME)))
# Install all binaries in BININSTALLDIR:
	@echo Installing executables...
	@install -d $(EXECUTABLEINSTALLDIR)
	@install $(EXECUTABLES) $(EXECUTABLEINSTALLDIR)
ifeq ($(SYSTEM),DARWIN)
	@cp Share/MacOSX/runwithx Share/MacOSX/runwithx.tmp
	@sed -i "" "s:_VRUIETCDIR_:$(ETCINSTALLDIR):" Share/MacOSX/runwithx.tmp
	@sed -i "" "s:_VRUIBINDIR_:$(EXECUTABLEINSTALLDIR):" Share/MacOSX/runwithx.tmp
	@install Share/MacOSX/runwithx.tmp $(EXECUTABLEINSTALLDIR)/runwithx
	@rm Share/MacOSX/runwithx.tmp
endif
# Install all plug-ins in PLUGININSTALLDIR:
	@echo Installing plug-ins...
	@install -d $(PLUGININSTALLDIR)/VRTools
	@install $(VRTOOLS) $(PLUGININSTALLDIR)/VRTools
	@install -d $(PLUGININSTALLDIR)/VRVislets
	@install $(VRVISLETS) $(PLUGININSTALLDIR)/VRVislets
	@install -d $(PLUGININSTALLDIR)/VRDevices
	@install $(VRDEVICES) $(PLUGININSTALLDIR)/VRDevices
	@install -d $(PLUGININSTALLDIR)/VRCalibrators
	@install $(VRCALIBRATORS) $(PLUGININSTALLDIR)/VRCalibrators
# Install all configuration files in ETCINSTALLDIR:
	@echo Installing configuration files...
	@install -d $(ETCINSTALLDIR)
	@install -m u=rw,go=r Share/Vrui.cfg Share/VRDevices.cfg $(ETCINSTALLDIR)
ifeq ($(SYSTEM),DARWIN)
	@install -m u=rw,go=r Share/MacOSX/Vrui.xinitrc $(ETCINSTALLDIR)
endif
# Install all shared files in SHAREINSTALLDIR:
	@echo Installing shared files...
	@install -d $(SHAREINSTALLDIR)/GLFonts
	@install -m u=rw,go=r Share/GLFonts/* $(SHAREINSTALLDIR)/GLFonts
	@install -d $(SHAREINSTALLDIR)/Textures
	@install -m u=rw,go=r Share/Textures/* $(SHAREINSTALLDIR)/Textures
# Install makefile fragment in ETCINSTALLDIR:
	@echo Installing application makefile fragment...
	@install -m u=rw,go=r $(MAKEFILEFRAGMENT) $(ETCINSTALLDIR)
