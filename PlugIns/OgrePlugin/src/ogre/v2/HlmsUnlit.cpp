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

#include "ogre/v2/HlmsUnlit.h"
#include <Vao/OgreBufferPacked.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreTexBufferPacked.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCbTexture.h>
#include <OgreHlmsListener.h>

namespace Gsage {

  HlmsUnlit::HlmsUnlit(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders)
    : Ogre::HlmsUnlit(dataFolder, libraryFolders)
  {
  }

  HlmsUnlit::HlmsUnlit(Ogre::Archive* dataFolder, Ogre::ArchiveVec* libraryFolders,
          Ogre::HlmsTypes type, const Ogre::String& typeName)
    : Ogre::HlmsUnlit(dataFolder, libraryFolders, type, typeName)
  {
  }

  HlmsUnlit::~HlmsUnlit()
  {
  }

  Ogre::uint32 HlmsUnlit::fillBuffersForV2(
      const Ogre::HlmsCache* cache,
      const Ogre::QueuedRenderable& queuedRenderable,
      bool casterPass,
      Ogre::uint32 lastCacheHash,
      Ogre::CommandBuffer* commandBuffer
  )
  {
    HlmsUnlitDatablock* datablock = static_cast<HlmsUnlitDatablock*>(queuedRenderable.renderable->getDatablock());
    if(mCustomProjectionMatrices.count(datablock->getName()) != 0) {
      const HlmsUnlitDatablock *datablock = static_cast<const HlmsUnlitDatablock*>(
          queuedRenderable.renderable->getDatablock() );

      if(OGRE_EXTRACT_HLMS_TYPE_FROM_CACHE_HASH(lastCacheHash) != mType)
      {
        //We changed HlmsType, rebind the shared textures.
        mLastTextureHash = 0;
        mLastBoundPool = 0;

        //layout(binding = 0) uniform PassBuffer {} pass
        Ogre::ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer-1];
        *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(
            Ogre::VertexShader,
            0, passBuffer, 0,
            passBuffer->
            getTotalSizeBytes() );
        *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(
            Ogre::PixelShader,
            0, passBuffer, 0,
            passBuffer->
            getTotalSizeBytes());

        //layout(binding = 2) uniform InstanceBuffer {} instance
        if( mCurrentConstBuffer < mConstBuffers.size() &&
            (size_t)((mCurrentMappedConstBuffer - mStartMappedConstBuffer) + 4) <=
            mCurrentConstBufferSize )
        {
          *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
            Ogre::CbShaderBuffer(Ogre::VertexShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
          *commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
            Ogre::CbShaderBuffer(Ogre::PixelShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
        }

        rebindTexBuffer( commandBuffer );

        mListener->hlmsTypeChanged( casterPass, commandBuffer, datablock );
      }

      //Don't bind the material buffer on caster passes (important to keep
      //MDI & auto-instancing running on shadow map passes)
      if( mLastBoundPool != datablock->getAssignedPool() && !casterPass )
      {
        //layout(binding = 1) uniform MaterialBuf {} materialArray
        const Ogre::ConstBufferPool::BufferPool *newPool = datablock->getAssignedPool();
        *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(
            Ogre::PixelShader,
            1, newPool->materialBuffer, 0,
            newPool->materialBuffer->
            getTotalSizeBytes() );
        if( newPool->extraBuffer )
        {
          Ogre::TexBufferPacked *extraBuffer = static_cast<Ogre::TexBufferPacked*>( newPool->extraBuffer );
          *commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(
              Ogre::VertexShader, 1,
              extraBuffer, 0,
              extraBuffer->
              getTotalSizeBytes() );
        }

        mLastBoundPool = newPool;
      }

      Ogre::uint32 * RESTRICT_ALIAS currentMappedConstBuffer    = mCurrentMappedConstBuffer;
      float * RESTRICT_ALIAS currentMappedTexBuffer       = mCurrentMappedTexBuffer;

      bool exceedsConstBuffer = (size_t)((currentMappedConstBuffer - mStartMappedConstBuffer) + 4) >
        mCurrentConstBufferSize;

      const size_t minimumTexBufferSize = 16;
      bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
        minimumTexBufferSize >= mCurrentTexBufferSize;

      if( exceedsConstBuffer || exceedsTexBuffer )
      {
        currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

        if( exceedsTexBuffer )
          mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
        else
          rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

        currentMappedTexBuffer = mCurrentMappedTexBuffer;
      }

      //---------------------------------------------------------------------------
      //                          ---- VERTEX SHADER ----
      //---------------------------------------------------------------------------
      //uint materialIdx[]
      *currentMappedConstBuffer = datablock->getAssignedSlot();
      *reinterpret_cast<float * RESTRICT_ALIAS>( currentMappedConstBuffer+1 ) = datablock->
        mShadowConstantBias;
      *(currentMappedConstBuffer+2) = false;
      currentMappedConstBuffer += 4;

      //mat4 worldViewProj
      Ogre::Matrix4 tmp = mCustomProjectionMatrices[datablock->getName()];
#if !OGRE_DOUBLE_PRECISION
      memcpy(currentMappedTexBuffer, &tmp, sizeof(Ogre::Matrix4));
      currentMappedTexBuffer += 16;
#else
      for( int y = 0; y < 4; ++y )
      {
        for( int x = 0; x < 4; ++x )
        {
          *currentMappedTexBuffer++ = tmp[y][x];
        }
      }
#endif

      //---------------------------------------------------------------------------
      //                          ---- PIXEL SHADER ----
      //---------------------------------------------------------------------------

      if( !casterPass )
      {
        if( datablock->mTextureHash != mLastTextureHash )
        {
          //Rebind textures
          size_t texUnit = 2;

          Ogre::UnlitBakedTextureArray::const_iterator itor = datablock->mBakedTextures.begin();
          Ogre::UnlitBakedTextureArray::const_iterator end  = datablock->mBakedTextures.end();

          while( itor != end )
          {
            *commandBuffer->addCommand<Ogre::CbTexture>() =
              Ogre::CbTexture( texUnit++, true, itor->texture.get(), itor->samplerBlock );
            ++itor;
          }

          *commandBuffer->addCommand<Ogre::CbTextureDisableFrom>() = Ogre::CbTextureDisableFrom( texUnit );

          mLastTextureHash = datablock->mTextureHash;
        }
      }

      mCurrentMappedConstBuffer   = currentMappedConstBuffer;
      mCurrentMappedTexBuffer     = currentMappedTexBuffer;

      return ((mCurrentMappedConstBuffer - mStartMappedConstBuffer) >> 2) - 1;
    }

    return Ogre::HlmsUnlit::fillBuffersForV2(
        cache,
        queuedRenderable,
        casterPass,
        lastCacheHash,
        commandBuffer
    );
  }

  void HlmsUnlit::getDefaultPaths(Ogre::String& outDataFolderPath, Ogre::StringVector& outLibraryFoldersPaths)
  {
    return Ogre::HlmsUnlit::getDefaultPaths(outDataFolderPath, outLibraryFoldersPaths);
  }

  void HlmsUnlit::setUseCustomProjectionMatrix(Ogre::IdString name, const Ogre::Matrix4& matrix)
  {
    mCustomProjectionMatrices[name] = matrix;
  }

  Ogre::HlmsDatablock* HlmsUnlit::createDatablockImpl(Ogre::IdString datablockName,
          const Ogre::HlmsMacroblock *macroblock,
          const Ogre::HlmsBlendblock *blendblock,
          const Ogre::HlmsParamVec &paramVec)
  {
    return OGRE_NEW Gsage::HlmsUnlitDatablock(datablockName, this, macroblock, blendblock, paramVec);
  }

}
