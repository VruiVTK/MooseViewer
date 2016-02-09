#include "mvGLObject.h"

//------------------------------------------------------------------------------
mvGLObject::mvGLObject()
{
}

//------------------------------------------------------------------------------
mvGLObject::~mvGLObject()
{
}

//------------------------------------------------------------------------------
void mvGLObject::initContext(GLContextData &) const
{
}

//------------------------------------------------------------------------------
void mvGLObject::initMvContext(mvContextState &, GLContextData &) const
{
}

//------------------------------------------------------------------------------
void mvGLObject::syncApplicationState(const mvApplicationState &)
{
}

//------------------------------------------------------------------------------
void mvGLObject::syncContextState(const mvApplicationState &,
                                  const mvContextState &,
                                  GLContextData &) const
{
}
