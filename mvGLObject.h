#ifndef MVGLOBJECT_H
#define MVGLOBJECT_H

#include <GL/GLObject.h>

class mvApplicationState;
class mvContextState;

/**
 * @brief The mvGLObject class extends the VRUI GLObject abstraction.
 *
 * mvGLObject extends VRUI's GLObject system for separating and maintaining
 * application / context state. It has defined synchronization points in the
 * MooseViewer app. Each subclass should manage one or more related props in
 * the rendered scene.
 *
 * Do not implement GLObject::initContext -- it will not be called, as auto
 * initializatoin is disabled for these classes. Instead, put all initialization
 * code into initMvContext.
 *
 * The goal of this class is to simplify the integration of VTK rendering
 * concepts into the VRUI GLObject abstraction.
 *
 * The synchronization points are:
 *
 * initMvContext: Called from MooseViewer::initContext. Use this to do one-time
 *                setup of this objects context-specific state with respect to
 *                the application's mvContextState (e.g. add actors to the
 *                renderer). This replaces the GLObject::initContext method.
 * syncApplicationState: Called from MooseViewer::frame. Update the
 *                       application-side data pipeline. Any data computations
 *                       that are shared between contexts should be performed
 *                       here, eventually producing renderable polydata.
 * syncContextState: Called from MooseViewer::display() prior to rendering.
 *                   This is where data should be transfered from the
 *                   application-side data pipeline into the context-side
 *                   rendering pipeline. At minimum, this will typically mean
 *                   taking the output polydata from the data pipeline and
 *                   connecting it to a mapper.
 *
 * At all stage, only the minimum work should be done; the data pipeline
 * should only re-execute when inputs changes, and the rendering pipeline
 * should only be updated when the application data changes.
 */
class mvGLObject : public GLObject
{
public:
  typedef GLObject Superclass;

  /**
   * All subclasses should inherit their DataItems from this.
   */
  struct DataItem : public Superclass::DataItem
  { /* Placeholder for future shared state. */  };

  mvGLObject();
  ~mvGLObject();

  /**
   * Do not use, this will not be executed. Put all context intialization in
   * initMvContext.
   */
  void initContext(GLContextData &contextData) const final;

  /**
   * Initialize object with respect to the MooseViewer app. This is the spot
   * to add actors to the context's renderer, etc. All initialization for
   * GLObject needs to happen here as well, such as adding an instance of
   * DataItem to GLContextData.
   */
  virtual void initMvContext(mvContextState &mvContext,
                             GLContextData &contextData) const;

  /**
   * Synchronization point between main application and object. Called per-frame
   * from MooseViewer::frame().
   */
  virtual void syncApplicationState(const mvApplicationState &state);

  /**
   * Prepare the context state for rendering. Called per-frame, per-context from
   * MooseViewer::display(), after syncing application state but before
   * rendering.
   */
  virtual void syncContextState(const mvApplicationState &appState,
                                const mvContextState &contextState,
                                GLContextData &contextData) const;
};

#endif // MVGLOBJECT_H
