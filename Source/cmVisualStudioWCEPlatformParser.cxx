/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVisualStudioWCEPlatformParser.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmXMLParser.h"

int cmVisualStudioWCEPlatformParser::ParseVersion(const char* version)
{
  const std::string registryBase =
    cmGlobalVisualStudioGenerator::GetRegistryBase(version);
  const std::string vckey = registryBase + "\\Setup\\VC;ProductDir";
  const std::string vskey = registryBase + "\\Setup\\VS;ProductDir";

  if(!cmSystemTools::ReadRegistryValue(vckey.c_str(), this->VcInstallDir) ||
     !cmSystemTools::ReadRegistryValue(vskey.c_str(), this->VsInstallDir))
    {
    return 0;
    }
  cmSystemTools::ConvertToUnixSlashes(this->VcInstallDir);
  cmSystemTools::ConvertToUnixSlashes(this->VsInstallDir);
  this->VcInstallDir.append("/");
  this->VsInstallDir.append("/");

  const std::string configFilename =
    this->VcInstallDir + "vcpackages/WCE.VCPlatform.config";

  return this->ParseFile(configFilename.c_str());
}

std::string cmVisualStudioWCEPlatformParser::GetOSVersion() const
{
  if (this->OSMinorVersion.empty())
    {
    return OSMajorVersion;
    }

  return OSMajorVersion + "." + OSMinorVersion;
}

const char* cmVisualStudioWCEPlatformParser::GetArchitectureFamily() const
{
  std::map<std::string, std::string>::const_iterator it =
    this->Macros.find("ARCHFAM");
  if (it != this->Macros.end())
    {
    return it->second.c_str();
    }

  return 0;
}

void cmVisualStudioWCEPlatformParser::StartElement(const char* name,
                                                   const char** attributes)
{
  if(this->FoundRequiredName)
    {
    return;
    }

  this->CharacterData = "";

  if(strcmp(name, "PlatformData") == 0)
    {
    this->PlatformName = "";
    this->OSMajorVersion = "";
    this->OSMinorVersion = "";
    this->Macros.clear();
    }

  if(strcmp(name, "Macro") == 0)
    {
    std::string macroName;
    std::string macroValue;

    for(const char** attr = attributes; *attr; attr += 2)
      {
      if(strcmp(attr[0], "Name") == 0)
        {
        macroName = attr[1];
        }
      else if(strcmp(attr[0], "Value") == 0)
        {
        macroValue = attr[1];
        }
      }

    if(!macroName.empty())
      {
      this->Macros[macroName] = macroValue;
      }
    }
  else if(strcmp(name, "Directories") == 0)
    {
    for(const char** attr = attributes; *attr; attr += 2)
      {
      if(strcmp(attr[0], "Include") == 0)
        {
        this->Include = attr[1];
        }
      else if(strcmp(attr[0], "Library") == 0)
        {
        this->Library = attr[1];
        }
      else if(strcmp(attr[0], "Path") == 0)
        {
        this->Path = attr[1];
        }
      }
    }
}

void cmVisualStudioWCEPlatformParser::EndElement(const char* name)
{
  if(!this->RequiredName)
    {
    if(strcmp(name, "PlatformName") == 0)
      {
      this->AvailablePlatforms.push_back(this->CharacterData);
      }
    return;
    }

  if(this->FoundRequiredName)
    {
    return;
    }

  if(strcmp(name, "PlatformName") == 0)
    {
    this->PlatformName = this->CharacterData;
    }
  else if(strcmp(name, "OSMajorVersion") == 0)
    {
    this->OSMajorVersion = this->CharacterData;
    }
  else if(strcmp(name, "OSMinorVersion") == 0)
   {
   this->OSMinorVersion = this->CharacterData;
   }
  else if(strcmp(name, "Platform") == 0)
    {
    if(this->PlatformName == this->RequiredName)
      {
      this->FoundRequiredName = true;
      }
    }
}

void cmVisualStudioWCEPlatformParser::CharacterDataHandler(const char* data,
                                                           int length)
{
  this->CharacterData.append(data, length);
}

std::string cmVisualStudioWCEPlatformParser::FixPaths(
    const std::string& paths) const
{
  std::string ret = paths;
  cmSystemTools::ReplaceString(ret, "$(PATH)", "%PATH%");
  cmSystemTools::ReplaceString(ret, "$(VCInstallDir)", VcInstallDir.c_str());
  cmSystemTools::ReplaceString(ret, "$(VSInstallDir)", VsInstallDir.c_str());
  cmSystemTools::ReplaceString(ret, "\\", "/");
  cmSystemTools::ReplaceString(ret, "//", "/");
  cmSystemTools::ReplaceString(ret, "/", "\\");
  return ret;
}
