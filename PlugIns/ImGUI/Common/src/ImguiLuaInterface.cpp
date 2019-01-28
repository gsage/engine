/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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

#include "ImguiLuaInterface.h"
#include <stdio.h>
#include <imgui.h>
#include <deque>

#include "sol.hpp"

namespace Gsage {

  ImguiTextBuffer::ImguiTextBuffer(int size, const char* value)
    : mBuffer(new char[size]())
    , mSize(size)
  {
  }

  ImguiTextBuffer::~ImguiTextBuffer()
  {
    delete[] mBuffer;
  }

  std::string ImguiTextBuffer::read() const
  {
    return std::string(mBuffer);
  }

  char* ImguiTextBuffer::read()
  {
    return mBuffer;
  }

  bool ImguiTextBuffer::write(const std::string& value)
  {
    if(value.size() > mSize) {
      return false;
    }
    strcpy(mBuffer, value.c_str());
    return true;
  }

  // define ENABLE_IM_LUA_END_STACK
  // to keep track of end and begins and clean up the imgui stack
  // if lua errors

#ifdef ENABLE_IM_LUA_END_STACK
  // Stack for imgui begin and end
  std::deque<int> endStack;
  static void AddToStack(int type) {
    endStack.push_back(type);
  }

  static void PopEndStack(int type) {
    if (!endStack.empty()) {
      endStack.pop_back(); // hopefully the type matches
    }
  }

  static void ImEndStack(int type);

#endif

#define IMGUI_FUNCTION_DRAW_LIST(name) \
  static int impl_draw_list_##name(lua_State *L) { \
    int max_args = lua_gettop(L); \
    int arg = 1; \
    int stackval = 0;

#define IMGUI_FUNCTION(name) \
    static int impl_##name(lua_State *L) { \
      int max_args = lua_gettop(L); \
      int arg = 1; \
      int stackval = 0;

      // I use OpenGL so this is a GLuint
      // Using unsigned int cause im lazy don't copy me
#define IM_TEXTURE_ID_ARG(name) \
      const ImTextureID name = (ImTextureID)luaL_checkinteger(L, arg++);

#define OPTIONAL_LABEL_ARG(name) \
      const char* name; \
      if (arg <= max_args) { \
        name = lua_tostring(L, arg++); \
      } else { \
        name = NULL; \
      }

#define LABEL_ARG(name) \
      size_t i_##name##_size; \
      const char * name = luaL_checklstring(L, arg++, &(i_##name##_size));

#define IM_VEC_2_ARG(name)\
      const lua_Number i_##name##_x = luaL_checknumber(L, arg++); \
      const lua_Number i_##name##_y = luaL_checknumber(L, arg++); \
      const ImVec2 name((double)i_##name##_x, (double)i_##name##_y);

#define OPTIONAL_IM_VEC_2_ARG(name, x, y) \
      lua_Number i_##name##_x = x; \
      lua_Number i_##name##_y = y; \
      if (arg <= max_args - 1) { \
        i_##name##_x = luaL_checknumber(L, arg++); \
        i_##name##_y = luaL_checknumber(L, arg++); \
      } \
      const ImVec2 name((double)i_##name##_x, (double)i_##name##_y);

#define IM_VEC_4_ARG(name) \
      const lua_Number i_##name##_x = luaL_checknumber(L, arg++); \
      const lua_Number i_##name##_y = luaL_checknumber(L, arg++); \
      const lua_Number i_##name##_z = luaL_checknumber(L, arg++); \
      const lua_Number i_##name##_w = luaL_checknumber(L, arg++); \
      const ImVec4 name((double)i_##name##_x, (double)i_##name##_y, (double)i_##name##_z, (double)i_##name##_w);

#define OPTIONAL_IM_VEC_4_ARG(name, x, y, z, w) \
      lua_Number i_##name##_x = x; \
      lua_Number i_##name##_y = y; \
      lua_Number i_##name##_z = z; \
      lua_Number i_##name##_w = w; \
      if (arg <= max_args - 1) { \
        i_##name##_x = luaL_checknumber(L, arg++); \
        i_##name##_y = luaL_checknumber(L, arg++); \
        i_##name##_z = luaL_checknumber(L, arg++); \
        i_##name##_w = luaL_checknumber(L, arg++); \
      } \
      const ImVec4 name((double)i_##name##_x, (double)i_##name##_y, (double)i_##name##_z, (double)i_##name##_w);

#define NUMBER_ARG(name)\
      lua_Number name = luaL_checknumber(L, arg++);

#define OPTIONAL_NUMBER_ARG(name, otherwise)\
      lua_Number name = otherwise; \
      if (arg <= max_args) { \
        name = lua_tonumber(L, arg++); \
      }

#define FLOAT_POINTER_ARG(name) \
      float i_##name##_value = luaL_checknumber(L, arg++); \
      float* name = &(i_##name##_value);

#define END_FLOAT_POINTER(name) \
      if (name != NULL) { \
        lua_pushnumber(L, i_##name##_value); \
        stackval++; \
      }

#define OPTIONAL_INT_ARG(name, otherwise)\
      int name = otherwise; \
      if (arg <= max_args) { \
        name = (int)lua_tonumber(L, arg++); \
      }

#define INT_ARG(name) \
      const int name = (int)luaL_checknumber(L, arg++);

#define OPTIONAL_UINT_ARG(name, otherwise)\
      unsigned int name = otherwise; \
      if (arg <= max_args) { \
        name = (unsigned int)lua_tounsigned(L, arg++); \
      }

#define UINT_ARG(name) \
      const unsigned int name = (unsigned int)luaL_checkinteger(L, arg++);

#define INT_POINTER_ARG(name) \
      int i_##name##_value = (int)luaL_checkinteger(L, arg++); \
      int* name = &(i_##name##_value);

#define END_INT_POINTER(name) \
      if (name != NULL) { \
        lua_pushnumber(L, i_##name##_value); \
        stackval++; \
      }

#define UINT_POINTER_ARG(name) \
      unsigned int i_##name##_value = (unsigned int)luaL_checkinteger(L, arg++); \
      unsigned int* name = &(i_##name##_value);

#define END_UINT_POINTER(name) \
      if (name != NULL) { \
        lua_pushnumber(L, i_##name##_value); \
        stackval++; \
      }

#define BOOL_POINTER_ARG(name) \
      bool i_##name##_value = lua_toboolean(L, arg++); \
      bool* name = &(i_##name##_value);

#define OPTIONAL_BOOL_POINTER_ARG(name) \
      bool i_##name##_value; \
      bool* name = NULL; \
      if (arg <= max_args) { \
        i_##name##_value = lua_toboolean(L, arg++); \
        name = &(i_##name##_value); \
      }

#define OPTIONAL_BOOL_ARG(name, otherwise) \
      bool name = otherwise; \
      if (arg <= max_args) { \
        name = lua_toboolean(L, arg++); \
      }

#define BOOL_ARG(name) \
      bool name = lua_toboolean(L, arg++);

#define CALL_FUNCTION(name, retType,...) \
      retType ret = ImGui::name(__VA_ARGS__);

#define DRAW_LIST_CALL_FUNCTION(name, retType,...) \
      retType ret = ImGui::GetWindowDrawList()->name(__VA_ARGS__);

#define CALL_FUNCTION_NO_RET(name, ...) \
      ImGui::name(__VA_ARGS__);

#define DRAW_LIST_CALL_FUNCTION_NO_RET(name, ...) \
      ImGui::GetWindowDrawList()->name(__VA_ARGS__);

#define PUSH_NUMBER(name) \
      lua_pushnumber(L, name); \
      stackval++;

#define PUSH_BOOL(name) \
      lua_pushboolean(L, (int) name); \
      stackval++;

#define END_BOOL_POINTER(name) \
      if (name != NULL) { \
        lua_pushboolean(L, (int)i_##name##_value); \
        stackval++; \
      }

#define END_IMGUI_FUNC \
      return stackval; \
    }

#ifdef ENABLE_IM_LUA_END_STACK
#define IF_RET_ADD_END_STACK(type) \
    if (ret) { \
      AddToStack(type); \
    }

#define ADD_END_STACK(type) \
    AddToStack(type);

#define POP_END_STACK(type) \
    PopEndStack(type);

#define END_STACK_START \
    static void ImEndStack(int type) { \
      switch(type) {

#define END_STACK_OPTION(type, function) \
        case type: \
                   ImGui::function(); \
        break;

#define END_STACK_END \
      } \
    }
#else
#define END_STACK_START
#define END_STACK_OPTION(type, function)
#define END_STACK_END
#define IF_RET_ADD_END_STACK(type)
#define ADD_END_STACK(type)
#define POP_END_STACK(type)
#endif

#include "ImguiLuaIterator.h"

    static const struct luaL_Reg imguilib [] = {
#undef IMGUI_FUNCTION
#define IMGUI_FUNCTION(name) {#name, impl_##name},
#undef IMGUI_FUNCTION_DRAW_LIST
#define IMGUI_FUNCTION_DRAW_LIST(name) {"DrawList_" #name, impl_draw_list_##name},
      // These defines are just redefining everything to nothing so
      // we can get the function names
#undef IM_TEXTURE_ID_ARG
#define IM_TEXTURE_ID_ARG(name)
#undef OPTIONAL_LABEL_ARG
#define OPTIONAL_LABEL_ARG(name)
#undef LABEL_ARG
#define LABEL_ARG(name)
#undef IM_VEC_2_ARG
#define IM_VEC_2_ARG(name)
#undef OPTIONAL_IM_VEC_2_ARG
#define OPTIONAL_IM_VEC_2_ARG(name, x, y)
#undef IM_VEC_4_ARG
#define IM_VEC_4_ARG(name)
#undef OPTIONAL_IM_VEC_4_ARG
#define OPTIONAL_IM_VEC_4_ARG(name, x, y, z, w)
#undef NUMBER_ARG
#define NUMBER_ARG(name)
#undef OPTIONAL_NUMBER_ARG
#define OPTIONAL_NUMBER_ARG(name, otherwise)
#undef FLOAT_POINTER_ARG
#define FLOAT_POINTER_ARG(name)
#undef END_FLOAT_POINTER
#define END_FLOAT_POINTER(name)
#undef OPTIONAL_INT_ARG
#define OPTIONAL_INT_ARG(name, otherwise)
#undef INT_ARG
#define INT_ARG(name)
#undef OPTIONAL_UINT_ARG
#define OPTIONAL_UINT_ARG(name, otherwise)
#undef UINT_ARG
#define UINT_ARG(name)
#undef INT_POINTER_ARG
#define INT_POINTER_ARG(name)
#undef END_INT_POINTER
#define END_INT_POINTER(name)
#undef UINT_POINTER_ARG
#define UINT_POINTER_ARG(name)
#undef END_UINT_POINTER
#define END_UINT_POINTER(name)
#undef BOOL_POINTER_ARG
#define BOOL_POINTER_ARG(name)
#undef OPTIONAL_BOOL_POINTER_ARG
#define OPTIONAL_BOOL_POINTER_ARG(name)
#undef OPTIONAL_BOOL_ARG
#define OPTIONAL_BOOL_ARG(name, otherwise)
#undef BOOL_ARG
#define BOOL_ARG(name)
#undef CALL_FUNCTION
#define CALL_FUNCTION(name, retType, ...)
#undef DRAW_LIST_CALL_FUNCTION
#define DRAW_LIST_CALL_FUNCTION(name, retType, ...)
#undef CALL_FUNCTION_NO_RET
#define CALL_FUNCTION_NO_RET(name, ...)
#undef DRAW_LIST_CALL_FUNCTION_NO_RET
#define DRAW_LIST_CALL_FUNCTION_NO_RET(name, ...)
#undef PUSH_NUMBER
#define PUSH_NUMBER(name)
#undef PUSH_BOOL
#define PUSH_BOOL(name)
#undef END_BOOL_POINTER
#define END_BOOL_POINTER(name)
#undef END_IMGUI_FUNC
#define END_IMGUI_FUNC
#undef END_STACK_START
#define END_STACK_START
#undef END_STACK_OPTION
#define END_STACK_OPTION(type, function)
#undef END_STACK_END
#define END_STACK_END
#undef IF_RET_ADD_END_STACK
#define IF_RET_ADD_END_STACK(type)
#undef ADD_END_STACK
#define ADD_END_STACK(type)
#undef POP_END_STACK
#define POP_END_STACK(type)
#include "ImguiLuaIterator.h"
    {"Button", impl_Button},
    {NULL, NULL}
  };

  bool InputText(const char* label, ImguiTextBuffer& buffer, ImGuiInputTextFlags flags = 0)
  {
    return ImGui::InputText(label, buffer.read(), buffer.size(), flags);
  }

  bool InputTextWithCallback(const char* label, ImguiTextBuffer& buffer, ImGuiInputTextFlags flags, sol::function callback)
  {
    return ImGui::InputText(label, buffer.read(), buffer.size(), flags, [](ImGuiTextEditCallbackData* data) {
        sol::function cb = *static_cast<sol::function*>(data->UserData);
        int res = cb(data);
        return res;
    }, &callback);
  }

  void ImguiLuaInterface::addLuaBindings(sol::state_view& lua) {
    lua_State* L = lua.lua_state();
    lua_newtable(L);
    luaL_setfuncs(L, imguilib, 0);
    lua_setglobal(L, "imgui");

    // Enums not handled by iterator yet
    lua_pushnumber(L, ImGuiWindowFlags_NoTitleBar);
    lua_setglobal(L, "ImGuiWindowFlags_NoTitleBar");
    lua_pushnumber(L, ImGuiWindowFlags_NoResize);
    lua_setglobal(L, "ImGuiWindowFlags_NoResize");
    lua_pushnumber(L, ImGuiWindowFlags_NoMove);
    lua_setglobal(L, "ImGuiWindowFlags_NoMove");
    lua_pushnumber(L, ImGuiWindowFlags_NoScrollbar);
    lua_setglobal(L, "ImGuiWindowFlags_NoScrollbar");
    lua_pushnumber(L, ImGuiWindowFlags_NoScrollWithMouse);
    lua_setglobal(L, "ImGuiWindowFlags_NoScrollWithMouse");
    lua_pushnumber(L, ImGuiWindowFlags_NoCollapse);
    lua_setglobal(L, "ImGuiWindowFlags_NoCollapse");
    lua_pushnumber(L, ImGuiWindowFlags_AlwaysAutoResize);
    lua_setglobal(L, "ImGuiWindowFlags_AlwaysAutoResize");
    lua_pushnumber(L, ImGuiWindowFlags_NoSavedSettings);
    lua_setglobal(L, "ImGuiWindowFlags_NoSavedSettings");
    lua_pushnumber(L, ImGuiWindowFlags_NoInputs);
    lua_setglobal(L, "ImGuiWindowFlags_NoInputs");
    lua_pushnumber(L, ImGuiWindowFlags_MenuBar);
    lua_setglobal(L, "ImGuiWindowFlags_MenuBar");
    lua_pushnumber(L, ImGuiWindowFlags_HorizontalScrollbar);
    lua_setglobal(L, "ImGuiWindowFlags_HorizontalScrollbar");
    lua_pushnumber(L, ImGuiWindowFlags_NoFocusOnAppearing);
    lua_setglobal(L, "ImGuiWindowFlags_NoFocusOnAppearing");
    lua_pushnumber(L, ImGuiWindowFlags_NoBringToFrontOnFocus);
    lua_setglobal(L, "ImGuiWindowFlags_NoBringToFrontOnFocus");
    lua_pushnumber(L, ImGuiWindowFlags_ChildWindow);
    lua_setglobal(L, "ImGuiWindowFlags_ChildWindow");
    lua_pushnumber(L, ImGuiWindowFlags_Tooltip);
    lua_setglobal(L, "ImGuiWindowFlags_Tooltip");
    lua_pushnumber(L, ImGuiWindowFlags_Popup);
    lua_setglobal(L, "ImGuiWindowFlags_Popup");
    lua_pushnumber(L, ImGuiWindowFlags_Modal);
    lua_setglobal(L, "ImGuiWindowFlags_Modal");
    lua_pushnumber(L, ImGuiWindowFlags_ChildMenu);
    lua_setglobal(L, "ImGuiWindowFlags_ChildMenu");
    lua_pushnumber(L, ImGuiInputTextFlags_CharsDecimal);
    lua_setglobal(L, "ImGuiInputTextFlags_CharsDecimal");
    lua_pushnumber(L, ImGuiInputTextFlags_CharsHexadecimal);
    lua_setglobal(L, "ImGuiInputTextFlags_CharsHexadecimal");
    lua_pushnumber(L, ImGuiInputTextFlags_CharsUppercase);
    lua_setglobal(L, "ImGuiInputTextFlags_CharsUppercase");
    lua_pushnumber(L, ImGuiInputTextFlags_CharsNoBlank);
    lua_setglobal(L, "ImGuiInputTextFlags_CharsNoBlank");
    lua_pushnumber(L, ImGuiInputTextFlags_AutoSelectAll);
    lua_setglobal(L, "ImGuiInputTextFlags_AutoSelectAll");
    lua_pushnumber(L, ImGuiInputTextFlags_EnterReturnsTrue);
    lua_setglobal(L, "ImGuiInputTextFlags_EnterReturnsTrue");
    lua_pushnumber(L, ImGuiInputTextFlags_CallbackCompletion);
    lua_setglobal(L, "ImGuiInputTextFlags_CallbackCompletion");
    lua_pushnumber(L, ImGuiInputTextFlags_CallbackHistory);
    lua_setglobal(L, "ImGuiInputTextFlags_CallbackHistory");
    lua_pushnumber(L, ImGuiInputTextFlags_CallbackAlways);
    lua_setglobal(L, "ImGuiInputTextFlags_CallbackAlways");
    lua_pushnumber(L, ImGuiInputTextFlags_CallbackCharFilter);
    lua_setglobal(L, "ImGuiInputTextFlags_CallbackCharFilter");
    lua_pushnumber(L, ImGuiInputTextFlags_AllowTabInput);
    lua_setglobal(L, "ImGuiInputTextFlags_AllowTabInput");
    lua_pushnumber(L, ImGuiInputTextFlags_CtrlEnterForNewLine);
    lua_setglobal(L, "ImGuiInputTextFlags_CtrlEnterForNewLine");
    lua_pushnumber(L, ImGuiInputTextFlags_NoHorizontalScroll);
    lua_setglobal(L, "ImGuiInputTextFlags_NoHorizontalScroll");
    lua_pushnumber(L, ImGuiInputTextFlags_AlwaysInsertMode);
    lua_setglobal(L, "ImGuiInputTextFlags_AlwaysInsertMode");
    lua_pushnumber(L, ImGuiInputTextFlags_ReadOnly);
    lua_setglobal(L, "ImGuiInputTextFlags_ReadOnly");
    lua_pushnumber(L, ImGuiInputTextFlags_Password);
    lua_setglobal(L, "ImGuiInputTextFlags_Password");
    lua_pushnumber(L, ImGuiInputTextFlags_Multiline);
    lua_setglobal(L, "ImGuiInputTextFlags_Multiline");
    lua_pushnumber(L, ImGuiSelectableFlags_DontClosePopups);
    lua_setglobal(L, "ImGuiSelectableFlags_DontClosePopups");
    lua_pushnumber(L, ImGuiSelectableFlags_SpanAllColumns);
    lua_setglobal(L, "ImGuiSelectableFlags_SpanAllColumns");
    lua_pushnumber(L, ImGuiStyleVar_FramePadding);
    lua_setglobal(L, "ImGuiStyleVar_FramePadding");
    lua_pushnumber(L, ImGuiStyleVar_ItemSpacing);
    lua_setglobal(L, "ImGuiStyleVar_ItemSpacing");
    lua_pushnumber(L, ImGuiStyleVar_WindowPadding);
    lua_setglobal(L, "ImGuiStyleVar_WindowPadding");
    lua_pushnumber(L, ImGuiStyleVar_WindowRounding);
    lua_setglobal(L, "ImGuiStyleVar_WindowRounding");
    lua_pushnumber(L, ImGuiKey_Tab);
    lua_setglobal(L, "ImGuiKey_Tab");
    lua_pushnumber(L, ImGuiKey_LeftArrow);
    lua_setglobal(L, "ImGuiKey_LeftArrow");
    lua_pushnumber(L, ImGuiKey_RightArrow);
    lua_setglobal(L, "ImGuiKey_RightArrow");
    lua_pushnumber(L, ImGuiKey_UpArrow);
    lua_setglobal(L, "ImGuiKey_UpArrow");
    lua_pushnumber(L, ImGuiKey_DownArrow);
    lua_setglobal(L, "ImGuiKey_DownArrow");
    lua_pushnumber(L, ImGuiKey_PageUp);
    lua_setglobal(L, "ImGuiKey_PageUp");
    lua_pushnumber(L, ImGuiKey_PageDown);
    lua_setglobal(L, "ImGuiKey_PageDown");
    lua_pushnumber(L, ImGuiKey_Home);
    lua_setglobal(L, "ImGuiKey_Home");
    lua_pushnumber(L, ImGuiCol_Text);
    lua_setglobal(L, "ImGuiCol_Text");
    lua_pushnumber(L, ImGuiCol_FrameBg);
    lua_setglobal(L, "ImGuiCol_FrameBg");
    lua_pushnumber(L, ImGuiCol_ChildWindowBg);
    lua_setglobal(L, "ImGuiCol_ChildWindowBg");

    sol::table imgui = lua["imgui"];

    imgui["InputFloatN"] = [] (const std::string& label, sol::table t, const int length, int decimal_precision, ImGuiInputTextFlags extra_flags) -> bool {
      float* values = new float[length];
      for(int i = 0; i < length; i++) {
        values[i] = t[i + 1];
      }

      bool ret = false;

      switch(length) {
        case 2:
          ret = ImGui::InputFloat2(label.c_str(), values, decimal_precision, extra_flags);
          break;
        case 3:
          ret = ImGui::InputFloat3(label.c_str(), values, decimal_precision, extra_flags);
          break;
        case 4:
          ret = ImGui::InputFloat4(label.c_str(), values, decimal_precision, extra_flags);
          break;
        default:
          return false;
      }

      for(int i = 0; i < length; i++) {
        t[i + 1] = values[i];
      }

      delete[] values;
      return ret;
    };

    imgui["DragFloatN"] = [] (const std::string& label, sol::table t, const int length, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const std::string& display_format = "%.3f", float power = 1.0f) -> bool {
      float* values = new float[length];
      for(int i = 0; i < length; i++) {
        values[i] = t[i + 1];
      }

      bool ret = false;

      switch(length) {
        case 2:
          ret = ImGui::DragFloat2(label.c_str(), values, v_speed, v_min, v_max, display_format.c_str(), power);
          break;
        case 3:
          ret = ImGui::DragFloat3(label.c_str(), values, v_speed, v_min, v_max, display_format.c_str(), power);
          break;
        case 4:
          ret = ImGui::DragFloat4(label.c_str(), values, v_speed, v_min, v_max, display_format.c_str(), power);
          break;
        default:
          return false;
      }

      for(int i = 0; i < length; i++) {
        t[i + 1] = values[i];
      }

      delete[] values;
      return ret;
    };

    imgui["TextBuffer"] = [] (int size) -> std::shared_ptr<ImguiTextBuffer> {
      return std::shared_ptr<ImguiTextBuffer>(new ImguiTextBuffer(size));
    };

    imgui.new_usertype<ImguiTextBuffer>("ImguiTextBuffer",
        "write", &ImguiTextBuffer::write,
        "read", (std::string(ImguiTextBuffer::*)()const)&ImguiTextBuffer::read
    );

//ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);

    imgui["InputText"] = InputText;
    imgui["InputTextWithCallback"] = InputTextWithCallback;

    imgui.new_usertype<ImGuiTextEditCallbackData>("ImGuiTextEditCallbackData",
        "eventFlag", &ImGuiTextEditCallbackData::EventFlag,
        "insertChars", &ImGuiTextEditCallbackData::InsertChars,
        "deleteChars", &ImGuiTextEditCallbackData::DeleteChars,
        "cursorPos", &ImGuiTextEditCallbackData::CursorPos,
        "getBuf", [] (ImGuiTextEditCallbackData* data) { return std::string(data->Buf); },
        "setBuf", [] (ImGuiTextEditCallbackData* data, const std::string& value) {
            strcpy(data->Buf, value.c_str());
            return value.size();
          },
        "bufSize", &ImGuiTextEditCallbackData::BufSize,
        "eventKey", &ImGuiTextEditCallbackData::EventKey,
        "bufDirty", &ImGuiTextEditCallbackData::BufDirty,
        "selectionStart", &ImGuiTextEditCallbackData::SelectionStart,
        "selectionEnd", &ImGuiTextEditCallbackData::SelectionEnd,
        "bufTextLen", &ImGuiTextEditCallbackData::BufTextLen
    );

    imgui.new_usertype<ImVector<char>>("ImVector",
        "data", &ImVector<char>::Data
    );

    imgui["PushStyleColor_U32"] = [] (ImGuiCol idx, ImU32 col) {
      ImGui::PushStyleColor(idx, ImGui::ColorConvertU32ToFloat4(col));
    };

    imgui["DisplaySize"] = [] () {
      ImVec2 size = ImGui::GetIO().DisplaySize;
      return std::make_tuple(size.x, size.y);
    };

    imgui["SetWantCaptureMouse"] = [](bool value) {
      ImGui::GetIO().WantCaptureMouse = value;
    };
  }
}
