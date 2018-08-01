#ifndef _IMGUI_EXPORT_HEADER_
#define _IMGUI_EXPORT_HEADER_

#include "GsageDefinitions.h"

#if GSAGE_PLATFORM == GSAGE_WIN32

#ifdef IMGUI_PLUGIN_EXPORT
#define IMGUI_PLUGIN_API __declspec(dllexport)
#else
#define IMGUI_PLUGIN_API __declspec(dllimport)
#endif

#else
#define IMGUI_PLUGIN_API
#endif

#endif
