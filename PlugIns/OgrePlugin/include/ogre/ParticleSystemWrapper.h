/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

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

#ifndef _ParticleSystemWrapper_H_
#define _ParticleSystemWrapper_H_

#include <OgreParticleSystem.h>
#include "ogre/OgreObject.h"

namespace Gsage {

  class ParticleSystemWrapper : public OgreObject
  {
    public:
      static const std::string TYPE;

      ParticleSystemWrapper();
      virtual ~ParticleSystemWrapper();

      /**
       * Override default behavior for read
       */
      bool read(const DataProxy& dict);

      /**
       * Set particle system template. Note that it requires particle system recreation
       * @param templateName Template name
       */
      void setTemplate(const std::string& templateName);

      /**
       * Get particle system template name
       */
      const std::string& getTemplate() const;

      /**
       * Manualy create particle in the system
       * @param index Index of the emitter
       * @param nodeId Emit particle from the specified node
       */
      void createParticle(unsigned short index, const std::string& nodeId);

      /**
       * Manually create text particle in the system
       * Particle system type should be "manual_text" to use this method. Otherwise it will just generate a particle
       * @param index Index of the emitter
       * @param nodeId Emit particle from the specified node
       * @param value Text value
       */
      void createParticle(unsigned short index, const std::string& nodeId, const std::string& value);

      /**
       * Manually create text particle in the system
       * Particle system type should be "manual_text" to use this method. Otherwise it will just generate a particle
       * @param index Index of the emitter
       * @param nodeId Emit particle from the specified node
       * @param value Text value
       * @param rotate the emitter in the following direction
       */
      void createParticle(unsigned short index, const std::string& nodeId, const std::string& value, const Ogre::Quaternion& rotation);
    private:

      void createParticle(unsigned short index, Ogre::ParticleVisualData* data, const std::string& nodeId, const Ogre::Quaternion& rotation);

      std::string mTemplate;
      Ogre::ParticleSystem* mParticleSystem;
  };
}
#endif
