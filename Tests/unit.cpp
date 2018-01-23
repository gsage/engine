#include "GsageDefinitions.h"
#include <gtest/gtest.h>
#include "Logger.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#include "CoreFoundation/CoreFoundation.h"
#endif

#if GSAGE_PLATFORM == GSAGE_WIN32
#define WIN32_LEAN_AND_MEAN
#include "WIN32/WindowsIncludes.h"
#include <Shellapi.h>
#include <string>

void fetchCmdArgs(int* argc, char*** argv) {
    // init results
    *argc = 0;

    // prepare extraction
    char* winCmd = GetCommandLine();
    int index = 0;
    bool newOption = true;
    // use static so converted command line can be
    // accessed from outside this function
    static std::vector<char*> argVector;

    // walk over the command line and convert it to argv
    while(winCmd[index] != 0){
        if (winCmd[index] == ' ') {
            // terminate option string
            winCmd[index] = 0;
            newOption = true;

        } else  {
            if(newOption){
                argVector.push_back(&winCmd[index]);
                (*argc)++;
            }
            newOption = false;
        }
        index++;
    }

    // elements inside the vector are guaranteed to be continous
    *argv = &argVector[0];
}

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
  char** argv;
  fetchCmdArgs(&argc, &argv);
#endif
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
