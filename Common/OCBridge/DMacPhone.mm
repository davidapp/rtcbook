#import "DMacPhone.h"
#import <CoreAudio/CoreAudio.h>
#import <CoreFoundation/CoreFoundation.h>

BOOL DeviceHasBuffersInScope(AudioObjectID deviceID, AudioObjectPropertyScope scope)
{
    AudioObjectPropertyAddress propertyAddress = {kAudioDevicePropertyStreamConfiguration, scope, kAudioObjectPropertyElementWildcard};
    UInt32 dataSize = 0;
    AudioObjectGetPropertyDataSize(deviceID, &propertyAddress, 0, NULL, &dataSize);
    AudioBufferList *bufferList = (AudioBufferList *)malloc(dataSize);
    AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, NULL, &dataSize, bufferList);
    BOOL supportsScope = bufferList->mNumberBuffers > 0;
    free(bufferList);
    return supportsScope;
}

DVoid DMacPhone::PrintDefault()
{
    AudioDeviceID defaultDeviceID = GetDefaultDeviceID();
    printf("defaultDeviceID = %d\n", defaultDeviceID);
    std::string defaultDeviceName = DMacPhone::GetDeviceName(defaultDeviceID);
    printf("defaultDeviceName = %s\n", defaultDeviceName.c_str());
}

DVoid DMacPhone::PrintAll()
{
    std::vector<DUInt32> ids = DMacPhone::GetAllDeviceIDs();
    for (UInt32 i = 0;i < ids.size();i++) {
        printf("devicesID[%d] = %d, %s[%s:%d][%s]\n", i, ids[i], GetDeviceName(ids[i]).c_str(),
            GetTypeString(GetDeviceType(ids[i])).c_str(),
            GetDeviceID(GetDeviceName(ids[i])),
            DeviceHasBuffersInScope(ids[i], kAudioObjectPropertyScopeInput)?"input":"output");
    }
}

DUInt32 DMacPhone::GetDefaultDeviceID()
{
    AudioObjectPropertyAddress propertyAddressDefault = {kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    AudioDeviceID defaultDeviceID = kAudioDeviceUnknown;
    UInt32 uintSize = sizeof(UInt32);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddressDefault, 0, NULL, &uintSize, &defaultDeviceID);
    return defaultDeviceID;
}

std::vector<DUInt32> DMacPhone::GetAllDeviceIDs()
{
    AudioObjectPropertyAddress propertyAddress = {kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &size);
    UInt32 numberDevices = size / sizeof(AudioDeviceID);

    AudioDeviceID * pDevicesID = new AudioDeviceID[numberDevices];
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &size, pDevicesID);

    std::vector<DUInt32> res;
    for (UInt32 i = 0;i < numberDevices;i++) {
        res.push_back((DUInt32)pDevicesID[i]);
    }
    
    delete []pDevicesID;
    return res;
}

std::string DMacPhone::GetDeviceName(DUInt32 deviceID)
{
    // 将 AudioDeviceID deviceID 转换为字符串 std::string device_name
    AudioObjectPropertyAddress property_address = {kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    
    Boolean bHasName = AudioObjectHasProperty(deviceID, &property_address);
    if (bHasName == NO) return "";
    
    CFStringRef audio_device_uid = NULL;
    UInt32 size = sizeof(audio_device_uid);
    AudioObjectGetPropertyData(deviceID, &property_address, 0, nullptr, &size, &audio_device_uid);

    if (audio_device_uid == NULL) return "";
    
    // 读取字符串到 string
    CFIndex length = CFStringGetLength(audio_device_uid);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    std::string device_name;
    device_name.resize(maxSize);
    CFStringGetCString(audio_device_uid, (char*)device_name.c_str(), maxSize, kCFStringEncodingUTF8);

    // 释放得到的字符串
    CFRelease(audio_device_uid);
    return device_name;
}

DUInt32 DMacPhone::GetDeviceID(std::string deviceName)
{
    AudioDeviceID deviceID = kAudioDeviceUnknown;
    CFStringRef inUID = CFStringCreateWithCString(kCFAllocatorDefault, deviceName.c_str(), kCFStringEncodingUTF8);

    AudioValueTranslation translation;
    translation.mInputData = &inUID;
    translation.mInputDataSize = sizeof(CFStringRef);
    translation.mOutputData = &deviceID;
    translation.mOutputDataSize = sizeof(AudioDeviceID);
    UInt32 size = sizeof(translation);
    AudioObjectPropertyAddress property_address = {kAudioHardwarePropertyDeviceForUID,  kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &property_address, 0, nullptr, &size, &translation);
    
    CFRelease(inUID);
    return deviceID;
}

DUInt32 DMacPhone::GetDeviceType(DUInt32 deviceID)
{
    DUInt32 transportType = kAudioDeviceTransportTypeUnknown;
    AudioObjectPropertyAddress property_address = {kAudioDevicePropertyTransportType, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    UInt32 size = sizeof(transportType);
    AudioObjectGetPropertyData(deviceID, &property_address, 0, nullptr, &size, &transportType);
    return transportType;
}

std::string DMacPhone::GetTypeString(DUInt32 deviceID)
{
    switch (deviceID) {
        case kAudioDeviceTransportTypeBuiltIn:
            return "BuiltIn";
        case kAudioDeviceTransportTypeVirtual:
            return "Virtual";
        case kAudioDeviceTransportTypePCI:
            return "PCI";
        case kAudioDeviceTransportTypeUSB:
            return "USB";
        case kAudioDeviceTransportTypeBluetooth:
        case kAudioDeviceTransportTypeBluetoothLE:
            return "Bluetooth";
        case kAudioDeviceTransportTypeHDMI:
            return "HDMI";
        case kAudioDeviceTransportTypeDisplayPort:
            return "DisplayPort";
        case kAudioDeviceTransportTypeAirPlay:
            return "AirPlay";
    }
    return "Unknown";
}


