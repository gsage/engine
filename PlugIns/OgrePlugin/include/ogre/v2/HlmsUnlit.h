#ifndef _HlmsUnlit_H_
#define _HlmsUnlit_H_

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

#include <OgreHlmsUnlit.h>
#include "HlmsUnlitDatablock.h"

namespace Gsage {
  class HlmsUnlit;

  /**
   * Extends Ogre standard HlmsUnlit
   */
  class HlmsUnlit : public Ogre::HlmsUnlit
  {
    public:
      HlmsUnlit(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders);
      HlmsUnlit(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders,
          Ogre::HlmsTypes type, const Ogre::String& typeName);
      virtual ~HlmsUnlit();

      virtual Ogre::uint32 fillBuffersForV2(
          const Ogre::HlmsCache* cache,
          const Ogre::QueuedRenderable& queuedRenderable,
          bool casterPass,
          Ogre::uint32 lastCacheHash,
          Ogre::CommandBuffer* commandBuffer
      );

      static void getDefaultPaths(Ogre::String& outDataFolderPath, Ogre::StringVector& outLibraryFoldersPaths);

      /**
       * Make HlmsUnlit use the same projection matrix for all renderables which are using datablock
       * with the specified name.
       *
       * @param name Datablock name
       * @param matrix Ogre::Matrix4 to use
       */
      void setUseCustomProjectionMatrix(Ogre::IdString name, const Ogre::Matrix4& matrix);
    protected:
      /*virtual const HlmsCache* createShaderCacheEntry(uint32 renderableHash,
          const HlmsCache &passCache,
          uint32 finalHash,
          const QueuedRenderable &queuedRenderable);

      virtual void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces );*/

      virtual Ogre::HlmsDatablock* createDatablockImpl(Ogre::IdString datablockName,
          const Ogre::HlmsMacroblock *macroblock,
          const Ogre::HlmsBlendblock *blendblock,
          const Ogre::HlmsParamVec &paramVec);

      typedef std::map<Ogre::IdString, Ogre::Matrix4> CustomProjectionMatrices;

      CustomProjectionMatrices mCustomProjectionMatrices;
  };
}

#endif
