/*
 *      Copyright (C) 2015-2017 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "Controller.h"
#include "ControllerDefinitions.h"
#include "ControllerLayout.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/XBMCTinyXML.h"

using namespace KODI;
using namespace GAME;

const ControllerPtr CController::EmptyPtr;

std::unique_ptr<CController> CController::FromExtension(ADDON::CAddonInfo addonInfo, const cp_extension_t* ext)
{
  return std::unique_ptr<CController>(new CController(std::move(addonInfo)));
}

CController::CController(ADDON::CAddonInfo addonInfo) :
  CAddon(std::move(addonInfo)),
  m_layout(new CControllerLayout)
{
}

CController::~CController() = default;

std::string CController::Label(void)
{
  if (m_layout->Label() > 0)
    return g_localizeStrings.GetAddonString(ID(), m_layout->Label());
  return "";
}

std::string CController::ImagePath(void) const
{
  if (!m_layout->Image().empty())
    return URIUtils::AddFileToFolder(URIUtils::GetDirectory(LibPath()), m_layout->Image());
  return "";
}

void CController::GetFeatures(std::vector<std::string>& features,
                              FEATURE_TYPE type /* = FEATURE_TYPE::UNKNOWN */) const
{
  for (const CControllerFeature& feature : m_layout->Features())
  {
    if (type == FEATURE_TYPE::UNKNOWN || type == feature.Type())
      features.push_back(feature.Name());
  }
}

JOYSTICK::FEATURE_TYPE CController::GetFeatureType(const std::string &feature) const
{
  for (auto it = m_layout->Features().begin(); it != m_layout->Features().end(); ++it)
  {
    if (feature == it->Name())
      return it->Type();
  }
  return JOYSTICK::FEATURE_TYPE::UNKNOWN;
}

JOYSTICK::INPUT_TYPE CController::GetInputType(const std::string& feature) const
{
  for (auto it = m_layout->Features().begin(); it != m_layout->Features().end(); ++it)
  {
    if (feature == it->Name())
      return it->InputType();
  }
  return JOYSTICK::INPUT_TYPE::UNKNOWN;
}

bool CController::LoadLayout(void)
{
  if (!m_bLoaded)
  {
    std::string strLayoutXmlPath = LibPath();

    CXBMCTinyXML xmlDoc;
    if (!xmlDoc.LoadFile(strLayoutXmlPath))
    {
      CLog::Log(LOGDEBUG, "Unable to load %s: %s at line %d", strLayoutXmlPath.c_str(), xmlDoc.ErrorDesc(), xmlDoc.ErrorRow());
      return false;
    }

    TiXmlElement* pRootElement = xmlDoc.RootElement();
    if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != LAYOUT_XML_ROOT)
    {
      CLog::Log(LOGERROR, "%s: Can't find root <%s> tag", strLayoutXmlPath.c_str(), LAYOUT_XML_ROOT);
      return false;
    }

    CLog::Log(LOGINFO, "Loading controller layout %s", strLayoutXmlPath.c_str());

    if (m_layout->Deserialize(pRootElement, this))
      m_bLoaded = true;
  }

  return m_bLoaded;
}
