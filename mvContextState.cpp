#include "mvContextState.h"

#include <ExternalVTKWidget.h>
#include <vtkExternalOpenGLRenderer.h>

mvContextState::mvContextState()
{
  m_widget->GetRenderWindow()->AddRenderer(m_renderer.GetPointer());

  m_renderer->SetUseDepthPeeling(1);
  m_renderer->SetMaximumNumberOfPeels(4);
  m_renderer->SetOcclusionRatio(0.1);
}

mvContextState::~mvContextState()
{
}
