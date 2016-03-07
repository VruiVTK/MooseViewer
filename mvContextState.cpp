#include "mvContextState.h"

#include <ExternalVTKWidget.h>
#include <vtkExternalOpenGLRenderer.h>

mvContextState::mvContextState()
{
  m_widget->GetRenderWindow()->AddRenderer(m_renderer.GetPointer());

  m_renderer->SetUseDepthPeeling(1);
  m_renderer->SetMaximumNumberOfPeels(8);
  m_renderer->SetOcclusionRatio(0.001);
}

mvContextState::~mvContextState()
{
}
