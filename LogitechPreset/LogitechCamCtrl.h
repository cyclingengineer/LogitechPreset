// LogitechCamCtrl1.h: interface for the LogitechCamCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGITECHCAMCTRL1_H__F624D230_C54B_45A9_B36D_CBED1759DBEE__INCLUDED_)
#define AFX_LOGITECHCAMCTRL1_H__F624D230_C54B_45A9_B36D_CBED1759DBEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dshow.h>
#include <Ks.h>				// Required by KsMedia.h
#include <KsMedia.h>		// For KSPROPERTY_CAMERACONTROL_FLAGS_*

#include "LVUVCPublic.h"

#include <Mmsystem.h>
#include "vidcap.h"

struct ControlInfo {
	long min;
	long max;
	long step;
	long def;
	long flags;
};

typedef struct PRESET_VALUE {
	int nPos_X;
	int nPos_Y;
	int nNumberPos;
} PRESET_VALUE;

#define EXPOSURE_TIME_UNIT	1 // = 100us
#define EXPOSURE_TIME_10MS	100*EXPOSURE_TIME_UNIT
#define EXPOSURE_TIME_20MS	2*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_30MS	3*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_40MS	4*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_50MS	5*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_60MS	6*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_70MS	7*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_80MS	8*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_90MS	9*EXPOSURE_TIME_10MS
#define EXPOSURE_TIME_100MS	10*EXPOSURE_TIME_10MS

#define EXPOSURE_TIME_DEFAULT EXPOSURE_TIME_10MS
/*
 * Enumerate all video devices
 *
 * See also:
 *
 * Using the System Device Enumerator:
 *     http://msdn2.microsoft.com/en-us/library/ms787871.aspx
 */
int enum_devices(char* devname, IBaseFilter** captureFilter);


class LogitechCamCtrl  
{
public:
	enum ControlInfoType
	{
		CIT_PAN=1,
		CIT_TILT,
		CIT_ZOOM,
		CIT_EXPOSURE,
		CIT_GAIN,
		CIT_BRIGHTNESS,
		CIT_CONTRAST,
		CIT_COLOR_INTENSITY,
		CIT_WB
	};

public:
	LogitechCamCtrl();
	virtual ~LogitechCamCtrl();
	bool open(char* devName);
	void close();
	void resetMechanicalPanTilt();
	void gotoHome();

	// Test
	void test1();

	// Image quality

	int setExposure(int exposure);
	int getExposure(int & exposure);

	int setGain(int gain);
	int getGain(int & gain);

	int setAutoExposure(bool on);
	int getAutoExposure(bool & on);

	int setBrightness(int brightness);
	int getBrightness(int & brightness);

	int setContrast(int contrast);
	int getContrast(int & contrast);

	int setColorIntensity(int intensity);
	int getColorIntensity(int & intensity);

	int setAutoWB(bool on);
	int getAutoWB(bool & on);

	int setWB(int wb);
	int getWB(int & wb);

	// Anti-flicker
	void setAntuflicker(int mode); // mode = 0: Off, =1: NTSD-60Hz, =2: PAL-50Hz	

	// type = 1 : panInfo, = 2 : tiltInfo, = 3 : zoomInfo, = 4 : exposureInfo
	int getControlInfo(ControlInfoType type, ControlInfo & info);

	// Digitai Pan, Til, Zoom
	// zoom = [100, 400]; zoom=100: no zoom; step = 2;
	int setDigitalZoom(int zoom);
	int getDigitalZoom(int & zoom);

	// pan = [-100, 100], default = 0: center, step = 1
	int setDigitalPan(int pan);
	int getDigitalPan(int & pan);

	void setMechanicalPan(long _val = 0);
	void getMechanicalPan(long & _val);

	// tilt = [-100, 100], default = 0: center, step = 1
	int setDigitalTilt(int tilt);
	int getDigitalTilt(int & tilt);

	void setMechanicalTilt(long _value = 0);
	void getMechanicalTilt(long & _value);

	void setPresetPos(int nNumber);
	void gotoPresetPos(int nNumber);

	int TurnUp();
	int TurnRight();
	int TurnLeft();
	int TurnDown();
	int ZoomIn();
	int ZoomOut();
	int ChangeBrightness(bool bAdd);

	void getValue();

	/*
	Auto-exposure priority determines whether the camera can dynamically vary the frame rate depending on lighting conditions.

	Without auto-exposure, for instance, if the frame rate is 30 fps, the exposure time cannot exceed 33 ms.

	With auto-exposure priority, however, the camera could compensate for low lighting by decreasing the frame rate. For instance, the camera could reduce frame rate to 25 fps, thereby lengthening exposure time to 40 ms.

	KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY maps to the Low-Light Compensation check box on the USB Video Class property page.

	In order to use KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, you must set KSPROPERTY_CAMERACONTROL_EXPOSURE to auto. In other words, the camera must be in auto-exposure mode for the auto-exposure-priority mode to be a valid option.

	The default value for KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY is zero.

	Available for Windows Vista and later versions of the Windows operating system.
	**/
	int setRightLight(bool on);
	int getRightLight(bool & on);	

	static void CALLBACK TimeProcChecking(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

protected:
	int getKsPropCamCtrl(GUID guidPropSet, int propId, int & outVal);
	int setKsPropCamCtrl(GUID guidPropSet, int propId, int inVal);

	int getKsPropLP1(GUID guidPropSet, int propId, void* ksLP1Header, int headerSize);
	int setKsPropLP1(GUID guidPropSet, int propId, void* ksLP1Header, int headerSize);

protected:
	IBaseFilter*		capture;
	IKsPropertySet*		ksProp;
	IAMCameraControl*	amCamCtrl;
	ICameraControl*		VidCamCtrl;
	IAMVideoProcAmp*	amVpa;

	bool m_bMechanicalPanTilt;

	int m_nGain;
	int m_nZoom;
	int m_nPan;
	int m_nTil;

	int m_nPos_X;
	int m_nPos_Y;

	PRESET_VALUE m_aPreset[30];
};

#endif // !defined(AFX_LOGITECHCAMCTRL1_H__F624D230_C54B_45A9_B36D_CBED1759DBEE__INCLUDED_)

