#pragma once

struct KinectJoint {
	int parent;
	int joint;
	int child;
	int smpl;

	KinectJoint(int p, int j, int c, int s) : parent(p), joint(j), child(c), smpl(s) {}
};

#define KINECT_ROTATIONS_COUNT 19

struct KinectRotations {
	DirectX::XMVECTOR rotations[KINECT_ROTATIONS_COUNT];
	KinectJoint hierarchy[KINECT_ROTATIONS_COUNT];

	KinectRotations();

	void printQuaternions() const;
};