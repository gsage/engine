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

#ifndef _StatsComponent_H_
#define _StatsComponent_H_

#include "EventDispatcher.h"
#include "Component.h"

namespace Gsage {

  /**
   * Event that signals that some stat was updated
   */
  class StatEvent : public Event
  {
    public:
      static const std::string STAT_CHANGE;
      StatEvent(const std::string& type, const std::string& statId);

      /**
       * Get updated stat id
       */
      const std::string& getId() const;
    private:
      std::string mStatId;
  };

  class StatsComponent : public EntityComponent, public EventDispatcher
  {
    public:
      static const std::string SYSTEM;

      StatsComponent();
      virtual ~StatsComponent();
      /**
       * Chech if component has the specified stat
       *
       * @param id Stat identifier
       */
      bool hasStat(const std::string& id);
      /**
       * Get stat by id
       *
       * @param id Stat identifier
       */
      template<typename T>
      const T getStat(const std::string& id)
      {
        return mStats.get<T>(id).first;
      }
      /**
       * Get stat by id
       *
       * @param id Stat identifier
       * @param defaultValue Return this value if not found
       */
      template<typename T>
      const T getStat(const std::string& id, const T& defaultValue)
      {
        return mStats.get(id, defaultValue);
      }
      /**
       * Set stat by id
       *
       * @param id Stat identifier
       * @param value Stat value
       */
      template<typename T>
      void setStat(const std::string& id, const T& value)
      {
        if(mStats.count(id) != 0 && mStats.get<T>(id).first == value)
          return;

        mStats.put(id, value);
        fireEvent(StatEvent(StatEvent::STAT_CHANGE, id));
      }

      /**
       * Overrides default behavior of the decoding
       * @param dict DataProxy with all stats
       */
      bool read(const DataProxy& dict);

      /**
       * Overrides default behavior of the encoding
       *
       * @param dict DataProxy to write to
       */
      bool dump(DataProxy& dict);

      /**
       * Increase numeric types, shortcut function
       * If stat is not set it will be set to 0 and then increased
       *
       * @param key Stat key
       * @param n Value to increase
       * @returns Modified value
       */
      const float increase(const std::string& key, const float& n);

      /**
       * Get data
       */
      DataProxy& data();
    private:
      DataProxy mStats;
  };
}

#endif
