#include "character.h"
#include <duk/duk.h>


namespace duk
{


void* class_traits<Character>::prototype_heap_ptr([[maybe_unused]] duk_context* ctx)
{
  return prototype;
}


void* class_traits<Character>::prototype = nullptr;


} // namespace duk


void* registerCharacter(duk_context* ctx, duk_idx_t idx)
{
  duk::push_function<
    duk::ctor<Character>
  >(ctx);

  // Enum definition goes into constructor function, not prototype object.
  duk::def_prop_enum(ctx, -1, "Type",
    "NEUTRAL", Character::Type::NEUTRAL,
    "ENEMY", Character::Type::ENEMY,
    "FRIEND", Character::Type::FRIEND
  );

  duk_push_object(ctx); // prototype

  auto prototypeHandle = duk_get_heapptr(ctx, -1);

  duk::def_prop<&Character::id>(ctx, -1, "id");
  duk::def_prop<&Character::name>(ctx, -1, "name");
  duk::def_prop<&Character::active>(ctx, -1, "active");
  duk::def_prop<&Character::type>(ctx, -1, "type");
  duk::def_prop<&Character::position>(ctx, -1, "position");

  duk_put_prop_string(ctx, -2, "prototype");

  duk_put_prop_string(ctx, idx - 1, "Character");

  duk::class_traits<Character>::prototype = prototypeHandle;

  return prototypeHandle;
}
