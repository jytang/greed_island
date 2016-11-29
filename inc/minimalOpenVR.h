/**
  \file minimalOpenGL/minimalOpenVR.h
  \author Morgan McGuire, http://graphics.cs.williams.edu
  Distributed with the G3D Innovation Engine http://g3d.cs.williams.edu

  Minimal headers for including Valve's OpenVR / SteamVR API, 
  which currently supports Vive and Oculus Virtual Reality 
  head mounted displays (HMDs). This does not depend on any 
  vector math library or specific OpenGL intialization library.

  This requires the bin, lib, and headers directories from the 
  OpenVR SDK (which are also distributed with G3D):
   
     https://github.com/ValveSoftware/openvr

  The runtime for OpenVR is distributed with Steam. Ensure that 
  you've run Steam and let it update to the latest SteamVR before
  running an OpenVR program.
*/

#include <openvr.h>
#include <string>

/** Called by initOpenVR */
std::string getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr) {
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
vr::IVRSystem* initOpenVR(uint32_t& hmdWidth, uint32_t& hmdHeight) {
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem* hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None) {
        fprintf(stderr, "OpenVR Initialization Error: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
        return nullptr;
	}
    
	const std::string& driver = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	const std::string& model  = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);
	const std::string& serial = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
    const float freq = hmd->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);

    //get the proper resolution of the hmd
    hmd->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);

    fprintf(stderr, "HMD: %s '%s' #%s (%d x %d @ %g Hz)\n", driver.c_str(), model.c_str(), serial.c_str(), hmdWidth, hmdHeight, freq);

    // Initialize the compositor
    vr::IVRCompositor* compositor = vr::VRCompositor();
	if (! compositor) {
		fprintf(stderr, "OpenVR Compositor initialization failed. See log file for details\n");
        vr::VR_Shutdown();
	}

    return hmd;
}

void ProcessVREvent(const vr::VREvent_t & event)
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
glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
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
void RenderControllerAxes(vr::TrackedDevicePose_t trackedDevicePose[])
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

		if (hmd->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller) {
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

}*/