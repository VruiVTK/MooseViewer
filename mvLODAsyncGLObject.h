#ifndef MVLODASYNCGLOBJECT_H
#define MVLODASYNCGLOBJECT_H

#include "mvGLObject.h"

#include <vtkNew.h>

#include <future>
#include <type_traits>

class mvProgressCookie;

/**
 * @brief The mvLODAsyncGLObject class manages scene objects with multiple
 * levels of detail, which may be updated asynchronously.
 *
 * Together, the nested classes DataPipeline, LODData, and RenderPipeline form
 * a LOD when subclassed. The appropriate implementations must be returned
 * from the create* virtual methods.
 *
 * Rather than storing mvGLObject state in member variables, it should be
 * stored in a subclass of mvLODAsyncGLObject::ObjectState, which is passed
 * to the LOD components as needed with correct const-ness to ensure that
 * asynchronous updates occur safely. This state can be accessed through the
 * objectState() method.
 */
class mvLODAsyncGLObject : public mvGLObject
{
public:
  using Superclass = mvGLObject;

  /**
   * Enumerates the available levels of detail in order from best to fastest.
   */
  enum class LevelOfDetail
    {
    HiRes = 0,
    LoRes,
    Hint,

    Count,
    NoLOD = -1,
    };

  /**
   * Subclass this to store internal data for this mvGLObject. Parameters
   * should be stored here so that they can be passed to the LOD pipelines
   * safely. There should only be one per mvLODAsyncGLObject subclass. It is
   * accessable by the protected template function objectState().
   */
  struct ObjectState
  {
    /**
     * Sync object with application state. This is the spot to access
     * mvInteractor, etc.
     */
    virtual void update(const mvApplicationState &state) = 0;
  };

  /**
   * Subclass this to store LOD data that is passed between the data
   * and rendering pipelines. Typical VTK objects are best stored in a
   * vtkSmartPointer<vtkDataObject>.
   */
  struct LODData { };

  /**
   * Subclass this to implement a data pipeline for a single LOD.
   */
  struct DataPipeline
  {
    virtual ~DataPipeline();

    /** If true, the pipeline will update in the UI thread instead of async. */
    virtual bool forceSynchronousUpdates() const { return false; }

    /** Used to configure this data pipeline from the app and object states. */
    virtual void configure(const ObjectState &objState,
                           const mvApplicationState &appState) = 0;

    /** Return true if the pipeline needs to re-execute. */
    virtual bool needsUpdate(const ObjectState &objState,
                             const LODData &result) const = 0;

    /** Execute the data pipeline here. May execute asynchronously. */
    virtual void execute() = 0;

    /** Copy the data pipeline's outputs to @a result.
     */
    virtual void exportResult(LODData &result) const = 0;
  };

  /**
   * Subclass this to implement a rendering pipeline for a single LOD.
   */
  struct RenderPipeline
  {
    virtual ~RenderPipeline();

    /**
     * Initialize the rendering pipeline state. Typically this is used to add
     * vtkActors to the renderer in @a contextState.
     */
    virtual void init(const ObjectState &objState,
                      mvContextState &contextState) = 0;

    /**
     * Called to enable this rendering pipeline. All relevant props should be
     * made visible and synchronized with the various provided states.
     */
    virtual void update(const ObjectState &objState,
                        const mvApplicationState &appState,
                        const mvContextState &contextState,
                        const LODData &result) = 0;

    /**
     * Called to disable an LOD. All relevant props should have visibility
     * turned off.
     */
    virtual void disable() = 0;
  };

public:
  mvLODAsyncGLObject();
  ~mvLODAsyncGLObject();

  void init(const mvApplicationState &appState) override;

  void initMvContext(mvContextState &contextState,
                     GLContextData &contextData) const final;

  void syncApplicationState(const mvApplicationState &state) final;


  void syncContextState(const mvApplicationState &appState,
                        const mvContextState &contextState,
                        GLContextData &contextData) const final;

  /** Set true to print update timing information to stderr. @{ */
  bool benchmark() const { return m_benchmark; }
  void setBenchmark(bool benchmark) { m_benchmark = benchmark; }
  /** @} */

protected:

  /**
   * Return the object state for this mvLODAsyncGLObject. The template parameter
   * is a convenience to static cast the result into the proper subclass.
   * @{
   */
  template <typename T = ObjectState>
  T& objectState() { return *static_cast<T*>(m_objState); }
  template <typename T = ObjectState>
  const T& objectState() const { return *static_cast<T*>(m_objState); }
  /** @} */

private: // vtkLODAsyncGLObject virtual API:

  /**
   * The string returned here will be used to label the background indicator.
   * See mvProgress.
   */
  virtual std::string progressLabel() const = 0;

  /**
   * Create and return an instance of an ObjectState subclass that will be
   * used to hold this mvLODAsyncGLObject's state.
   */
  virtual ObjectState* createObjectState() const = 0;

  /**
   * Create and return an instance of DataPipeline for the requested @a lod.
   */
  virtual DataPipeline* createDataPipeline(LevelOfDetail lod) const = 0;

  /**
   * Create and return an instance of RenderPipeline for the requested @a lod.
   * Note that this will not be called if createDataPipeline return nullptr for
   * this @a lod.
   */
  virtual RenderPipeline* createRenderPipeline(LevelOfDetail lod) const = 0;

  /**
   * Create and return an instance of LODData for the requested @a lod.
   * Note that this will not be called if createDataPipeline return nullptr for
   * this @a lod.
   */
  virtual LODData* createLODData(LevelOfDetail lod) const = 0;

private: // Private nested classes, implementation  details, etc:

  // These can be iterated over best->fast:
  static constexpr LevelOfDetail BestDetail { LevelOfDetail::HiRes };
  static constexpr LevelOfDetail FastDetail { LevelOfDetail::Hint };

  /**
   * Used to store per-LOD state, just an array of length LevelOfDetail::Count.
   */
  template <typename T>
  using LODArray = std::array<T, static_cast<size_t>(LevelOfDetail::Count)>;

  /**
   * The DataItem for mvLODAsyncGLObject. This should NOT be subclassed by
   * subclasses -- use RenderPipeline for context-specific state instead.
   */
  struct DataItem : public Superclass::DataItem
  {
    DataItem() { renderPipelines.fill(nullptr); }
    ~DataItem();

    RenderPipeline *renderPipeline(LevelOfDetail lod)
    {
      return renderPipelines[static_cast<size_t>(lod)];
    }

    LODArray<RenderPipeline*> renderPipelines;
  };

  /**
   * Possible sync states for LODs.
   */
  enum class LODStatus
    {
    Invalid, /// nullptr returned from createDataPipeline(lod).
    OutOfDate, /// Pipeline needs updating.
    Updating, /// Pipeline is executing.
    UpToDate /// Pipeline is up-to-date.
    };

  /**
   * Contains data pipeline details for a single LOD.
   */
  struct DataPipelineManager
  {
    ~DataPipelineManager();
    LODStatus status{LODStatus::Invalid};
    DataPipeline *dataPipeline{nullptr};
    LODData *result{nullptr};
    std::future<void> monitor;
    mvProgressCookie *cookie{nullptr};
  };

  /**
   * Iterator type.
   * @{
   */
  template <typename ManagerArrayType> struct LODIteratorImpl;
  using iterator = LODIteratorImpl<LODArray<DataPipelineManager>&>;
  using const_iterator = LODIteratorImpl<const LODArray<DataPipelineManager>&>;
  /** @} */

private: // Private methods:

  /**
   * Create an iterator to traverse the LODs from best to fastest. If
   * skipInvalid is true (default), unused LODs will not be considered during
   * initialization and traversal.
   * @{
   */
  iterator bestToFast(bool skipInvalid = true);
  const_iterator bestToFast(bool skipInvalid = true) const;
  /** @} */

  /**
   * Create an iterator to traverse the LODs from best to fastest. If
   * skipInvalid is true (default), unused LODs will not be considered during
   * initialization and traversal.
   * @{
   */
  iterator fastToBest(bool skipInvalid = true);
  const_iterator fastToBest(bool skipInvalid = true) const;
  /** @} */

  /**
   * Wrapper to call Vrui::requestUpdate after a pipeline finishes. Used for
   * background updates.
   */
  void executeWrapper(LevelOfDetail lod, DataPipeline *pipeline);

private: // Data members:

  // Object state object:
  ObjectState *m_objState{nullptr};

  // Data Pipelines and state:
  LODArray<DataPipelineManager> m_dataPipelines;

  // Enable to print update benchmark timings.
  bool m_benchmark;
};

/**
 * Iterate through LODs from best-to-fast or fast-to-best, depending on
 * Direction, skipping undefined LODs when skip is true (default=true).
 *
 * Dereferencing the iterator with *lod or lod-> will access the appropriate
 * DataPipelineManager.
 *
 * The iterator can be implicitly cast to a LevelOfDetail enum type (returns
 * the current LOD), bool (returns true until the iterator is exhausted), or
 * size_t (returns the LOD as an index, for use with LODArray).
 */
template <typename ManagerArrayType>
struct mvLODAsyncGLObject::LODIteratorImpl
{
  enum class Direction { BestToFast, FastToBest };

  LODIteratorImpl(Direction dir, ManagerArrayType mgrs, bool skip = true)
    : m_skipInvalid(skip),
      m_direction(dir),
      m_managers(mgrs),
      m_current(m_direction == Direction::BestToFast ? m_best : m_fast)
  {
    // Find the first valid LOD:
    if (m_skipInvalid && (*this)->status == LODStatus::Invalid)
      {
      this->next();
      }
  }

  /** Jump to the fastest LOD. */
  void jumpToFast()
  {
    m_current = m_fast;
    if (m_skipInvalid && (*this)->status == LODStatus::Invalid)
      {
      if (m_direction == Direction::BestToFast)
        {
        this->prev();
        }
      else
        {
        this->next();
        }
      }
  }

  /** Jump to the best LOD. */
  void jumpToBest()
  {
    m_current = m_best;
    if (m_skipInvalid && (*this)->status == LODStatus::Invalid)
      {
      if (m_direction == Direction::BestToFast)
        {
        this->next();
        }
      else
        {
        this->prev();
        }
      }
  }

  /** Implicit cast to LevelOfDetail enum value. */
  operator LevelOfDetail () const { return m_current; }

  /** Implicit cast to size_t (For use with LODArrays). */
  operator size_t () const { return static_cast<size_t>(m_current); }

  /** Explicit dereferences return the current DataPipelineManager. @{ */
  const DataPipelineManager& operator*() const
  { return m_managers[static_cast<size_t>(m_current)]; }
  const DataPipelineManager* operator->() const
  { return &m_managers[static_cast<size_t>(m_current)]; }

  // Non-const versions of the dereference operators are only enabled if
  // ManagerArrayType is not const. Extra template mumbo-jumbo is to get
  // SFINAE to work using enable_if on a class template parameter. Eww.
  template <typename T = ManagerArrayType>
  typename std::enable_if<
    !std::is_const<typename std::remove_reference<T>::type >::value,
    DataPipelineManager&
  >::type
  operator*()
  { return m_managers[static_cast<size_t>(m_current)]; }

  template <typename T = ManagerArrayType>
  typename std::enable_if<
    !std::is_const<typename std::remove_reference<T>::type >::value,
    DataPipelineManager*
  >::type
  operator->()
  { return &m_managers[static_cast<size_t>(m_current)]; }
  /** @} */

  /** Returns false when the iterator is exhausted. */
  operator bool () const { return !this->isDone(); }

  /** Increment/decrement iterator. May skip Invalid LODs. @{ */
  LODIteratorImpl& operator++ () { this->next(); }
  LODIteratorImpl& operator-- () { this->prev(); }

  LODIteratorImpl operator++ (int)
  {
    LODIteratorImpl<ManagerArrayType> result(*this);
    this->next();
    return result;
  }

  LODIteratorImpl operator-- (int)
  {
    LODIteratorImpl<ManagerArrayType> result(*this);
    this->prev();
    return result;
  }

  /** @} */

private:

  using EnumIntType = std::underlying_type<LevelOfDetail>::type;

  bool isDone() const { return m_current < m_best || m_current > m_fast; }

  void next()
  {
    do
      {
      reinterpret_cast<EnumIntType&>(m_current) +=
          (m_direction == Direction::BestToFast ? +1 : -1);
      }
    while (m_skipInvalid && (*this)->status == LODStatus::Invalid);
  }

  void prev()
  {
    do
      {
      reinterpret_cast<EnumIntType&>(m_current) +=
          (m_direction == Direction::BestToFast ? -1 : +1);
      }
    while (m_skipInvalid && (*this)->status == LODStatus::Invalid);
  }

  const LevelOfDetail m_best{BestDetail};
  const LevelOfDetail m_fast{FastDetail};

  bool m_skipInvalid;
  Direction m_direction;
  ManagerArrayType m_managers;
  LevelOfDetail m_current;
};

//------------------------------------------------------------------------------
inline mvLODAsyncGLObject::iterator
mvLODAsyncGLObject::bestToFast(bool skipInvalid)
{
  return iterator(iterator::Direction::BestToFast, m_dataPipelines,
                  skipInvalid);
}

//------------------------------------------------------------------------------
inline mvLODAsyncGLObject::const_iterator
mvLODAsyncGLObject::bestToFast(bool skipInvalid) const
{
  return const_iterator(const_iterator::Direction::BestToFast, m_dataPipelines,
                        skipInvalid);
}

//------------------------------------------------------------------------------
inline mvLODAsyncGLObject::iterator
mvLODAsyncGLObject::fastToBest(bool skipInvalid)
{
  return iterator(iterator::Direction::FastToBest, m_dataPipelines,
                  skipInvalid);
}

//------------------------------------------------------------------------------
inline mvLODAsyncGLObject::const_iterator
mvLODAsyncGLObject::fastToBest(bool skipInvalid) const
{
  return const_iterator(const_iterator::Direction::FastToBest, m_dataPipelines,
                        skipInvalid);
}

#endif // MVLODASYNCGLOBJECT_H
