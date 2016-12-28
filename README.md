# greed_island
An HTC Vive VR Experience.. on an island!

Joshua Tang
Varanon Austin Pukasamsombut

https://www.youtube.com/watch?v=arQL7ayVPCk

Notes for Running this on a PC (Windows):
- Open the project using Microsoft Visual Studio 2015
- Make sure the NuGet Packages glm, nupengl.core.redist, and nupengl.core are installed
- Download SOIL (http://www.lonesock.net/soil.html) and build the VC9 version, then move the generated
  SOIL.lib file to the project directory under "%(ProjectDir)/lib/soil/lib/x64/" (or x86 instead if building under Win32.
- Go to the project properties and under ConfigurationSetting->Debugging, have the "Environment" be set to
  PATH=%PATH%;$(ProjectDir)\lib\openvr\bin\win32; or win64 if using running using 64 bit.
- For the VR Portion to run, SteamVR is required. Change the "vr_on" to true in "greed.cpp"
