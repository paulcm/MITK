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

#include "mitkFastMarchingTool3D.h"
#include "mitkToolManager.h"


#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkApplicationCursor.h"

//#include "mitkFastMarchingTool3D.xpm"
#include "mitkInteractionConst.h"

#include "itkOrImageFilter.h"
#include "mitkImageTimeSelector.h"



namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FastMarchingTool3D, "FastMarching tool");
}


mitk::FastMarchingTool3D::FastMarchingTool3D()
:FeedbackContourTool("PressMoveReleaseAndPointSetting"),
m_NeedUpdate(true),
m_CurrentTimeStep(0),
m_LowerThreshold(0),
m_UpperThreshold(200),
m_InitialLowerThreshold(0.0),
m_InitialUpperThreshold(100.0),
m_InitialStoppingValue(100),
m_StoppingValue(100),
m_Sigma(1.0),
m_Alpha(-0.5),
m_Beta(3.0)
{
  CONNECT_ACTION( AcADDPOINTRMB, OnAddPoint );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcREMOVEPOINT, OnDelete );

  m_ReferenceImageAsITK = InternalImageType::New();

  m_ProgressCommand = mitk::ToolCommand::New();

  m_ThresholdFilter = ThresholdingFilterType::New();
  m_ThresholdFilter->SetLowerThreshold( m_InitialLowerThreshold );
  m_ThresholdFilter->SetUpperThreshold( m_InitialUpperThreshold );
  m_ThresholdFilter->SetOutsideValue( 0 );
  m_ThresholdFilter->SetInsideValue( 1.0 );

  m_SmoothFilter = SmoothingFilterType::New();
  m_SmoothFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_SmoothFilter->SetTimeStep( 0.05 );
  m_SmoothFilter->SetNumberOfIterations( 2 );
  m_SmoothFilter->SetConductanceParameter( 9.0 );

  m_GradientMagnitudeFilter = GradientFilterType::New();
  m_GradientMagnitudeFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_GradientMagnitudeFilter->SetSigma( m_Sigma );

  m_SigmoidFilter = SigmoidFilterType::New();
  m_SigmoidFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_SigmoidFilter->SetAlpha( m_Alpha );
  m_SigmoidFilter->SetBeta( m_Beta );
  m_SigmoidFilter->SetOutputMinimum( 0.0 );
  m_SigmoidFilter->SetOutputMaximum( 1.0 );

  m_FastMarchingFilter = FastMarchingFilterType::New();
  m_FastMarchingFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_FastMarchingFilter->SetStoppingValue( m_InitialStoppingValue );

  m_SeedContainer = NodeContainer::New();
  m_SeedContainer->Initialize();
  m_FastMarchingFilter->SetTrialPoints( m_SeedContainer );

  //set up pipeline
  m_SmoothFilter->SetInput( m_ReferenceImageAsITK );
  m_GradientMagnitudeFilter->SetInput( m_SmoothFilter->GetOutput() );
  m_SigmoidFilter->SetInput( m_GradientMagnitudeFilter->GetOutput() );
  m_FastMarchingFilter->SetInput( m_SigmoidFilter->GetOutput() );
  m_ThresholdFilter->SetInput( m_FastMarchingFilter->GetOutput() );
}

mitk::FastMarchingTool3D::~FastMarchingTool3D()
{
  m_ReferenceSlice = NULL;
  m_WorkingSlice = NULL;
}


float mitk::FastMarchingTool3D::CanHandleEvent( StateEvent const *stateEvent) const
{
  float returnValue = Superclass::CanHandleEvent(stateEvent);

  //we can handle delete
  if(stateEvent->GetId() == 12 )
  {
    returnValue = 1.0;
  }

  return returnValue;
}


const char** mitk::FastMarchingTool3D::GetXPM() const
{
  return NULL;//mitkFastMarchingTool3D_xpm;
}

std::string mitk::FastMarchingTool3D::GetIconPath() const
{
  return ":/Segmentation/FastMarching_48x48.png";
}

const char* mitk::FastMarchingTool3D::GetName() const
{
  return "FastMarching3D";
}

void mitk::FastMarchingTool3D::SetUpperThreshold(double value)
{
  m_UpperThreshold = value / 10.0;
  m_ThresholdFilter->SetUpperThreshold( m_UpperThreshold );
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::SetLowerThreshold(double value)
{
  m_LowerThreshold = value / 10.0;
  m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::SetBeta(double value)
{
  m_Beta = value / 10.0;
  m_SigmoidFilter->SetBeta( m_Beta );
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::SetAlpha(double value)
{
  m_Alpha = value / 10.0;
  m_SigmoidFilter->SetAlpha( m_Alpha );
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::SetStoppingValue(double value)
{
  m_StoppingValue = value / 10.0;
  m_FastMarchingFilter->SetStoppingValue( m_StoppingValue );
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::Activated()
{
  Superclass::Activated();

  m_ResultImageNode = mitk::DataNode::New();
  m_ResultImageNode->SetName("FastMarching_Preview");
  m_ResultImageNode->SetBoolProperty("helper object", true);
  m_ResultImageNode->SetColor(0.0, 1.0, 0.0);
  m_ResultImageNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( this->m_ResultImageNode, m_ToolManager->GetReferenceData(0));

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("Seeds_Preview");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( this->m_SeedsAsPointSetNode, m_ToolManager->GetReferenceData(0));

  this->Initialize();
}

void mitk::FastMarchingTool3D::Deactivated()
{
  Superclass::Deactivated();
  m_ToolManager->GetDataStorage()->Remove( this->m_ResultImageNode );
  m_ToolManager->GetDataStorage()->Remove( this->m_SeedsAsPointSetNode );
  this->ClearSeeds();
  this->m_SmoothFilter->RemoveAllObservers();
  this->m_SigmoidFilter->RemoveAllObservers();
  this->m_GradientMagnitudeFilter->RemoveAllObservers();
  this->m_FastMarchingFilter->RemoveAllObservers();
  m_ResultImageNode = NULL;
}

void mitk::FastMarchingTool3D::Initialize()
{
  m_ReferenceSlice = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData());
  if(m_ReferenceSlice->GetTimeSlicedGeometry()->GetTimeSteps() > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
    timeSelector->SetInput( m_ReferenceSlice );
    timeSelector->SetTimeNr( m_CurrentTimeStep );
    timeSelector->UpdateLargestPossibleRegion();
    m_ReferenceSlice = timeSelector->GetOutput();
  }
  CastToItkImage(m_ReferenceSlice, m_ReferenceImageAsITK);
  m_SmoothFilter->SetInput( m_ReferenceImageAsITK );
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::ConfirmSegmentation()
{
  // combine preview image with current working segmentation
  if (dynamic_cast<mitk::Image*>(m_ResultImageNode->GetData()))
  {
    //logical or combination of preview and segmentation slice
    OutputImageType::Pointer segmentationImageInITK = OutputImageType::New();

    mitk::Image::Pointer workingImage = dynamic_cast<mitk::Image*>(this->m_ToolManager->GetWorkingData(0)->GetData());
    if(workingImage->GetTimeSlicedGeometry()->GetTimeSteps() > 1)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput( workingImage );
      timeSelector->SetTimeNr( m_CurrentTimeStep );
      timeSelector->UpdateLargestPossibleRegion();
      CastToItkImage( timeSelector->GetOutput(), segmentationImageInITK );
    }
    else
    {
      CastToItkImage( workingImage, segmentationImageInITK );
    }

    typedef itk::OrImageFilter<OutputImageType, OutputImageType> OrImageFilterType;
    OrImageFilterType::Pointer orFilter = OrImageFilterType::New();

    orFilter->SetInput(0, m_ThresholdFilter->GetOutput());
    orFilter->SetInput(1, segmentationImageInITK);
    orFilter->Update();

    //set image volume in current time step from itk image
    workingImage->SetVolume( (void*)(orFilter->GetOutput()->GetPixelContainer()->GetBufferPointer()), m_CurrentTimeStep);
    this->m_ResultImageNode->SetVisibility(false);
    this->ClearSeeds();
    workingImage->Modified();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


bool mitk::FastMarchingTool3D::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  // Add a new seed point for FastMarching algorithm
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  mitk::Point3D clickInIndex;

  m_ReferenceSlice->GetGeometry()->WorldToIndex(positionEvent->GetWorldPosition(), clickInIndex);
  itk::Index<3> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];
  seedPosition[2] = clickInIndex[2];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );
  this->m_SeedContainer->InsertElement(this->m_SeedContainer->Size(), node);
  m_FastMarchingFilter->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), positionEvent->GetWorldPosition());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  m_NeedUpdate = true;

  this->Update();

  return true;
}


bool mitk::FastMarchingTool3D::OnDelete(Action* action, const StateEvent* stateEvent)
{
  // delete last seed point
  if(!(this->m_SeedContainer->empty()))
  {
    //delete last element of seeds container
    this->m_SeedContainer->pop_back();
    m_FastMarchingFilter->Modified();

    //delete last point in pointset - somehow ugly
    m_SeedsAsPointSet->GetPointSet()->GetPoints()->DeleteIndex(m_SeedsAsPointSet->GetSize() - 1);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    m_NeedUpdate = true;
  }
  return true;
}


void mitk::FastMarchingTool3D::Update()
{
  // update FastMarching pipeline and show result
  if (m_NeedUpdate)
  {
    m_ProgressCommand->AddStepsToDo(20);
    CurrentlyBusy.Send(true);
    try
    {
      m_ThresholdFilter->UpdateLargestPossibleRegion();
    }
    catch( itk::ExceptionObject & excep )
    {
     MITK_ERROR << "Exception caught: " << excep.GetDescription();
     m_ProgressCommand->SetRemainingProgress(100);
     CurrentlyBusy.Send(false);
     return;
    }
    m_ProgressCommand->SetRemainingProgress(100);
    CurrentlyBusy.Send(false);
    //make output visible
    mitk::Image::Pointer result = mitk::Image::New();
    CastToMitkImage( m_ThresholdFilter->GetOutput(), result);
    result->GetGeometry()->SetOrigin(m_ReferenceSlice->GetGeometry()->GetOrigin() );
    result->GetGeometry()->SetIndexToWorldTransform(m_ReferenceSlice->GetGeometry()->GetIndexToWorldTransform() );
    m_ResultImageNode->SetData(result);
    m_ResultImageNode->SetVisibility(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void mitk::FastMarchingTool3D::ClearSeeds()
{
  // clear seeds for FastMarching as well as the PointSet for visualization
  this->m_SeedContainer->Initialize();
  this->m_SeedsAsPointSet->Clear();

  m_FastMarchingFilter->Modified();

  this->m_NeedUpdate = true;

  m_ResultImageNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void mitk::FastMarchingTool3D::Reset()
{
  //clear all seeds and preview empty result
  this->ClearSeeds();

  m_ResultImageNode->SetVisibility(false);
}

void mitk::FastMarchingTool3D::SetCurrentTimeStep(int t)
{
  if( m_CurrentTimeStep != t )
  {
    m_CurrentTimeStep = t;

    this->Initialize();
  }
}
