/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef CHERRYLOGVIEW_H_
#define CHERRYLOGVIEW_H_

#include <QtGui/QWidget>

#include <org.opencherry.ui/src/cherryViewPart.h>

#include "cherryUiQtDll.h"

namespace cherry {
  
class CHERRY_UI_QT LogView : public ViewPart
{
public:
  void* CreatePartControl(void* parent);
  void SetFocus();
 
};

} // namespace cherry

#endif /*CHERRYLOGVIEW_H_*/
