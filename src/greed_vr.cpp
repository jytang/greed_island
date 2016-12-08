#include "greed_vr.h"
#include "util.h"

bool trigger_1_pressed = false;
bool trigger_2_pressed = false;
SceneTransform *grabbed_object_1 = nullptr;
SceneTransform *grabbed_object_2 = nullptr;
glm::mat4 controller_offset = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -0.06f));

void GreedVR::init()
{
	vars.hmd = GreedVR::initOpenVR(vars.framebufferWidth, vars.framebufferHeight);

	glGenFramebuffers(2, vars.framebuffer);

	glGenTextures(vars.numEyes, vars.colorRenderTarget);
	glGenTextures(vars.numEyes, vars.depthRenderTarget);
	for (int eye = 0; eye < vars.numEyes; ++eye) {
		glBindTexture(GL_TEXTURE_2D, vars.colorRenderTarget[eye]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, vars.framebufferWidth, vars.framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, vars.depthRenderTarget[eye]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, vars.framebufferWidth, vars.framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

		glBindFramebuffer(GL_FRAMEBUFFER, vars.framebuffer[eye]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, vars.colorRenderTarget[eye], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, vars.depthRenderTarget[eye], 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/** Called by initOpenVR */
std::string GreedVR::getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
	if (unRequiredBufferLen == 0) {
		return "";
	}

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;

	return sResult;
}


/** Call immediately before initializing OpenGL

\param hmdWidth, hmdHeight recommended render target resolution
*/
vr::IVRSystem* GreedVR::initOpenVR(uint32_t& hmdWidth, uint32_t& hmdHeight)
{
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem* hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None) {
		fprintf(stderr, "OpenVR Initialization Error: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return nullptr;
	}

	const std::string& driver = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	const std::string& model = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);
	const std::string& serial = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
	const float freq = hmd->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);

	//get the proper resolution of the hmd
	hmd->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);

	fprintf(stderr, "HMD: %s '%s' #%s (%d x %d @ %g Hz)\n", driver.c_str(), model.c_str(), serial.c_str(), hmdWidth, hmdHeight, freq);

	// Initialize the compositor
	vr::IVRCompositor* compositor = vr::VRCompositor();
	if (!compositor) {
		fprintf(stderr, "OpenVR Compositor initialization failed. See log file for details\n");
		vr::VR_Shutdown();
	}

	return hmd;
}

void GreedVR::ProcessVREvent(const vr::VREvent_t & event)
{
	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		fprintf(stderr, "Device %u attached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		fprintf(stderr, "Device %u detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		fprintf(stderr, "Device %u updated.\n", event.trackedDeviceIndex);
	}
	break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
glm::mat4 GreedVR::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}


void GreedVR::vr_update_controllers(Scene *scene, SceneTransform * controller_1_transform, SceneTransform * controller_2_transform, glm::mat4 cam_pos_mat)
{
	// don't draw controllers if somebody else has input focus
	if (vars.hmd->IsInputFocusCapturedByAnotherProcess())
		return;

	int trackedControllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!vars.hmd->IsTrackedDeviceConnected(unTrackedDevice))
			continue;
		if (vars.hmd->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;
		trackedControllerCount += 1;
		if (!vars.trackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		glm::mat4 mat = ConvertSteamVRMatrixToMatrix4(vars.trackedDevicePose[unTrackedDevice].mDeviceToAbsoluteTracking);

		if (trackedControllerCount == 1)
			controller_1_transform->transformation = cam_pos_mat * mat;
		else if (trackedControllerCount == 2)
			controller_2_transform->transformation = cam_pos_mat * mat;
	}
}

void GreedVR::vr_check_interaction(SceneTransform *controller_1_transform, SceneTransform *controller_2_transform, std::vector<BoundingSphere *> interactable_objects)
{
	// check if somebody else has input focus
	if (GreedVR::vars.hmd->IsInputFocusCapturedByAnotherProcess())
		return;

	int trackedControllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		//Preliminary Checks
		if (!GreedVR::vars.hmd->IsTrackedDeviceConnected(unTrackedDevice))
			continue;
		if (GreedVR::vars.hmd->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;		
		if (!GreedVR::vars.trackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		//Get Controller State
		vr::VRControllerState_t pControllerState;
		if (!GreedVR::vars.hmd->GetControllerState(unTrackedDevice, &pControllerState, sizeof(pControllerState)))
			continue;

		trackedControllerCount += 1;

		if (pControllerState.ulButtonPressed == TRIGGER_ID || pControllerState.ulButtonPressed == (TRACKPAD_ID + TRIGGER_ID))
		{
			if (trackedControllerCount == 1) //Seperating allows player to carry two objects
			{
				//Checks if trigger already being pressed down
				if (trigger_1_pressed && grabbed_object_1 != nullptr)
				{
					grabbed_object_1->transformation = controller_1_transform->transformation * controller_offset;
					continue;
				}
				else
					trigger_1_pressed = true;

				glm::vec3 controller_center = glm::vec3(controller_1_transform->transformation * glm::vec4(0.f, 0.f, 0.06f, 1.f));

				for (BoundingSphere *obj : interactable_objects)
				{
					glm::vec3 obj_center = glm::vec3(obj->translation_mat->transformation * glm::vec4(0.f, 0.f, 0.f, 1.f));
					if (glm::abs(glm::distance(controller_center, obj_center)) <= obj->radius)
					{
						if (obj->interact_type == GRAB)
							grabbed_object_1 = obj->translation_mat;
						else
							obj->check_interact = true;
					}
				}
			}
			else if (trackedControllerCount == 2) //Seperating allows player to carry two objects
			{
				//Checks if trigger already being pressed down
				if (trigger_2_pressed && grabbed_object_2 != nullptr)
				{
					grabbed_object_2->transformation = controller_2_transform->transformation * controller_offset;
					continue;
				}
				else
					trigger_2_pressed = true;

				glm::vec3 controller_center = glm::vec3(controller_2_transform->transformation * glm::vec4(0.f, 0.f, 0.06f, 1.f));

				for (BoundingSphere *obj : interactable_objects)
				{
					glm::vec3 obj_center = glm::vec3(obj->translation_mat->transformation * glm::vec4(0.f, 0.f, 0.f, 1.f));
					if (glm::abs(glm::distance(controller_center, obj_center)) <= obj->radius)
					{
						if (obj->interact_type == GRAB)
							grabbed_object_2 = obj->translation_mat;
						else
							obj->check_interact = true;
					}
				}
			}
		}
		else if (trigger_1_pressed && trackedControllerCount == 1)
		{
			trigger_1_pressed = false;
			if (grabbed_object_1 != nullptr)
				grabbed_object_1 = nullptr;
		}
		else if (trigger_2_pressed && trackedControllerCount == 2)
		{
			trigger_2_pressed = false;
			if (grabbed_object_2 != nullptr)
				grabbed_object_2 = nullptr;
		}
	}
}