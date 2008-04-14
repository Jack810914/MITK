/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <qregexp.h>
#include <qmessagebox.h>
#include <string>
#include <strstream>
#include "mitkPointSetWriter.h"
#include "mitkConfig.h"
#include "mitkCoreObjectFactory.h"
#include "QmitkCommonFunctionality.h"
#include <mitkImageAccessByItk.h>
#include <mitkPicFileReader.h>

#include <ipPic/ipPic.h>

#include <vtkWindowToImageFilter.h>
#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>

void CommonFunctionality::SaveToFileWriter( mitk::FileWriterWithInformation::Pointer fileWriter, mitk::BaseData::Pointer data, const char* aFileName)
{ 
  if (! fileWriter->CanWrite(data) ) {
    std::cout << "ERROR: wrong data in SaveToFileWriter" << std::endl;
    return;
  }
  QString fileName;
  if (aFileName == NULL)
  {
    fileName = QFileDialog::getSaveFileName(QString(fileWriter->GetDefaultFilename()),fileWriter->GetFileDialogPattern());

    // Check if an extension exists already and if not, append the default extension
    if ( fileName.find( '.' ) == -1 )
    {
      fileName.append( fileWriter->GetDefaultExtension() );
    }
  }
  else
    fileName = aFileName;

  if (fileName.isEmpty() == false )
  {
    fileWriter->SetFileName( fileName.ascii() );
    fileWriter->DoWrite( data );
  }
}



/**
 * Saves the given mitk::BaseData to a file. The user is prompted to
 * enter a file name. Currently only mitk::Image, mitk::Surface, mitk::PointSet and
 * mitk::VesselGraphData are supported. This function is deprecated
 * until the save-problem is solved by means of a Save-Factory or any
 * other "nice" mechanism
 */
void CommonFunctionality::SaveBaseData( mitk::BaseData* data, const char * aFileName )
{
  if (data != NULL)
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data);
    if(image.IsNotNull())
    {
      CommonFunctionality::SaveImage(image);
    }

    mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(data);
    if(pointset.IsNotNull())
    {
      std::string fileName;
      if(aFileName == NULL)
        fileName = "PointSet";
      else
        fileName = aFileName;
      fileName = itksys::SystemTools::GetFilenameWithoutExtension(fileName);
      fileName += ".mps";
      QString qfileName = QFileDialog::getSaveFileName(QString(fileName.c_str()),"MITK Point-Sets (*.mps)");
      if (fileName.empty() == false )
      {
        mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
        writer->SetInput( pointset );
        writer->SetFileName( qfileName.ascii() );
        writer->Update();
      }
    }

    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(data);
    if(surface.IsNotNull())
    {
      CommonFunctionality::SaveSurface(surface, aFileName);
    }

    // now try the file writers provided by the CoreObjectFactory
    mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
    for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it) {
      if ( (*it)->CanWrite(data) ) {
        SaveToFileWriter(*it, data, NULL); 
      } 
    }
    
    // internal vessel graph save code removed ...
  }
}

mitk::DataTreeNode::Pointer CommonFunctionality::AddVtkMeshToDataTree(vtkPolyData* polys, mitk::DataTreeIteratorBase* iterator, std::string str)
{
  mitk::DataTreeIteratorClone it=iterator;

  mitk::DataTreeNode::Pointer node = NULL;
  mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", mitk::StringProperty::New( str.c_str() ));

  if (subTree->IsAtEnd() || subTree->Get().IsNull() )
  {
    node=mitk::DataTreeNode::New();
    mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New(str.c_str());
    node->SetProperty("name",nameProp);
    it->Add(node);
  }
  else
  {
    node = subTree->Get();
  }

  mitk::Surface::Pointer surface = mitk::Surface::New();
  surface->SetVtkPolyData(polys);
  node->SetData( surface );
  node->SetProperty("layer", mitk::IntProperty::New(1));
  node->SetVisibility(true,NULL);

  float meshColor[3] = {1.0,0.5,0.5};
  node->SetColor(meshColor,  NULL );
  node->SetVisibility(true, NULL );

  return node;
}

mitk::DataTreeNode::Pointer CommonFunctionality::AddPicImageToDataTree(ipPicDescriptor * pic, mitk::DataTreeIteratorBase* iterator, std::string str)
{
  mitk::DataTreeIteratorClone it=iterator;

  mitk::Image::Pointer image = mitk::Image::New();
  image->Initialize(pic);
  image->SetPicVolume(pic);

  mitk::DataTreeNode::Pointer node = NULL;
  mitk::DataTreeIteratorClone subTree = ((mitk::DataTree *) it->GetTree())->GetNext("name", mitk::StringProperty::New( str.c_str() ));

  if (subTree->IsAtEnd() || subTree->Get().IsNull() )
  {
    node=mitk::DataTreeNode::New();
    mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New(str.c_str());
    node->SetProperty("name",nameProp);
    node->SetData(image);
    it->Add(node);
  }
  else
  {
    node = subTree->Get();
    node->SetData(image);
  }

  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  mitk::LevelWindow levelWindow;
  levelWindow.SetAuto( image );
  levWinProp->SetLevelWindow(levelWindow);
  node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
  return node;
}


mitk::DataTreeNode::Pointer CommonFunctionality::FileOpen( const char * fileName )
{
  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  try
  {
    factory->SetFileName( fileName );
    factory->Update();
    return factory->GetOutput( 0 );
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
    return NULL;
  }
}


mitk::DataTreeNode::Pointer CommonFunctionality::FileOpenImageSequence(const char* aFileName)
{
  if(aFileName==NULL) return NULL;

  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  QString fileName = aFileName;
  if (!fileName.contains("dcm") && !fileName.contains("DCM"))
  {
    int fnstart = fileName.findRev( QRegExp("[/\\\\]"), fileName.length() );
    if(fnstart<0) fnstart=0;
    int start = fileName.find( QRegExp("[0-9]"), fnstart );
    if(start<0)
    {
      return FileOpen(fileName.ascii());;
    }

    char prefix[1024], pattern[1024];

    strncpy(prefix, fileName.ascii(), start);
    prefix[start]=0;

    int stop=fileName.find( QRegExp("[^0-9]"), start );
    sprintf(pattern, "%%s%%0%uu%s",stop-start,fileName.ascii()+stop);


    factory->SetFilePattern( pattern );
    factory->SetFilePrefix( prefix );
  }
  else
  {
    //    factory->SetFileName( fileName );
    factory->SetFilePattern( fileName.ascii() );
    factory->SetFilePrefix( fileName.ascii() );
  }
  factory->Update();
  return factory->GetOutput( 0 );

}

mitk::DataTreeNode::Pointer CommonFunctionality::FileOpenImageSequence()
{
  QString fileName = QFileDialog::getOpenFileName(NULL,mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());

  if ( !fileName.isNull() )
  {
    return FileOpenImageSequence(fileName);
  }
  else
  {
    return NULL;
  }
}

mitk::DataTreeNode::Pointer CommonFunctionality::FileOpen()
{
  return CommonFunctionality::FileOpenSpecific( mitk::CoreObjectFactory::GetInstance()->GetFileExtensions() );
}

mitk::DataTreeNode::Pointer CommonFunctionality::FileOpenSpecific( const char *fileExtensions )
{
  QString fileName = QFileDialog::getOpenFileName( NULL, fileExtensions );
  if ( !fileName.isNull() )
  {
    mitk::DataTreeNode::Pointer result = FileOpen(fileName.ascii());
    if ( result.IsNull() )
    {
      return FileOpenImageSequence(fileName);
    }
    else
    {
      return result;
    }
  }
  else
  {
    return NULL;
  }
}

mitk::DataTreeNode::Pointer CommonFunctionality::OpenVolumeOrSliceStack()
{
  mitk::DataTreeNode::Pointer newNode = NULL;

  QString fileName = QFileDialog::getOpenFileName(NULL,mitk::CoreObjectFactory::GetInstance()->GetFileExtensions() );
  if ( !fileName.isNull() )
  {
    newNode = CommonFunctionality::FileOpen(fileName);
    if (newNode.IsNotNull())
    {
      mitk::Image::Pointer imageData = dynamic_cast<mitk::Image*> (newNode->GetData()) ;
      if (imageData.IsNull()) return NULL;

      if (imageData->GetDimension(2) == 1)
      {
        //        std::string dir = itksys::SystemTools::GetFilenamePath( std::string(fileName.ascii()) )
        newNode = CommonFunctionality::FileOpenImageSequence(fileName);
        imageData = dynamic_cast<mitk::Image*> (newNode->GetData());
      }
      return newNode;
    }
  }
  {
    return NULL;
  }
}
mitk::DataTreeIteratorBase* CommonFunctionality::GetIteratorToFirstImage(mitk::DataTreeIteratorBase* dataTreeIterator)
{    
  mitk::DataTreeIteratorClone it = dataTreeIterator;
  while ( !it->IsAtEnd() )
  {
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node->GetData() != NULL )
    {
      // access the original image
      mitk::Image::Pointer img = dynamic_cast<mitk::Image*>( node->GetData() );

      // enquiry whether img is NULL
      if ( img.IsNotNull() )
      {
        return it->Clone();
      }
    }
    ++it;
  }
  std::cout << "No node containing an mitk::Image found, returning NULL..." << std::endl;
  return NULL;
}

mitk::DataTreeIteratorBase* CommonFunctionality::GetIteratorToFirstImageInDataTree(mitk::DataTree::Pointer dataTree)
{
  mitk::DataTreePreOrderIterator dataTreeIterator( dataTree );

  if ( dataTree.IsNull() )
  {
    std::cout << "iterator to data tree is NULL. I cannot work without datatree !!"  << std::endl;
    return NULL;
  }

  return GetIteratorToFirstImage(&dataTreeIterator);
}

mitk::Image* CommonFunctionality::GetFirstImageInDataTree(mitk::DataTree::Pointer dataTree)
{
  mitk::DataTreeIteratorClone it = GetIteratorToFirstImageInDataTree(dataTree);
  if(it.IsNull())
    return NULL;
  return static_cast<mitk::Image*>(it->Get()->GetData());
}
mitk::DataTreeNode* CommonFunctionality::GetFirstNodeByProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
{
  mitk::DataTreeIteratorClone pos = dynamic_cast<mitk::DataTree*>( it->GetTree() )->GetNext( propertyKey.c_str(), property, it.GetPointer() );
  if ( ! pos->IsAtEnd() )
  {
    return pos->Get();
  }
  else
  {
    return NULL;
  }

}
mitk::BaseData* CommonFunctionality::GetFirstDataByProperty( mitk::DataTreeIteratorClone it, std::string propertyKey, mitk::BaseProperty* property )
{
  mitk::DataTreeNode* node = GetFirstNodeByProperty( it, propertyKey, property );
  if ( node == NULL )
  {
    return NULL;
  }
  else
  {
    return node->GetData();
  }
}

mitk::DataTreeNode* CommonFunctionality::GetNodeForData( mitk::DataTreeIteratorClone it, mitk::BaseData* data )
{
  if ( it.GetPointer() == NULL )
  {
    return NULL;
  }

  mitk::DataTreeIteratorClone iteratorClone = it;
  while ( !iteratorClone->IsAtEnd() )
  {
    mitk::DataTreeNode::Pointer node = iteratorClone->Get();
    if ( node.IsNotNull() )
    {
      if ( node->GetData() == data )
        return node.GetPointer();
    }
    ++iteratorClone;
  }
  return NULL;
}

CommonFunctionality::DataTreeIteratorVector CommonFunctionality::FilterNodes(mitk::DataTreeIteratorClone it, bool (* FilterFunction)(mitk::DataTreeNode*))
{

  DataTreeIteratorVector result;

  if ( it.GetPointer() != NULL )
  {

    mitk::DataTreeIteratorClone iteratorClone = it;
    while ( !iteratorClone->IsAtEnd() )
    {
      mitk::DataTreeNode::Pointer node = iteratorClone->Get();
      if ( (FilterFunction == NULL) || FilterFunction( node ) )
      {
        result.push_back(iteratorClone);
      }
      ++iteratorClone;
    }
  }
  return result;

}

#include "mitkSurfaceVtkWriter.h"
#include <vtkSTLWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataWriter.h>

std::string CommonFunctionality::SaveSurface(mitk::Surface* surface, const char* aFileName)
{
  std::string fileName;
  if(aFileName == NULL)
    fileName = "Surface";
  else
    fileName = aFileName;

  std::string selectedItemsName = itksys::SystemTools::GetFilenameWithoutExtension(fileName);
  selectedItemsName += ".stl";
  QString qfileName = QFileDialog::getSaveFileName(QString(selectedItemsName.c_str()),"Surface Data(*.stl *.vtk)");
  if (qfileName != NULL )
  {
    if(qfileName.endsWith(".stl")==true)
    {
      mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
      writer->SetInput( surface );
      writer->SetFileName(qfileName.latin1());
      writer->GetVtkWriter()->SetFileTypeToBinary();
      writer->Write();
    }
    else
    if(qfileName.endsWith(".vtp")==true)
    {
      mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::New();
      writer->SetInput( surface );
      writer->SetFileName(qfileName.latin1());
      writer->GetVtkWriter()->SetDataModeToBinary();
      writer->Write();
    }
    else
    {
      if (qfileName.endsWith(".vtk")==false)
        qfileName += ".vtk";
      mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer writer=mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
      writer->SetInput( surface );
      writer->SetFileName(qfileName.latin1());
      writer->Write();
    }
    fileName = qfileName.ascii();
  }
  else
  {
    fileName.clear();
  }
  return fileName;
}

#include "mitkImageWriter.h"
#include <itksys/SystemTools.hxx>

std::string CommonFunctionality::SaveImage(mitk::Image* image, const char* aFileName, bool askForDifferentFilename)
{
  static QString lastDirectory = "";

  std::string fileName;
  if(aFileName == NULL || askForDifferentFilename)
  {
    QString initialFilename(aFileName);
    if (initialFilename.isEmpty()) initialFilename = "NewImage.pic";

    // prepend the last directory
    initialFilename = lastDirectory + initialFilename;
    QString qfileName = QFileDialog::getSaveFileName( initialFilename ,mitk::CoreObjectFactory::GetInstance()->GetSaveFileExtensions());
    if (qfileName == NULL )
      return "";
    fileName = qfileName.ascii();
  }
  else
    fileName = aFileName;

  try
  {
    std::string dir = itksys::SystemTools::GetFilenamePath( fileName );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( fileName );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( fileName );

    if (extension == "")
      extension = ".pic";

    if (extension == ".gz")
    {
      QMessageBox::critical( NULL, "SaveDialog", "Warning: You can not save an image in the compressed \n"
                                                 ".pic.gz format. You must save as a normal .pic file.\n"
                                                 "Please press Save again and choose a filename with a .pic ending.",
                                                 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      return "";
    }
    dir += "/";
    lastDirectory = dir.c_str(); // remember path for next save dialog
    dir += baseFilename;

    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
    imageWriter->SetInput(image);
    imageWriter->SetFileName(dir.c_str());
    imageWriter->SetExtension(extension.c_str());
    imageWriter->Write();
  }
  catch ( itk::ExceptionObject &err)
  {
    itkGenericOutputMacro( << "Exception during write: " << err );
    return "";
  }
  catch ( ... )
  {
    itkGenericOutputMacro( << "Unknown type of exception during write" );
  }
  return fileName;
}


std::string CommonFunctionality::SaveScreenshot( vtkRenderWindow* renderWindow , const char* filename )
{
  //
  // perform some error checking
  //
  if ( ! renderWindow )
  {
    itkGenericOutputMacro( << "render window is NULL!" );
    return std::string("");
  }
  if ( ! renderWindow )
  {
    itkGenericOutputMacro( << "Unsupported type of render window! The only supported type is currently QmitkRenderWindow." );
    return std::string("");
  }
  
  //
  // create the screenshot before the filechooser is opened,
  // so there the file chooser will not be part of the screenshot
  //
  //QPixmap buffer = QPixmap::grabWindow( qtRenderWindow->winId() );

  // new Version: 
  vtkWindowToImageFilter* wti = vtkWindowToImageFilter::New();
	// take screenshot of render window without the coloured frame of 2 pixels
	int* windowSize = renderWindow->GetSize();
	double* renderWindowSize = new double[2];
	renderWindowSize[0] = windowSize[0];
	renderWindowSize[1] = windowSize[1];
	double framesize = 5;
	double* viewport = new double[4];
	viewport[0] = (double)(framesize/renderWindowSize[0]);
	viewport[1] = (double)(framesize/renderWindowSize[1]);
	viewport[2] = (double)((renderWindowSize[0]-framesize)/renderWindowSize[0]);
	viewport[3] = (double)((renderWindowSize[1]-framesize)/renderWindowSize[1]);
	wti->SetViewport(viewport);
  vtkPNGWriter* pngWriter = vtkPNGWriter::New();
  
  //
  // if the provided filename is empty ask the user 
  // for the name of the file in which the screenshot
  // should be saved
  //
  std::string concreteFilename = "";
  if( filename == NULL )
  {
    //
    // show a file selector with the supported file formats
    //
    QString qfileName = QFileDialog::getSaveFileName( QString( "" ), QString( ".png" ).lower() );
    if ( qfileName == NULL )
      return "";
    concreteFilename = qfileName.latin1();
  }
  else
    concreteFilename = filename;

  // make sure the filename ends with .png
  const std::string outFileSuffix("png");
  std::string::size_type pos = concreteFilename.rfind('.');

  if ( pos == std::string::npos )
    concreteFilename = concreteFilename + '.' + outFileSuffix;
  else 
  {
    std::string extname = concreteFilename.substr(pos+1);
    if ( extname.empty() ) concreteFilename += outFileSuffix; // name ended with '.'
    if ( !(extname == outFileSuffix) )
      concreteFilename.replace( pos+1, std::string::npos, "png" );
  }

  //
  // wait for 500 ms to let the file chooser close itself
  //  
  // int msecs = 500;
  // clock_t ticks = ( clock_t )( ( ( ( float ) msecs ) / 1000.0f ) * ( ( float ) CLOCKS_PER_SEC ) );
  // clock_t goal = ticks + std::clock();
  // while ( goal > std::clock() );
  //
  
  //
  // save the screenshot under the given filename
  //

  wti->SetInput( renderWindow );
  pngWriter->SetInput( wti->GetOutput() );
  pngWriter->SetFileName( concreteFilename.c_str() );
  
  pngWriter->Write();

  if ( pngWriter->GetErrorCode() != 0 )
    QMessageBox::information(NULL, "Save Screenshot...", "The file could not be saved. Please check filename, format and access rights...");

  wti->Delete();
  pngWriter->Delete();
  return concreteFilename;  
}
