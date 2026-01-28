; This file is a template for creating your own local setupvars.iss file included by setup.iss
; setupvars.iss is not version controlled as it is user-specific
; Copy this file to setupvars.iss and update the paths for your system

; Directory where TristansKortAnimator.exe is located (after windeployqt)
#define AppBuildDir "C:\CODE\TristansKortAnimator\build\Release"

; Directory for MSVC redistributable package (vcredist)
; Update this path based on your Visual Studio installation
#define MsvcRedist_Dir "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Redist\MSVC\v143"
