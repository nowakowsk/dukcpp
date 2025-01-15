#ifndef DUKCPP_TEST_CHARACTER_H
#define DUKCPP_TEST_CHARACTER_H

#include "vector.h"
#include <duk/class.h>


struct Character
{
  enum class Type
  {
    NEUTRAL,
    ENEMY,
    FRIEND
  };

  const bool& active() const
  {
    return active_;
  }

  void active(const bool& value)
  {
    active_ = value;
  }

  int id = 0;
  std::string name;
  Type type = Type::NEUTRAL;
  Vector position;

private:
  bool active_ = false;
};


// Returns prototype object handle
void* registerCharacter(duk_context* ctx, duk_idx_t idx);


#endif // DUKCPP_TEST_CHARACTER_H
