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

#ifndef _LightWrapper_H_
#define _LightWrapper_H_

#include "ogre/OgreObject.h"
#include <OgreLight.h>

namespace Gsage {
  class LightWrapper : public OgreObject
  {
    public:
      static const std::string TYPE;

      LightWrapper();
      /**
       * Create light instance
       * @param light name
       */
      void create(const std::string& name);

      /**
       * Get light name
       */
      const std::string& getName() const;

      /**
       * Set light type
       * @param type point/directional/spot
       */
      void setType(const std::string& type);

      /**
       * Get light type
       */
      std::string getType();

      /**
       * Set light position
       * @param position Ogre::Vector3 position
       */
      void setPosition(const Ogre::Vector3& position);

      /**
       * Get light position
       */
      const Ogre::Vector3& getPosition() const;

      /**
       * Set light diffuse colour
       * @param value ColourValue
       */
      void setDiffuseColour(const Ogre::ColourValue& value);

      /**
       * Get light diffuse colour
       */
      const Ogre::ColourValue& getDiffuseColour() const;

      /**
       * Set light specular colour
       * @param value ColourValue
       */
      void setSpecularColour(const Ogre::ColourValue& value);

      /**
       * Get light specular colour
       */
      const Ogre::ColourValue& getSpecularColour() const;

      /**
       * Set light direction
       * @param direction Vector3 direction, non relevant for point light
       */
      void setDirection(const Ogre::Vector3& value);

      /**
       * Get light direction
       */
      const Ogre::Vector3& getDirection() const;

      /**
       * Cast shadows from this light
       * @param value Cast shadows
       */
      void setCastShadows(const bool& value);

      /**
       * Get cast shadows
       */
      bool getCastShadows();
    private:
      Ogre::Light* mLight;

      /**
       * Get ogre internal light type from string
       * @param type point/directional/spotlight
       */
      Ogre::Light::LightTypes mapType(const std::string& type);

      /**
       * Convert Ogre light type to string type
       * @param ogre Ogre::Light::LightTypes
       */
      std::string mapType(const Ogre::Light::LightTypes& type);
  };
}
#endif
