#ifndef _PUSystemWrapper_H_
#define _PUSystemWrapper_H_

#include "ogre/OgreObject.h"
#include "ParticleUniverseSystem.h"

namespace Gsage
{
  class PUSystemWrapper : public OgreObject
  {
    public:
      static const std::string TYPE;

      PUSystemWrapper();
      virtual ~PUSystemWrapper();

      /**
       * Override default read behavior
       */
      bool read(const DataProxy& node);

      /**
       * Create particle system using previosly set name
       * @param template Particle system template file
       */
      void setTemplateName(const std::string& templateName);

      /**
       * Get particle system template
       */
      const std::string& getTemplateName() const;

      /**
       * Start particle system
       */
      void start();

      /**
       * Stop particle system
       */
      void stop();

      /**
       * Pause particle system
       */
      void pause();

      /**
       * Pause particle system
       * @param time Time to pause
       */
      void pause(const float& time);

      /**
       * Resume particle system
       */
      void resume();

      /**
       * Destroy particle system
       */
      void destroy();
    private:
      std::string mName;
      bool mStartOnCreate;

      ParticleUniverse::ParticleSystem* mParticleSystem;
  };
}
#endif
