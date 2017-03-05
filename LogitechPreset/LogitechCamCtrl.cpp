// LogitechCamCtrl1.cpp: implementation of the LogitechCamCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogitechCamCtrl.h"


#include <string>


#include <comutil.h>
#include "logitechcamctrl.h"

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//#pragma comment(lib, "comsupp.lib")
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

int enum_devices(char* devname, IBaseFilter** captureFilter)
{
	HRESULT hr;

	printf("Enumerating video input devices ...\n");
	CoInitializeEx(NULL, COINIT_MULTITHREADED); // init COM objects in multi-threading  mode

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if(FAILED(hr))
	{
		fprintf(stderr, "ERROR: Unable to create system device enumerator.\n");
		return hr;
	}

	// Obtain a class enumerator for the video input device category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if(hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if(SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					OutputDebugStringW(varName.bstrVal);
					std::string strFromBstr = (const char*)_bstr_t(V_BSTR(&varName));
					if (devname && strcmp(devname, strFromBstr.c_str())==0) { // FOUND
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)captureFilter);
						VariantClear(&varName);
						pPropBag->Release();
						pMoniker->Release();
						pEnumCat->Release();
						pSysDevEnum->Release();
						return 0;
					}
				}
				VariantClear(&varName);

				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//___________________________________________________________________________
LogitechCamCtrl::LogitechCamCtrl()
{
	capture = 0;
	ksProp = 0;
	amCamCtrl = 0;
	VidCamCtrl = 0;
	amVpa = 0;
	m_nGain = 0;
	m_nZoom = 100;
	m_nPan = 0;
	m_nTil = 0;

	m_bMechanicalPanTilt = false;

	m_nPos_X = 0;
	m_nPos_Y = 0;


}

LogitechCamCtrl::~LogitechCamCtrl()
{
	close();
}

bool LogitechCamCtrl::open(char* devName)
{
	HRESULT hr;

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

	const IID IID_ICameraControl = {0x2BA1785D, 0x4D1B, 0x44EF, {0x85,0xE8,0xC7,0xF1,0xD3,0xF2,0x01,0x84}};

	// Check invalid input param
	if (!devName || strcmp(devName, "")==0)
		return 0;

	// Get video capture filter
	enum_devices(devName, &capture);	
	if (!capture)
		return 0;

	if(strstr("QuickCam Orbit/Sphere AF", devName) != NULL)
		m_bMechanicalPanTilt = true;

	// Get video config interface
	capture->QueryInterface(IID_IKsPropertySet, (void **)&ksProp);
	if(!ksProp)	{
		fprintf(stderr, "ERROR: Unable to access IID_IKsPropertySet interface.\n");
		close();
		return 0;
	}
	// Get a pointer to the IAMCameraControl interface used to control the camera
	capture->QueryInterface(IID_IAMCameraControl, (void **)&amCamCtrl);
	if(!amCamCtrl)
	{
		fprintf(stderr, "ERROR: Unable to access IAMCameraControl interface.\n");
		return 0;
	}

	////CComQIPtr<IKsTopologyInfo> pTopo(capture);
	//IKsTopologyInfo* pTopo = NULL;

	//hr = capture->QueryInterface(__uuidof(IKsTopologyInfo), 
 //                              (void **) &pTopo);

	//if(hr !=S_OK)
	//{
	//	TRACE("ERROR: Unable to access IKsTopologyInfo interface.\n");
	//	return E_NOINTERFACE;
	//}

	//DWORD dNumNode =0;
	//DWORD dNameLen =0;
	//pTopo->get_NumNodes(&dNumNode);
	//char szNodeName[50];
	//memset(szNodeName, 0, 50);

	//for(int i=0; i<dNumNode; i++)
	//{
	//	//hr = pTopo->get_NodeName(i, (WCHAR*)szNodeName, 50, &dNameLen);
	//	GUID nodeType;

	//	hr = pTopo->get_NodeType(i, &nodeType);

	//	if(hr == S_OK && IsEqualGUID(nodeType, KSNODETYPE_VIDEO_CAMERA_TERMINAL))
	//	{
	//		hr = pTopo->CreateNodeInstance(i, __uuidof(ICameraControl), (void**)&VidCamCtrl);

	//		if(hr == S_OK)
	//			break;
	//	}

	//}


	capture->QueryInterface(IID_IAMVideoProcAmp, (void **)&amVpa);
	if(!amVpa)
	{
		fprintf(stderr, "ERROR: Unable to access IAMVideoProcAmp interface.\n");
		return 0;
	}

	//Setting HD Camera
	if(!m_bMechanicalPanTilt)
	{
		setRightLight(true);
		setAutoExposure(true);

		getGain(m_nGain);

		setDigitalZoom(m_nZoom);
	}

	//timeSetEvent(10000,0,TimeProcChecking,(DWORD_PTR)this,TIME_CALLBACK_FUNCTION);

	return 1;
}

void LogitechCamCtrl::resetMechanicalPanTilt()
{
	if(amCamCtrl == NULL)
		return;

	setMechanicalPan(180);
	Sleep(800);
	setMechanicalTilt(180);
	Sleep(800);
	setMechanicalPan(-64);
	Sleep(800);
	setMechanicalTilt(-24);
	Sleep(800);

	
	for(int i=0; i<100; i++)
	{
		m_aPreset[i].nPos_X = 0;
		m_aPreset[i].nPos_Y = 0;
		m_aPreset[i].nNumberPos = -1;
	}

}

void LogitechCamCtrl::gotoHome()
{
	//setMechanicalTilt(48);
	//Sleep(800);
	//setMechanicalTilt(-24);
	//Sleep(800);	

	//setMechanicalPan(128);
	//Sleep(800);
	//setMechanicalPan(-64);
	//Sleep(800);

	m_nPos_X = 0;
	m_nPos_Y = 0;
}

void LogitechCamCtrl::getValue()
{
	long value = 0;

	getMechanicalPan(value);
	getMechanicalTilt(value);
}

void LogitechCamCtrl::setPresetPos(int nNumber)
{
	for(int i=0; i<100; i++)
	{
		//Neu da ton tai vi tri thi ghi de
		//Neu khong ghi vao vi tri chua co
		if((m_aPreset[i].nNumberPos == nNumber) || (m_aPreset[i].nNumberPos == -1) )
		{
			m_aPreset[i].nPos_X = m_nPos_X;
			m_aPreset[i].nPos_Y = m_nPos_Y;
			m_aPreset[i].nNumberPos = nNumber;
			return;
		}
	}
}

void LogitechCamCtrl::gotoPresetPos(int nNumber)
{
	int nArrayPos = 0;
	int nDistanceX = 0;
	int nDistanceY = 0;

	for(int i=0; i<100; i++)
	{
		if(m_aPreset[i].nNumberPos == nNumber)
		{
			nArrayPos = i;
			break;
		}
		else if(m_aPreset[i].nNumberPos == -1)
			return;
	}

	nDistanceX = m_aPreset[nArrayPos].nPos_X - m_nPos_X;
	nDistanceY = m_aPreset[nArrayPos].nPos_Y - m_nPos_Y;	

	setMechanicalPan(nDistanceX);
	Sleep(300);
	setMechanicalTilt(nDistanceY);

	m_nPos_X = m_aPreset[nArrayPos].nPos_X;
	m_nPos_Y = m_aPreset[nArrayPos].nPos_Y;
}

void LogitechCamCtrl::TimeProcChecking(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	LogitechCamCtrl* pThis = (LogitechCamCtrl*)dwUser;

	int nExposure = 0;

	pThis->setRightLight(false);
	pThis->setAutoExposure(false);

	nExposure = 2;//GetPrivateProfileInt("av_settings", "av_exposure", 2, ".\\ES100.ini");
	if(nExposure > 9)
		nExposure = 9;
	else if(nExposure < 0)
		nExposure = 0;

	pThis->setExposure(nExposure * EXPOSURE_TIME_10MS);

}

//___________________________________________________________________________
void LogitechCamCtrl::close()
{
	if (capture) {
		ksProp->Release();
		amCamCtrl->Release();
		capture->Release();
	}
}
//___________________________________________________________________________
int LogitechCamCtrl::setExposure(int exposure)
{
	// Cach 1: dung VIDCAP_CAMERACONTROL
	//return setKsPropCamCtrl(PROPSETID_VIDCAP_CAMERACONTROL, KSPROPERTY_CAMERACONTROL_EXPOSURE, exposure);

	// Cach 2: (recommend)dung _KSPROPERTY_LP1_PROPERTY
	KSPROPERTY_LP1_EXPOSURE_TIME_S ksExposure;
	ksExposure.ulExposureTime = exposure;
	ksExposure.Header.ulFlags = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	return setKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, &ksExposure, sizeof(ksExposure));

}
//___________________________________________________________________________
int LogitechCamCtrl::getExposure(int & exposure)
{
	// Cach 1: dung VIDCAP_CAMERACONTROL
	//return getKsPropCamCtrl(PROPSETID_VIDCAP_CAMERACONTROL, KSPROPERTY_CAMERACONTROL_EXPOSURE, exposure);

	// Cach 2: (recommend)dung _KSPROPERTY_LP1_PROPERTY
	HRESULT hr;
	KSPROPERTY_LP1_EXPOSURE_TIME_S ksExposure;
	ksExposure.Header.ulFlags = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	hr = getKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, &ksExposure, sizeof(ksExposure));
	exposure = ksExposure.ulExposureTime;
	return hr;

}	
//___________________________________________________________________________
int LogitechCamCtrl::setGain(int gain)
{
	m_nGain = gain;
	return setKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, KSPROPERTY_VIDEOPROCAMP_GAIN, gain);
}
//___________________________________________________________________________
int LogitechCamCtrl::getGain(int & gain)
{
	return getKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, KSPROPERTY_VIDEOPROCAMP_GAIN, gain);
}
//___________________________________________________________________________
int LogitechCamCtrl::setBrightness(int brightness)
{	
	return setKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_Brightness, brightness);
}
//___________________________________________________________________________
int LogitechCamCtrl::getBrightness(int & brightness)
{	
	return getKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_Brightness, brightness);
}
//___________________________________________________________________________
int LogitechCamCtrl::setContrast(int contrast)
{
	return setKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_Contrast, contrast);
}
//___________________________________________________________________________
int LogitechCamCtrl::getContrast(int & contrast)
{
	return getKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_Contrast, contrast);
	
}
//___________________________________________________________________________
int LogitechCamCtrl::setColorIntensity(int intensity)
{
	return setKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_Saturation, intensity);
}
//___________________________________________________________________________
int LogitechCamCtrl::getColorIntensity(int & intensity)
{
	return getKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_Saturation, intensity);
}
//___________________________________________________________________________
int LogitechCamCtrl::setAutoWB(bool on)
{
	HRESULT hr = 0;
	
	KSPROPERTY_CAMERACONTROL_S datatopass;
	memset( &datatopass, 0, sizeof(datatopass) ) ;
	if (on) {
		datatopass.Flags=KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
		datatopass.Capabilities=KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
	}
	else {
		datatopass.Flags=KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
		datatopass.Capabilities=KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	}
	hr=ksProp->Set(PROPSETID_VIDCAP_VIDEOPROCAMP, 
				   KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE, 
				   &datatopass,
				   sizeof(datatopass), 
				   &datatopass, 
				   sizeof(datatopass)); 
	
	bool test;
	if (getAutoWB(test)==S_OK){
		if (test == on)
			return S_OK;
		else
			return S_FALSE;
	}
	return hr;
}
//___________________________________________________________________________
int LogitechCamCtrl::getAutoWB(bool & on)
{
	HRESULT hr = 0;
	int tmpReturn;
	
	KSPROPERTY_CAMERACONTROL_S datatopass;
	memset( &datatopass, 0, sizeof(datatopass) ) ;
	//datatopass.Flags=KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
	//datatopass.Capabilities=KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
	hr=ksProp->Get(PROPSETID_VIDCAP_VIDEOPROCAMP, 
				   KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE, 
				   NULL,
				   0, 
				   &datatopass, 
				   sizeof(datatopass),
				   (unsigned long*)&tmpReturn); 
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Get auto white balance");
		return hr;
	}

	if (datatopass.Flags == KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)
		on = 1;
	else
		on = 0;
	return hr;
}
//___________________________________________________________________________
int LogitechCamCtrl::setWB(int wb)
{
	return setKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_WhiteBalance, wb);
}
//___________________________________________________________________________
int LogitechCamCtrl::getWB(int & wb)
{
	return getKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOPROCAMP, VideoProcAmp_WhiteBalance, wb);
}
//___________________________________________________________________________
// Anti-flicker
void LogitechCamCtrl::setAntuflicker(int mode) // mode = 0: Off, =1: NTSD-60Hz, =2: PAL-50Hz
{

}
//___________________________________________________________________________
void LogitechCamCtrl::test1()
{
}
//___________________________________________________________________________
// get range of value
#ifndef VideoProcAmp_Gain
#define VideoProcAmp_Gain VideoProcAmp_BacklightCompensation + 1
#endif
int LogitechCamCtrl::getControlInfo(ControlInfoType type, ControlInfo & info)
{
	long prop;
	bool camctrl = 1;
	switch (type){
	case CIT_PAN:		prop = CameraControl_Pan;		
		break;
	case CIT_TILT:		prop = CameraControl_Tilt;
		break;
	case CIT_ZOOM:		prop = CameraControl_Zoom;
		break;
	case CIT_EXPOSURE:	prop = CameraControl_Exposure;
		break;
	case CIT_GAIN:				prop = VideoProcAmp_Gain;			camctrl = 0;
		break;
	case CIT_BRIGHTNESS:		prop = VideoProcAmp_Brightness;		camctrl = 0;
		break;
	case CIT_CONTRAST:			prop = VideoProcAmp_Contrast;		camctrl = 0;
		break;
	case CIT_WB:				prop = VideoProcAmp_WhiteBalance;	camctrl = 0;
		break;
	case CIT_COLOR_INTENSITY:	prop = VideoProcAmp_Saturation;		camctrl = 0;
		break;
	default:
		return -1;
	}

	if (camctrl)
		return amCamCtrl->GetRange(prop, &info.min, &info.max, &info.step, &info.def, &info.flags);
	else
		return amVpa->GetRange(prop, &info.min, &info.max, &info.step, &info.def, &info.flags);
}
//___________________________________________________________________________
int LogitechCamCtrl::getKsPropCamCtrl(GUID guidPropSet, int propId, int & outVal)
{
	HRESULT hr = 0;
	int tmpReturn;
	
	KSPROPERTY_CAMERACONTROL_S datatopass;
	memset( &datatopass, 0, sizeof(datatopass) ) ;
	datatopass.Flags=KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	datatopass.Capabilities=KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	hr=ksProp->Get(guidPropSet, 
				   propId, 
				   &datatopass,
				   sizeof(datatopass), 
				   &datatopass, 
				   sizeof(datatopass),
				   (unsigned long*)&tmpReturn); 
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Get KSPROPERTY_CAMERACONTROL = %d.\n", propId);
		return hr;
	}
	outVal = datatopass.Value;
	return hr;
}
//___________________________________________________________________________
int LogitechCamCtrl::setKsPropCamCtrl(GUID guidPropSet, int propId, int inVal)
{
	HRESULT hr = 0;	
	
	KSPROPERTY_CAMERACONTROL_S datatopass;
	memset( &datatopass, 0, sizeof(datatopass) ) ;
	datatopass.Value=inVal;
	datatopass.Flags=KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	datatopass.Capabilities=KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	hr=ksProp->Set(guidPropSet, 
				   propId, 
				   &datatopass,
				   sizeof(datatopass), 
				   &datatopass, 
				   sizeof(datatopass)); 
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Set KSPROPERTY_CAMERACONTROL = %d.\n", propId);
		return hr;
	}

	return hr;
}
//___________________________________________________________________________
int LogitechCamCtrl::getKsPropLP1(GUID guidPropSet, int propId, void* ksLP1Header, int headerSize)
{
	HRESULT hr = 0;
	int tmpReturn;

	hr=ksProp->Get(guidPropSet, 
				   propId, 
				   NULL,
				   0, 
				   ksLP1Header, 
				   headerSize,
				   (unsigned long*)&tmpReturn); 
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Get KSPROPERTY_LP1 = %d.\n", propId);
		return hr;
	}
	return hr;
}
//___________________________________________________________________________
int LogitechCamCtrl::setKsPropLP1(GUID guidPropSet, int propId, void* ksLP1Header, int headerSize)
{
	HRESULT hr = 0;

	hr=ksProp->Set(guidPropSet, 
				   propId, 
				   NULL,
				   0, 
				   ksLP1Header, 
				   headerSize); 
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Set KSPROPERTY_LP1 = %d.\n", propId);
		return hr;
	}
	return hr;
}
//___________________________________________________________________________
int LogitechCamCtrl::setDigitalZoom(int zoom)
{
	KSPROPERTY_LP1_DIGITAL_ZOOM_S ksZoom;
	ksZoom.ulZoom = zoom;	
	return setKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_DIGITAL_ZOOM, &ksZoom, sizeof(ksZoom));
}
//___________________________________________________________________________
int LogitechCamCtrl::getDigitalZoom(int & zoom)
{
	HRESULT hr;
	KSPROPERTY_LP1_DIGITAL_ZOOM_S ksZoom;
	hr = getKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_DIGITAL_ZOOM, &ksZoom, sizeof(ksZoom));
	zoom = ksZoom.ulZoom;
	return hr;
}

int LogitechCamCtrl::ZoomIn()
{
	if(m_nZoom < 400)
	m_nZoom+=2;

	setDigitalZoom(m_nZoom);

	return 0;
}

int LogitechCamCtrl::ZoomOut()
{
	if(m_nZoom > 100)
	m_nZoom-=2;

	setDigitalZoom(m_nZoom);
	return 0;
}
//___________________________________________________________________________
int LogitechCamCtrl::setDigitalPan(int pan)
{
	KSPROPERTY_LP1_DIGITAL_PAN_S ksPan;
	ksPan.lPan = pan;	
	return setKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_DIGITAL_PAN, &ksPan, sizeof(ksPan));
}
//___________________________________________________________________________
int LogitechCamCtrl::getDigitalPan(int & pan)
{
	HRESULT hr;
	KSPROPERTY_LP1_DIGITAL_PAN_S ksPan;
	hr = getKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_DIGITAL_PAN, &ksPan, sizeof(ksPan));
	pan = ksPan.lPan;
	return hr;
}

void LogitechCamCtrl::setMechanicalPan(long _value)
{
	//180 to 180
	 HRESULT hr = 0;
	long flags = KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE | KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;

	hr = amCamCtrl->Set(KSPROPERTY_CAMERACONTROL_PAN, _value, flags);
	Sleep(500);
}

void LogitechCamCtrl::getMechanicalPan(long & _value)
{
	HRESULT hr = 0;
	// long flags = 0;
	int value = 0;

	//hr = amCamCtrl->Get(KSPROPERTY_CAMERACONTROL_PAN, &value, &flags);

	getKsPropCamCtrl(PROPSETID_VIDCAP_CAMERACONTROL, KSPROPERTY_CAMERACONTROL_PAN, value);

	_value = value;
	
	if(hr == S_OK)
		TRACE("Get Mechanical Pan value OK");
}

int LogitechCamCtrl::TurnRight()
{
	if(m_bMechanicalPanTilt)
	{
		if((m_nPos_X-2)< -64)
		{
			TRACE("Out of horizontal range.\n");
			return 0;
		}

		m_nPos_X -= 2;
		setMechanicalPan(-2);

		char szLog[20];
		memset(szLog, 0, 20);

		sprintf(szLog, "\nX = %d Y = %d", m_nPos_X, m_nPos_Y);
		TRACE(szLog);

		return 0;
	}

	getDigitalPan(m_nPan);

	if(m_nPan < 100)
		m_nPan+=1;

	setDigitalPan(m_nPan);
	return 0;
}

int LogitechCamCtrl::TurnLeft()
{
	if(m_bMechanicalPanTilt)
	{
		if((m_nPos_X+1)> 64)
		{
			TRACE("Out of horizontal range.\n");
			return 0;
		}
		m_nPos_X += 2;
		setMechanicalPan(2);

		char szLog[20];
		memset(szLog, 0, 20);

		sprintf(szLog, "\nX = %d Y = %d", m_nPos_X, m_nPos_Y);
		TRACE(szLog);

		return 0;
	}

	getDigitalPan(m_nPan);

	if(m_nPan > -100)
		m_nPan-=1;

	setDigitalPan(m_nPan);
	return 0;
}

//___________________________________________________________________________
int LogitechCamCtrl::setDigitalTilt(int tilt)
{
	KSPROPERTY_LP1_DIGITAL_TILT_S ksTilt;
	ksTilt.lTilt = tilt;	
	return setKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_DIGITAL_TILT, &ksTilt, sizeof(ksTilt));
}
//___________________________________________________________________________
int LogitechCamCtrl::getDigitalTilt(int & tilt)
{
	HRESULT hr;
	KSPROPERTY_LP1_DIGITAL_TILT_S ksTilt;
	hr = getKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_DIGITAL_TILT, &ksTilt, sizeof(ksTilt));
	tilt = ksTilt.lTilt;
	return hr;
}

void LogitechCamCtrl::setMechanicalTilt(long _value)
{
	HRESULT hr = 0;
	long flags = KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE | KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;

	hr = amCamCtrl->Set(KSPROPERTY_CAMERACONTROL_TILT, _value, flags);
	Sleep(500);
}

void LogitechCamCtrl::getMechanicalTilt(long & _value)
{
	 HRESULT hr = 0;
	//long flags = KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE | KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;

	 long flags = 0;

	long value = 0;

	hr = amCamCtrl->Get(KSPROPERTY_CAMERACONTROL_TILT, &value, &flags);

	_value = value;
}

int LogitechCamCtrl::TurnUp()
{
	if(m_bMechanicalPanTilt)
	{
		if((m_nPos_Y+2)> 24)
		{
			TRACE("Out of vertical range.\n");
			return 0;
		}
		m_nPos_Y += 2;
		setMechanicalTilt(2);

		char szLog[20];
		memset(szLog, 0, 20);

		sprintf(szLog, "\nX = %d Y = %d", m_nPos_X, m_nPos_Y);
		TRACE(szLog);

		return 0;
	}

	getDigitalTilt(m_nTil);

	if(m_nTil > -100)
		m_nTil-=1;

	setDigitalTilt(m_nTil);
	return 0;
}

int LogitechCamCtrl::TurnDown()
{
	if(m_bMechanicalPanTilt)
	{
		if((m_nPos_Y-2< -24))
		{
			TRACE("Out of vertical range.\n");
			return 0;
		}
		m_nPos_Y -= 2;
		setMechanicalTilt(-2);

		char szLog[20];
		memset(szLog, 0, 20);

		sprintf(szLog, "\nX = %d Y = %d", m_nPos_X, m_nPos_Y);
		TRACE(szLog);

		return 0;
	}

	getDigitalTilt(m_nTil);

	if(m_nTil < 100)
	m_nTil+=1;

	setDigitalTilt(m_nTil);
	return 0;
}

//___________________________________________________________________________
int LogitechCamCtrl::setAutoExposure(bool on)
{
	KSPROPERTY_LP1_EXPOSURE_TIME_S ksExposure;
	ksExposure.ulExposureTime = EXPOSURE_TIME_DEFAULT;
	if (on){
		ksExposure.Header.ulFlags = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
	}
	else {
		ksExposure.Header.ulFlags = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	}

	setKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, &ksExposure, sizeof(ksExposure));

	bool test;
	getAutoExposure(test);
	if (test == on)
		return S_OK;
	 return S_FALSE;

}
//___________________________________________________________________________
int LogitechCamCtrl::getAutoExposure(bool & on)
{
	HRESULT hr;
	KSPROPERTY_LP1_EXPOSURE_TIME_S ksExposure;
	hr = getKsPropLP1(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, &ksExposure, sizeof(ksExposure));
	if (ksExposure.Header.ulFlags == KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)
		on = 1;
	else
		on = 0;
	return hr;
}
//____________________________________________________________________________
int LogitechCamCtrl::setRightLight(bool on)
{
	//return setKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOCONTROL, KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, on);
	HRESULT hr = 0;
	
	KSPROPERTY_CAMERACONTROL_S datatopass;
	memset( &datatopass, 0, sizeof(datatopass) ) ;
	datatopass.Property.Set = PROPSETID_VIDCAP_CAMERACONTROL;
	datatopass.Property.Id = KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY;
	datatopass.Value = on ? 1 : 0;
	hr = ksProp->Set(
					PROPSETID_VIDCAP_CAMERACONTROL,
					KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY,
					PKSPROPERTY(&datatopass) + 1,
					sizeof(datatopass) - sizeof(KSPROPERTY),
					&datatopass,
					sizeof(datatopass));
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Set KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY");
		return hr;
	}
	on = (bool)datatopass.Value;
	return hr;
}
//____________________________________________________________________________
int LogitechCamCtrl::getRightLight(bool & on)
{
	//return setKsPropCamCtrl(PROPSETID_VIDCAP_VIDEOCONTROL, KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, on);
	HRESULT hr = 0;
	int tmpReturn;
	
	KSPROPERTY_CAMERACONTROL_S datatopass;
	memset( &datatopass, 0, sizeof(datatopass) ) ;
	datatopass.Property.Set = PROPSETID_VIDCAP_CAMERACONTROL;
	datatopass.Property.Id = KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY;
	//datatopass.Value = on ? 1 : 0;
	hr = ksProp->Get(
					PROPSETID_VIDCAP_CAMERACONTROL,
					KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY,
					PKSPROPERTY(&datatopass) + 1,
					sizeof(datatopass) - sizeof(KSPROPERTY),
					&datatopass,
					sizeof(datatopass),
					(unsigned long*)&tmpReturn);
	if(hr != S_OK)
	{
		fprintf(stderr, "ERROR: Get KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY");
		return hr;
	}
	on = (bool)datatopass.Value;
	return hr;
}

int LogitechCamCtrl::ChangeBrightness(bool bAdd)
{	
	if(bAdd)
	{
		if(m_nGain < 255)
			m_nGain++;

		setGain(m_nGain);	
	}
	else
	{
		if(m_nGain > 0)
			m_nGain--;

		setGain(m_nGain);	
	}

	return 0;
}