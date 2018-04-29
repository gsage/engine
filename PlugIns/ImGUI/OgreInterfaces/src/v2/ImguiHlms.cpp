/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "v2/ImguiHlms.h"

#include <OgreHlmsUnlitDatablock.h>

namespace Gsage {

  ImguiHlms::ImguiHlms(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders)
    : Ogre::HlmsUnlit(dataFolder, libraryFolders)
  {
  }

  ImguiHlms::ImguiHlms(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders,
          Ogre::HlmsTypes type, const Ogre::String& typeName)
    : Ogre::HlmsUnlit(dataFolder, libraryFolders, type, typeName)
  {
  }

  ImguiHlms::~ImguiHlms()
  {
  }

  Ogre::uint32 ImguiHlms::fillBuffersForImgui(
      const Ogre::HlmsCache* cache,
      const Ogre::QueuedRenderable& queuedRenderable,
      bool casterPass,
      Ogre::uint32 baseVertex,
      Ogre::uint32 lastCacheHash,
      Ogre::CommandBuffer* commandBuffer
  )
  {
    return 0;
  }

  void ImguiHlms::getDefaultPaths(Ogre::String& outDataFolderPath, Ogre::StringVector& outLibraryFoldersPaths)
  {

  }

  Ogre::HlmsDatablock* ImguiHlms::createDatablockImpl(Ogre::IdString datablockName,
          const Ogre::HlmsMacroblock *macroblock,
          const Ogre::HlmsBlendblock *blendblock,
          const Ogre::HlmsParamVec &paramVec)
  {
    return OGRE_NEW Ogre::HlmsUnlitDatablock(datablockName, this, macroblock, blendblock, paramVec);
  }


}
