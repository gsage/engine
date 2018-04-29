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

#include "RecastNavigationPlugin.h"
#include "GsageFacade.h"
#include "RecastEvent.h"

#include "components/RecastNavigationComponent.h"
#include "systems/RecastNavigationSystem.h"

namespace Gsage {

  const std::string PLUGIN_NAME = "RecastNavigation";

  RecastNavigationPlugin::RecastNavigationPlugin()
  {
  }

  RecastNavigationPlugin::~RecastNavigationPlugin()
  {
  }

  const std::string& RecastNavigationPlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  void RecastNavigationPlugin::setupLuaBindings() {
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view lua(mLuaInterface->getState());

      lua.new_usertype<RecastNavigationSystem>("RecastNavigationSystem",
          sol::base_classes, sol::bases<EngineSystem>(),
          "defaultOptions", sol::property(&RecastNavigationSystem::getDefaultOptions),
          "rebuildNavMesh", &RecastNavigationSystem::rebuild,
          "findNearestPointOnNavmesh", &RecastNavigationSystem::findNearestPointOnNavmesh,
          "findPath", [](RecastNavigationSystem* self, Gsage::Vector3 start, Gsage::Vector3 end, sol::this_state s) -> sol::object {
            auto path = self->findPath(start, end);
            if(path == nullptr) {
              return sol::lua_nil;
            }
            sol::state_view lua(s);
            sol::table res = lua.create_table();
            DataProxy t = DataProxy::wrap(res);
            path->dump(t);
            return res;
          },
          "getNavMeshRawPoints", [](RecastNavigationSystem* self, sol::this_state s){
            sol::state_view lua(s);
            sol::table t = lua.create_table();
            auto points = self->getNavMeshRawPoints();
            for(int i = 0; i < points.size(); ++i) {
              t[i + 1] = points[i];
            }
            return t;
          }
      );

      // Components

      lua.new_usertype<RecastNavigationComponent>("RecastNavigationComponent",
          sol::base_classes, sol::bases<Reflection>(),
          "props", sol::property(&RecastNavigationComponent::getProps, &RecastNavigationComponent::setProps),
          "go", sol::overload(
            (void(RecastNavigationComponent::*)(const Gsage::Vector3&))&RecastNavigationComponent::setTarget,
            (void(RecastNavigationComponent::*)(float, float, float))&RecastNavigationComponent::setTarget
          )
      );

      lua["Engine"]["navigation"] = &Engine::getSystem<RecastNavigationSystem>;
      lua["Entity"]["navigation"] = &Entity::getComponent<RecastNavigationComponent>;

      // register recast events
      mLuaInterface->registerEvent<RecastEvent>("RecastEvent",
        "onRecastEvent",
        sol::base_classes, sol::bases<Event>(),
        "NAVIGATION_START", sol::var(RecastEvent::NAVIGATION_START),
        "entityID", &RecastEvent::mEntityID,
        "path", &RecastEvent::mPath
      );
      LOG(INFO) << "Registered lua bindings for " << PLUGIN_NAME;
    }
    else
    {
      LOG(WARNING) << "Lua bindings for recastNavigation plugin were not registered: lua state is nil";
    }
  }

  bool RecastNavigationPlugin::installImpl()
  {
    mFacade->registerSystemFactory<RecastNavigationSystem>();
    return true;
  }

  void RecastNavigationPlugin::uninstallImpl()
  {
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view& lua = *mLuaInterface->getSolState();

      lua["Engine"]["navigation"] = sol::lua_nil;
      lua["Entity"]["navigation"] = sol::lua_nil;
    }

    mFacade->getEngine()->removeSystem("navigation");
    mFacade->removeSystemFactory<RecastNavigationSystem>();
  }
}

Gsage::RecastNavigationPlugin* recastNavigationPlugin = NULL;

extern "C" bool PluginExport dllStartPlugin(Gsage::GsageFacade* facade)
{
  if(recastNavigationPlugin != NULL)
  {
    return false;
  }
  recastNavigationPlugin = new Gsage::RecastNavigationPlugin();
  return facade->installPlugin(recastNavigationPlugin);
}

extern "C" bool PluginExport dllStopPlugin(Gsage::GsageFacade* facade)
{
  if(recastNavigationPlugin == NULL)
    return true;

  bool res = facade->uninstallPlugin(recastNavigationPlugin);
  if(!res)
    return false;
  delete recastNavigationPlugin;
  recastNavigationPlugin = NULL;
  return true;
}
