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

  int id = 0;
  std::string name;
  bool active = false;
  Type type = Type::NEUTRAL;
  Vector position;
};


namespace duk
{


template<>
struct class_traits<Character>
{
  static void* prototype_heap_ptr(duk_context* ctx);

  static void* prototype;
};


} // namespace duk


// Returns prototype object handle
void* registerCharacter(duk_context* ctx, duk_idx_t idx);


#endif // DUKCPP_TEST_CHARACTER_H
