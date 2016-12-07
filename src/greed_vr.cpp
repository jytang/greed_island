#include "greed_vr.h"

GreedVR::GreedVR() {}

GreedVR::~GreedVR() {}

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


/*
void GreedVR::vr_update_controllers(SceneTransform)
{
	// don't draw controllers if somebody else has input focus
	if (hmd->IsInputFocusCapturedByAnotherProcess())
		return;

	std::vector<float> vertdataarray;
	unsigned int controllerVertcount = 0;
	int trackedControllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!hmd->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		if (hmd->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
		{
			continue;
		}

		trackedControllerCount += 1;

		if (!trackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		glm::mat4 mat = ConvertSteamVRMatrixToMatrix4(trackedDevicePose[unTrackedDevice].mDeviceToAbsoluteTracking);

		glm::vec4 center = mat * glm::vec4(0, 0, 0, 1);

		fprintf(stderr, "Printing for device %u: Center is %f\t%f\t%f\n", unTrackedDevice, center.x, center.y, center.z);

		for (int i = 0; i < 3; ++i)
		{
			glm::vec3 color(0, 0, 0);
			glm::vec4 point(0, 0, 0, 1);
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back(center.x);
			vertdataarray.push_back(center.y);
			vertdataarray.push_back(center.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			vertdataarray.push_back(point.x);
			vertdataarray.push_back(point.y);
			vertdataarray.push_back(point.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			controllerVertcount += 2;
		}

		glm::vec4 start = mat * glm::vec4(0, 0, -0.02f, 1);
		glm::vec4 end = mat * glm::vec4(0, 0, -39.f, 1);
		glm::vec3 color(.92f, .92f, .71f);

		vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);
		controllerVertcount += 2;
	}

	Window::setControllerAxes(vertdataarray, controllerVertcount);
}
*/
