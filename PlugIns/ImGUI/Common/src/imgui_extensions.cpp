#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#ifdef _WIN32
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#elif defined(__GLIBC__) || defined(__sun)
#include <alloca.h>     // alloca
#else
#include <stdlib.h>     // alloca
#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'xxxx' to type 'xxxx' casts away qualifiers
#endif
#include "imgui_extensions.h"
#include "Logger.h"

namespace ImGui
{
  VerticalGradient::VerticalGradient(const ImVec2& start, const ImVec2& end, const ImVec4& col0, const ImVec4& col1)
    : Col0(col0)
    , Col1(col1)
    , Start(start)
    , End(end)
  {
    evalStep();
  }

  VerticalGradient::VerticalGradient(const ImVec2& start, const ImVec2& end, ImU32 col0, ImU32 col1)
    : Col0(ColorConvertU32ToFloat4(col0))
    , Col1(ColorConvertU32ToFloat4(col1))
    , Start(start)
    , End(end)
  {
    evalStep();
  }

  void VerticalGradient::evalStep()
  {
    Len = ImLengthSqr(End - Start);
  }

  ImU32 VerticalGradient::Calc(const ImVec2& pos) const
  {
    const float fa = std::min(1.0f, (pos.y - Start.y)/(End.y - Start.y));
    const float fc = std::max(0.0f, (1.f - fa));

    return ColorConvertFloat4ToU32(ImVec4(
        Col0.x * fc + Col1.x * fa,
        Col0.y * fc + Col1.y * fa,
        Col0.z * fc + Col1.z * fa,
        Col0.w * fc + Col1.w * fa)
    );
  }
}
