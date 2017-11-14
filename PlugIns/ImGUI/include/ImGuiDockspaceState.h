/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Gsage Authors

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

// contains helpers for imgui dockstate converting to DataProxy

#include "DataProxy.h"
#include "ImGuiDockspace.h"

namespace Gsage {
  static DataProxy dumpState(ImGuiDockspaceState state)
  {
    DataProxy res = DataProxy::create(DataWrapper::JSON_OBJECT);
    for(auto pair : state.docks) {
      DataProxy child = res.createChild(pair.first);
      child.put("ratio", pair.second.ratio);

      if(!pair.second.children[0].empty() && !pair.second.children[1].empty()) {
        DataProxy children = child.createChild("children");
        children.push(pair.second.children[0]);
        children.push(pair.second.children[1]);
      }

      child.put("next", pair.second.next);
      child.put("prev", pair.second.prev);
      child.put("parent", pair.second.parent);

      child.put("layout", (int)pair.second.layout);
      child.put("location", (int)pair.second.location);

      child.put("active", pair.second.active);
      child.put("opened", pair.second.opened);
    }
    return res;
  }

/*
      float ratio;

      std::string children[2];
      std::string next;
      std::string prev;
      std::string parent;

      Dock::Layout layout;
      Dock::Location location;

      bool active;
      bool opened;
 */

  static ImGuiDockspaceState loadState(DataProxy dp)
  {
    ImGuiDockspaceState res;
    for(auto pair : dp) 
    {
      ImGuiDockspaceState::Dockstate dockstate;

      dockstate.ratio = pair.second.get("ratio", 0.5f);

      auto children = pair.second.get<DataProxy>("children");
      if(children.second) {
        for(int i = 0; i < 2; ++i) {
          dockstate.children[i] = children.first[i].as<std::string>();
        }
      }

      dockstate.next = pair.second.get("next", "");
      dockstate.prev = pair.second.get("prev", "");
      dockstate.parent = pair.second.get("parent", "");

      dockstate.layout= (Dock::Layout)(pair.second.get("layout", 0));
      dockstate.location = (Dock::Location)(pair.second.get("location", 10));

      dockstate.active = pair.second.get("active", true);
      dockstate.opened = pair.second.get("opened", true);

      res.docks[pair.first] = dockstate;
    }

    return res;
  }
}
