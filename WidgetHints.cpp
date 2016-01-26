#include "WidgetHints.h"

#include <vtk_jsoncpp.h>

#include <fstream>
#include <iostream>
#include <vector>

// Define to print debugging information:
//#define VERBOSE

//------------------------------------------------------------------------------
//*************************** WidgetHints::Internals ***************************
//------------------------------------------------------------------------------
struct WidgetHints::Internal
{
  typedef std::vector<std::string> Path;

  Path Groups;

  Json::Value Root;

  Path createPath(const std::string &input);

  Json::Value findValue(const std::string &path);
  Json::Value findValue(const Path &path);
};

//------------------------------------------------------------------------------
WidgetHints::Internal::Path
WidgetHints::Internal::createPath(const std::string &input)
{
  Path result(this->Groups);

#ifdef VERBOSE
  std::cout << "Creating path from: " << input << std::endl;
#endif

  size_t startIdx = 0;
  size_t endIdx = 0;
  while ((endIdx = input.find_first_of('/', startIdx)) != std::string::npos)
    {
    if (startIdx != endIdx)
      {
      result.push_back(input.substr(startIdx, endIdx - startIdx));
#ifdef VERBOSE
      std::cout << "Appending '" << result.back() << "'" << std::endl;
#endif
      }

    startIdx = ++endIdx;
    }
  if (startIdx != input.size())
    {
    result.push_back(input.substr(startIdx));
#ifdef VERBOSE
      std::cout << "Appending '" << result.back() << "'" << std::endl;
#endif
    }

  return result;
}

//------------------------------------------------------------------------------
Json::Value WidgetHints::Internal::findValue(const std::string &path)
{
  return this->findValue(this->createPath(path));
}

//------------------------------------------------------------------------------
Json::Value
WidgetHints::Internal::findValue(const WidgetHints::Internal::Path &path)
{
  Json::Value current = this->Root;
  std::string currentStr;

  for (Path::const_iterator it = path.begin(), itEnd = path.end(); it != itEnd;
       ++it)
    {
#ifdef VERBOSE
    std::cout << "Searching for value...current entry: " << *it
              << " JsonType: " << current.type() << std::endl;
#endif
    if (!current.isObject())
      {
#ifdef VERBOSE
      std::cerr << "Error locating widget hint at path " << currentStr << "/"
                << *it << ": JSON value at '" << currentStr
                << "' is not an object." << std::endl;
#endif
      return Json::Value();
      }

    current = current[*it];
    currentStr += std::string("/") + *it;

    if (current.isNull())
      {
#ifdef VERBOSE
      std::cerr << "Error locating widget hint at path " << currentStr
                << ": Invalid path, no such object." << std::endl;
#endif
      return Json::Value();
      }
    }

  return current;
}

//------------------------------------------------------------------------------
//*************************** WidgetHints **************************************
//------------------------------------------------------------------------------
WidgetHints::WidgetHints()
  : Internals(new Internal)
{
}

//------------------------------------------------------------------------------
WidgetHints::~WidgetHints()
{
  delete Internals;
}

//------------------------------------------------------------------------------
void WidgetHints::reset()
{
#ifdef VERBOSE
  std::cout << "Resetting widget hints." << std::endl;
#endif
  this->Internals->Root = Json::Value(Json::objectValue);
}

//------------------------------------------------------------------------------
bool WidgetHints::loadFile(const std::string &fileName)
{
  this->reset();

#ifdef VERBOSE
  std::cout << "Loading widget hints from: " << fileName << std::endl;
#endif

  std::ifstream inFile(fileName.c_str());
  if (!inFile)
    {
    std::cerr << "Error loading widget hints file: " << fileName << std::endl;
    return false;
    }

  Json::Reader reader;
  if (!reader.parse(inFile, this->Internals->Root))
    {
    std::cerr << "Error parsing widget hints file.\nFile:  " << fileName << "\n"
              << reader.getFormattedErrorMessages() << std::endl;
    return false;
    }

  return true;
}

//------------------------------------------------------------------------------
void WidgetHints::pushGroup(const std::string &group)
{
#ifdef VERBOSE
  std::cout << "Pushing group(s): " << group << std::endl;
#endif
  this->Internals->Groups = this->Internals->createPath(group);
}

//------------------------------------------------------------------------------
void WidgetHints::popGroup()
{
#ifdef VERBOSE
  std::cout << "Popping group: " << this->Internals->Groups.back() << std::endl;
#endif
  this->Internals->Groups.pop_back();
}

//------------------------------------------------------------------------------
bool WidgetHints::isEnabled(const std::string &name) const
{
  Json::Value value = this->Internals->findValue(name + "/enabled");

  if (value.isBool())
    {
    return value.asBool();
    }

  // True by default:
  return true;
}

