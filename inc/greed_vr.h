#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <openvr.h>
#include <string>
#include <glm/glm.hpp>

struct vr_vars {
	int numEyes = 2;
	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	GLuint framebuffer[2];
	GLuint colorRenderTarget[2];
	GLuint depthRenderTarget[2];
	uint32_t framebufferWidth = 1280, framebufferHeight = 720;
	vr::IVRSystem* hmd = nullptr;
};

class GreedVR
{
public:
	GreedVR();
	~GreedVR();

	static vr_vars vars;

	static void init();

	static std::string getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr);
	static vr::IVRSystem* initOpenVR(uint32_t& hmdWidth, uint32_t& hmdHeight);
	static void ProcessVREvent(const vr::VREvent_t & event);
	static glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
};