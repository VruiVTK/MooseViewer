#include "mvGLObject.h"

#include <iostream>
#include <cstdlib>

//------------------------------------------------------------------------------
mvGLObject::mvGLObject()
  : GLObject(/*autoInit=*/ false)
{
}

//------------------------------------------------------------------------------
mvGLObject::~mvGLObject()
{
}

//------------------------------------------------------------------------------
void mvGLObject::initContext(GLContextData &) const
{
  std::cerr << "mvGLObject::initContext should never be called. Use "
               "mvGLObject::initMvContext instead." << std::endl;
  abort();
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
