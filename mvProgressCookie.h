#ifndef MVPROGRESSCOOKIE_H
#define MVPROGRESSCOOKIE_H

#include <array>
#include <string>

class mvProgress;

/**
 * @brief The mvProgressCookie class is used to manage mvProgress notifications.
 */
class mvProgressCookie
{
public:
  std::string text() const { return m_text; }
  void setText(std::string text) { m_text = text; }

protected:
  friend class mvProgress;

  explicit mvProgressCookie(std::string text);
  ~mvProgressCookie();

private:
  std::string m_text;
};

#endif // MVPROGRESSCOOKIE_H
