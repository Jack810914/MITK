/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "mitkPluginActivator.h"
#include "../QmitkDataManagerView.h"
#include "../QmitkDataManagerPreferencePage.h"
#include "../QmitkDataManagerHotkeysPrefPage.h"
#include <QPrinterInfo>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  // Dummy code to force linkage to Qt5PrintSupport (issue with GCC 7.3)
  QPrinterInfo info;
  info.isNull();

  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerHotkeysPrefPage, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
