#include "HelloJNI.h"
#include <android/log.h>
#include "Base/DXP.h"

extern "C" JNIEXPORT void JNICALL Java_com_example_helloworld_1android_HelloJNI_HelloNative(JNIEnv *, jobject)
{
    __android_log_print(ANDROID_LOG_INFO, "Hello", " from JNI");

    std::string strHello = std::string("Hello ") + DXP::GetOSName() + D_LINES;
    DXP::Print(strHello);
}
