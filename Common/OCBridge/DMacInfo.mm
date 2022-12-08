//
//  MacInfo.mm
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 04/30/2021 for Dream.
//  Copyright 2021. All rights reserved.
//


#import "MacInfo.h"
#import <CoreServices/CoreServices.h>

DStringA MacInfo::getOSVersion() {
    SInt32 major, minor, bugfix;
    Gestalt(gestaltSystemVersionMajor, &major);
    Gestalt(gestaltSystemVersionMinor, &minor);
    Gestalt(gestaltSystemVersionBugFix, &bugfix);
    DStringA strRet;
    strRet.Format("%d.%d.%d", major, minor, bugfix);
    return strRet;
}
