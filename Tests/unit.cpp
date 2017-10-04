#include "GsageDefinitions.h"
#include <gtest/gtest.h>
#include "Logger.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#include "CoreFoundation/CoreFoundation.h"
#endif

#if GSAGE_PLATFORM == GSAGE_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <Shellapi.h>
#include <string>
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
#if GSAGE_PLATFORM == GSAGE_APPLE
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
  char path[PATH_MAX];
  if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)) // Error: expected unqualified-id before 'if'
  {
    return 1;
  }
  CFRelease(resourcesURL); // error: expected constructor, destructor or type conversion before '(' token
  std::stringstream p;
  p << path << "/Contents";
  chdir(p.str().c_str()); // error: expected constructor, destructor or type conversion before '(' token
#endif
#if GSAGE_PLATFORM == GSAGE_WIN32
  int argc = 0;
  char* argv[1] = {0};
#endif
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
