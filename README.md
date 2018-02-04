# SkeletonTracker

The project demonstrates avateering using 
* Kinect SkeletonFrame Data (Kinect v1.8) 
* SMPL model http://smpl.is.tue.mpg.de
</br>
Demo - https://www.youtube.com/watch?v=lUaVzO8DGoA

# Interface

* Interrupt - interrupts continuous perstisting of SkeletonFrame Data
* Next Frame - if interrupt enabled, fetch new SkeletonFrame, dump state to files
* Scale/Rotate - allows for scaling and rotation of 3D content. The numbers should be separated with spaces
* Apply Transform - apply scaling/rotation
* Fix Mapping - fix kinect skeleton as the t-pose (bind pose) to animate 3D content
* Online - updates 3D content in continuous fashion
* Apply Rotations - updates 3D content if online is not set, dumps state to files
* LBS Only - disables pose blendshape corrections from SMPL model, leaves Linear Blend Skinning only
* 3D - radio-buttons to select between SMPL Model/SMPL Skeleton

# Constraints
* Only one skeleton supported
* The spine is rigid

# Dependencies/Requirements
* Visual Studio 2017 Build Tools
* Windows 10 SDK (used 10.0.16299.0)
* Kinect v1.8 SDK
* Eigen (used 3.3.4) 
* Kinect v1 Device
* DirectX 11
