#include "DTypes.h"
#include <string>
#include <vector>

class DMacPhone {
public:
    static DVoid PrintDefault();
    static DVoid PrintAll();

public:
    static DUInt32 GetDefaultDeviceID();
    static std::vector<DUInt32> GetAllDeviceIDs();
    static std::string GetDeviceName(DUInt32 deviceID);
    static DUInt32 GetDeviceID(std::string deviceName);
    static DUInt32 GetDeviceType(DUInt32 deviceID);
    static std::string GetTypeString(DUInt32 deviceID);
};
