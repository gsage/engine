#include <OgrePlatform.h>
#include <gtest/gtest.h>
#include "Logger.h"

_INITIALIZE_EASYLOGGINGPP

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
