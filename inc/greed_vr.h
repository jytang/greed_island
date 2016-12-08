#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <openvr.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene_transform.h"
#include "bounding_sphere.h"

#define TRACKPAD 0
#define TRIGGER 1

#define TRACKPAD_ID 4294967296
#define TRIGGER_ID 8589934592
#define BUTTON_ID 2
#define GRIP_ID 4

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
	static vr_vars vars;

	static void init();

	static std::string getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr);
	static vr::IVRSystem* initOpenVR(uint32_t& hmdWidth, uint32_t& hmdHeight);
	static void ProcessVREvent(const vr::VREvent_t & event);
	static glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	static void vr_update_controllers(Scene * scene, SceneTransform * controller_1_transform, SceneTransform * controller_2_transform, glm::mat4 cam_pos_mat);
	static void vr_check_interaction(SceneTransform *controller_1_transform, SceneTransform *controller_2_transform, std::vector<BoundingSphere *> interactable_objects);
};