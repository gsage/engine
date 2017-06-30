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

#include "components/StatsComponent.h"

namespace Gsage {

  const std::string StatEvent::STAT_CHANGE = "statChange";

  StatEvent::StatEvent(const std::string& type, const std::string& statId)
    : Event(type)
    , mStatId(statId)
  {
  }

  const std::string& StatEvent::getId() const
  {
    return mStatId;
  }

  const std::string StatsComponent::SYSTEM = "combat";

  StatsComponent::StatsComponent()
    : mStats(DataProxy::create(DataWrapper::JSON_OBJECT))
  {
  }

  StatsComponent::~StatsComponent()
  {

  }

  bool StatsComponent::hasStat(const std::string& id)
  {
    return mStats.count(id) != 0;
  }

  bool StatsComponent::read(const DataProxy& dict)
  {
    dict.dump(mStats, DataProxy::ForceCopy);
    return true;
  }

  DataProxy& StatsComponent::data()
  {
    return mStats;
  }

  bool StatsComponent::dump(DataProxy& dict)
  {
    mStats.dump(dict, DataProxy::ForceCopy);
    return true;
  }

  const float StatsComponent::increase(const std::string& key, const float& n)
  {
    float newVal = getStat(key, 0.0f) + n;
    setStat(key, newVal);
    return newVal;
  }

}
