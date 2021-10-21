//
//  WinDSCamera.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 05/03/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include "Data/DData.h"
#include "Data/DArray.h"
#include "Codec/DColorSpace.h"
#include <dshow.h>

enum {
	DCAMERA_STRINGA_DEVICE_NAME = 0,
	DCAMERA_STRINGA_DEVICE_ID = 1,
	DCAMERA_STRINGA_PRODUCT_ID = 2,
	DCAMERA_UINT64_DEVICE_FILTER = 3,
};

enum {
	DPFINFO_UINT32_OS_PLATFORM = 0, // 1:Windows, 2:Mac
	DPFINFO_UINT32_WIDTH = 1,
	DPFINFO_UINT32_HEIGHT = 2,
	DPFINFO_UINT32_FRAMERATE = 3,
	DPFINFO_BOOL_IS_MAX_FRAMERATE = 4,
	DPFINFO_UINT32_PIXEL_FORMAT = 5
};

class DAPI WinDSCamera
{
public:
	// 初始化COM，并创建 ICreateDevEnum 接口
	static DBool Init();
	// 销毁 ICreateDevEnum 和 IEnumMoniker 接口
	static DVoid UnInit();

public:
	// 有哪些采集设备，每次重新创建一个 IEnumMoniker 接口来枚举
	static DArray* GetDevices();

	// 指定的设备，有哪些采集能力
	static DArray* GetDeviceCaps(DStr deviceID);
	
	// 拿到对应采集设备的 Filter
	static IBaseFilter* GetDeviceFilter(DCStr deviceID);

	// 显示对应设备的设置窗口
	static DBool ShowSettingDialog(DCStr deviceID, DVoid* parentWindow, DUInt32 positionX, DUInt32 positionY);

private:
	// Helper Functions
	static DStringA GetProductIdFromPath(DStringA& path);
	static IPin* GetInputPin(IBaseFilter* filter);
	static IPin* GetOutputPin(IBaseFilter* filter, REFGUID Category);
	static DBool PinMatchesCategory(IPin* pPin, REFGUID Category);
	static DVoid FreeMediaType(AM_MEDIA_TYPE& mt);
	static DUInt64 GetMaxOfFrameArray(DUInt64* maxFps, DUInt32 size);

#if defined(D_INCLUDE_TEST) && (D_INCLUDE_TEST==1)
public:
	static DVoid TestDevices();
#endif
};
