#ifndef MVASYNCGLOBJECT_H
#define MVASYNCGLOBJECT_H

#include "mvGLObject.h"

#include <future>

/**
 * @brief The mvAsyncGLObject class implements an asynchronous mvGLObject.
 *
 * mvAsyncGLObject provides a mvGLObject implementation that executes a data
 * pipeline in a background thread. This allows a separate rendering pipeline
 * to continue rendering stale data until the recomputed objects are ready.
 *
 * This class provides a final implementation of syncApplicationState, which
 * manages all interactions with the data pipeline. Subclasses implement their
 * data pipelines via the following virtuals:
 *
 * - void mvAsyncGLObject::configureDataPipeline(const mvApplicationState &)
 * - bool mvAsyncGLObject::dataPipelineNeedsUpdate() const
 * - void mvAsyncGLObject::executeDataPipeline() const
 * - void mvAsyncGLObject::retrieveDataPipelineResult()
 *
 * For proper usage, the data pipeline objects must only be modified from within
 * configureDataPipeline. Pipeline options should be stored in ivars and
 * synchronized with pipeline during this call, rather than having
 * setters/getters access pipeline state directly. This is because the pipeline
 * may be executing when the setter is called, leading to undefined behavior.
 */
class mvAsyncGLObject : public mvGLObject
{
public:
  typedef mvGLObject Superclass;

  struct DataItem : public Superclass::DataItem
  { /* Placeholder for future shared state. */ };

  mvAsyncGLObject();
  ~mvAsyncGLObject();

  /**
   * Implements thread handling and data syncing. Note that this implementation
   * is final -- subclasses may not reimplement this.
   */
  void syncApplicationState(const mvApplicationState &appState) final;

private: // Virtual API:

  /**
   * Update the data pipeline objects here. Ensure that any vtkObjects touched
   * here will not change their modification times unless the parameters really
   * have changed.
   */
  virtual void configureDataPipeline(const mvApplicationState &state) = 0;

  /**
   * Return true if the pipeline needs to be updated. A typical implementation
   * compares the modification times of cached data objects to the filters that
   * produced them.
   */
  virtual bool dataPipelineNeedsUpdate() const = 0;

  /**
   * Execute the data pipeline. This is called in a background thread. A typical
   * implementation simply calls 'Update()' on the data pipeline's sink
   * vtkAlgorithms.
   */
  virtual void executeDataPipeline() const = 0;

  /**
   * Retrieve the output data objects from the data pipeline. A typical
   * implementation will store them as ivars of the subclass so that they will
   * be available for rendering.
   *
   * To ensure that future pipeline updates will not modify the rendered object,
   * the following pattern should be used:
   *
   * @code
   * vtkDataObject *dataPipelineOutput = ...;
   * vtkSmartPointer<vtkDataObject> renderPipelineInput = ...;
   *
   * renderedPipelineInput.TakeReference(dataPipelineOutput->NewInstance());
   * renderedPipelineInput->ShallowCopy(dataPipelineOutput);
   * @endcode
   *
   * This will ensure that background modifications to dataPipelineOutput
   * (during future executions of the data pipeline) will be isolated from the
   * reference counted internals of the rendered object, and avoids the need to
   * deep copy the data.
   *
   */
  virtual void retrieveDataPipelineResult() = 0;

private:
  /**
   * Wrapper around the data pipeline update call. Ensures that a new frame is
   * requested when the pipeline updates.
   */
  void internalExecutePipeline() const;

  std::future<void> m_monitor;
};

#endif // MVASYNCGLOBJECT_H
