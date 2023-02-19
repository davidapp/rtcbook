#import <Foundation/Foundation.h>
#import "OCPath.h"

const char* GetSearchPathDirectory(NSSearchPathDirectory type)
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(type, NSUserDomainMask, YES);
    if (paths) {
        return [paths[0] UTF8String];
    }else
    {
        return nullptr;
    }
}

const char* getHomeDir()
{
    NSString *string = NSHomeDirectory();
    return [string UTF8String];
}

const char* getDocDir()
{
    return GetSearchPathDirectory(NSDocumentDirectory);
    
}

const char* getCacheDir()
{
    return GetSearchPathDirectory(NSCachesDirectory);
}

const char* getTempDir()
{
    NSString *string = NSTemporaryDirectory();
    return [string UTF8String];
}

const char* getMoivesDir()
{
    return GetSearchPathDirectory(NSMoviesDirectory);
}

const char* getMusicDir()
{
   return GetSearchPathDirectory(NSMusicDirectory);
}

const char* getPicDir()
{
    return GetSearchPathDirectory(NSPicturesDirectory);
}

const char* getDownloadDir()
{
    return GetSearchPathDirectory(NSDownloadsDirectory);
}
