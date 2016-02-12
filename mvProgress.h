#ifndef MVPROGRESS_H
#define MVPROGRESS_H

#include "mvGLObject.h"

#include <vtkNew.h>

#include <vector>

class mvProgressCookie;
class vtkTextActor;
class vtkTextProperty;

/**
 * @brief The mvProgress class notifies the user about background updates.
 *
 * When an asynchronous update starts, the user is shown stale data, or no data.
 * This mvGLObject displays an on-screen notification to the user, informing
 * them that some data is not yet available for rendering.
 *
 * Background processes indicate that they have begun a background by calling
 * addEntry, providing a string containing the text to be displayed in the
 * notification. addEntry returns a pointer to an mvProgressCookie, which
 * is used to update the notification text and remove the notification. The
 * notification is removed by passing the cookie back to removeEntry.
 *
 * Note that mvProgressCookie is not thread-safe, and any updates to it should
 * be performed from the GUI thread.
 */
class mvProgress : public mvGLObject
{
public:
  using Superclass = mvGLObject;

  struct DataItem : public Superclass::DataItem
  {
    DataItem();
    ~DataItem();

    vtkNew<vtkTextActor> actor;
  };

  mvProgress();
  ~mvProgress();

  /**
   * Create a new update notification entry with text @a text and obtain a
   * cookie.
   */
  mvProgressCookie* addEntry(std::string text);

  /**
   * Clear the notification text associated with the cookie.
   */
  void removeEntry(mvProgressCookie *cookie);

  virtual void initMvContext(mvContextState &mvContext,
                             GLContextData &contextData) const;
  virtual void syncApplicationState(const mvApplicationState &state);
  virtual void syncContextState(const mvApplicationState &appState,
                                const mvContextState &contextState,
                                GLContextData &contextData) const;

private:
  std::string m_text;
  vtkNew<vtkTextProperty> m_tprop;
  std::vector<mvProgressCookie*> m_entries;
};


#endif // MVPROGRESS_H
