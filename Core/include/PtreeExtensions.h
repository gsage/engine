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

#ifndef _PtreeExtensions_H_
#define _PtreeExtensions_H_

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreColourValue.h>
#include <OgreQuaternion.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "GsageDefinitions.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include "Logger.h"

using namespace Gsage;

static inline float fromHex(const unsigned int& value, const int& offset)
{
  return (value >> offset & 0xFF) / 255.0f;
}

static inline int toHex(const float& value, const int& offset)
{
  return (int)(value * 0xFF) << offset;
}

static void traverse_recursive(DataNode &parent, const DataNode::path_type &childPath, const DataNode &child)
{
  parent.put(childPath, child.data());
  for(DataNode::const_iterator it = child.begin(); it!=child.end(); ++it) {
    if(it->first.empty())
    {
      parent.get_child(childPath).push_back(std::make_pair("", it->second));
    }
    else
    {
      DataNode::path_type curPath = childPath / DataNode::path_type(it->first);
      traverse_recursive(parent, curPath, it->second);
    }
  }
}

static bool parseFile(const std::string& path, DataNode& dest)
{
  std::ifstream stream(path);
  bool succeed = true;
  try
  {
    boost::property_tree::read_json(stream, dest);
  }
  catch(boost::property_tree::json_parser_error& ex)
  {
    LOG(ERROR) << "Failed to read file: " << path << ", reason: " << ex.message();
    succeed = false;
  }

  stream.close();

  return succeed;
}

static bool dumpFile(const std::string& path, const DataNode& source)
{
  std::ofstream stream(path);
  bool succeed = true;
  try
  {
    boost::property_tree::write_json(stream, source, false);
  }
  catch(boost::property_tree::json_parser_error& ex)
  {
    LOG(ERROR) << "Failed to write save, reason: " << ex.message();
    succeed = false;
  }
  stream.close();

  return succeed;
}

static void traverse(DataNode& parent, const DataNode& child)
{
  traverse_recursive(parent, "", child);
}

static DataNode getMergedDataNode(const DataNode& first, const DataNode& second)
{
  DataNode parent = first;
  traverse(parent, second);
  return parent;
}

static void mergeNodes(DataNode& mergeTo, const DataNode& child)
{
  traverse(mergeTo, child);
}

struct DegreeTranslator
{
  typedef std::string internal_type;
  typedef Ogre::Degree external_type;

  boost::optional<external_type> get_value(const internal_type& str)
  {
    if(str.empty())
      return boost::optional<external_type>(boost::none);

    return boost::optional<external_type>(Ogre::Degree((float)atof(str.c_str())));
  }

  boost::optional<internal_type> put_value(const external_type& value)
  {
    std::stringstream stream;
    stream << value.valueDegrees();
    return boost::optional<internal_type>(stream.str());
  }
};

struct ColourValueTranslator
{
  typedef std::string internal_type;
  typedef Ogre::ColourValue external_type;

  boost::optional<external_type> get_value(const internal_type& str)
  {
    if(str.empty())
      return boost::optional<external_type>(boost::none);

    Ogre::ColourValue res;
    unsigned int value;

    try
    {
      value = std::stoul(str, nullptr, 16);
    }
    catch(std::invalid_argument)
    {
      // TODO: log error
      return boost::optional<external_type>(boost::none);
    }
    res.a = fromHex(value, 24);
    res.r = fromHex(value, 16);
    res.g = fromHex(value, 8);
    res.b = fromHex(value, 0);
    return boost::optional<external_type>(res);
  }

  boost::optional<internal_type> put_value(const external_type& value)
  {
    std::stringstream stream;
    stream << "0x" <<std::hex << (toHex(value.a, 24) | toHex(value.r, 16) | toHex(value.g, 8) | toHex(value.b, 0));
    return boost::optional<internal_type>(stream.str());
  }
};

struct Vector3Translator
{
  typedef std::string internal_type;
  typedef Ogre::Vector3 external_type;

  boost::optional<external_type> get_value(const internal_type& str)
  {
    if(str.empty())
      return boost::optional<external_type>(boost::none);

    Ogre::Vector3 res = Ogre::Vector3::ZERO;
    std::vector<std::string> values = split(str, ',');

    std::stringstream stream;

    if(values.size() == 0)
    {
      // TODO: log error
      return boost::optional<external_type>(boost::none);
    }

    for(unsigned int i = 0; i < values.size(); i++)
    {
      stream << values[i];
      if(i == 0)
        stream >> res.x;
      else if(i == 1)
        stream >> res.y;
      else if(i == 2)
        stream >> res.z;
      else
        break;

      stream.clear();
    }

    return boost::optional<external_type>(res);
  }

  boost::optional<internal_type> put_value(const external_type& value)
  {
    std::stringstream stream;
    stream << value.x << "," << value.y << "," << value.z;
    return boost::optional<internal_type>(stream.str());
  }
};

struct QuaternionTranslator
{
  typedef std::string internal_type;
  typedef Ogre::Quaternion external_type;

  boost::optional<external_type> get_value(const internal_type& str)
  {
    if(str.empty())
      return boost::optional<external_type>(boost::none);

    external_type res;
    std::vector<std::string> values = split(str, ',');

    std::stringstream stream;

    if(values.size() == 0)
    {
      // TODO: log error
      return boost::optional<external_type>(boost::none);
    }

    for(unsigned int i = 0; i < values.size(); i++)
    {
      stream << values[i];
      if(i == 0)
        stream >> res.w;
      else if(i == 1)
        stream >> res.x;
      else if(i == 2)
        stream >> res.y;
      else if(i == 3)
        stream >> res.z;
      else
        break;

      stream.clear();
    }

    return boost::optional<external_type>(res);
  }

  boost::optional<internal_type> put_value(const external_type& value)
  {
    std::stringstream stream;
    stream << value.w << "," << value.x << "," << value.y << "," << value.z;
    return boost::optional<internal_type>(stream.str());
  }
};

struct OgreFloatRectTranslator
{
  // left top right bottom
  typedef std::string internal_type;
  typedef Ogre::FloatRect external_type;

  boost::optional<external_type> get_value(const internal_type& str)
  {
    if(str.empty())
      return boost::optional<external_type>(boost::none);

    external_type res;
    std::vector<std::string> values = split(str, ',');

    std::stringstream stream;

    if(values.size() == 0)
    {
      // TODO: log error
      return boost::optional<external_type>(boost::none);
    }

    for(unsigned int i = 0; i < values.size(); i++)
    {
      stream << values[i];
      if(i == 0)
        stream >> res.left;
      else if(i == 1)
        stream >> res.top;
      else if(i == 2)
        stream >> res.right;
      else if(i == 3)
        stream >> res.bottom;
      else
        break;

      stream.clear();
    }

    return boost::optional<external_type>(res);
  }

  boost::optional<internal_type> put_value(const external_type& value)
  {
    std::stringstream stream;
    stream << value.left << "," << value.top << "," << value.right << "," << value.bottom;
    return boost::optional<internal_type>(stream.str());
  }
};

/*  Specialize translator_between so that it uses our custom translator for
    bool value types. Specialization must be in boost::property_tree
    namespace. */
namespace boost {
  namespace property_tree {

    template<typename Ch, typename Traits, typename Alloc> 
      struct translator_between<std::basic_string< Ch, Traits, Alloc >, Ogre::ColourValue>
      {
        typedef ColourValueTranslator type;
      };

    template<typename Ch, typename Traits, typename Alloc> 
      struct translator_between<std::basic_string< Ch, Traits, Alloc >, Ogre::Vector3>
      {
        typedef Vector3Translator type;
      };

    template<typename Ch, typename Traits, typename Alloc> 
      struct translator_between<std::basic_string< Ch, Traits, Alloc >, Ogre::Quaternion>
      {
        typedef QuaternionTranslator type;
      };

    template<typename Ch, typename Traits, typename Alloc> 
      struct translator_between<std::basic_string< Ch, Traits, Alloc >, Ogre::Degree>
      {
        typedef DegreeTranslator type;
      };

    template<typename Ch, typename Traits, typename Alloc> 
      struct translator_between<std::basic_string< Ch, Traits, Alloc >, Ogre::FloatRect>
      {
        typedef OgreFloatRectTranslator type;
      };
  } // namespace property_tree
} // namespace boost


#endif
