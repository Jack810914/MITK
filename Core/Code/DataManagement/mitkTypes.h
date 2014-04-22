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


#ifndef MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD
#define MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD


#include "mitkConstants.h"
#include "mitkQuaternion.h"
#include "mitkPoint.h"
#include "mitkVector.h"
#include "mitkMatrix.h"
#include "mitkEqual.h"


// this include hold the old deprecated ways to convert from itk 2 vtk and the likes.
// th calls to these functions shall be removed in future bugsquashings so that this include can be removed.
#include "mitkVectorDeprecated.h"


/*
 * This part of the code has been shifted here to avoid compiler clashes
 * caused by including <itkAffineGeometryFrame.h> before the declaration of
 * the Equal() methods above. This problem occurs when using MSVC and is
 * probably related to a compiler bug.
 */

#include <itkAffineGeometryFrame.h>

namespace mitk
{
  typedef itk::AffineGeometryFrame<ScalarType, 3>::TransformType AffineTransform3D;
}


#define mitkSetConstReferenceMacro(name,type) \
  virtual void Set##name (const type & _arg) \
  { \
    itkDebugMacro("setting " << #name " to " << _arg ); \
    if (this->m_##name != _arg) \
      { \
      this->m_##name = _arg; \
      this->Modified(); \
      } \
  }

#define mitkSetVectorMacro(name,type) \
  mitkSetConstReferenceMacro(name,type)

#define mitkGetVectorMacro(name,type) \
  itkGetConstReferenceMacro(name,type)

#endif /* MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD */
