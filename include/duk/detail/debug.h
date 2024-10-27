#ifndef DUK_DETAIL_DEBUG_H
#define DUK_DETAIL_DEBUG_H

#include <duk/scoped_pop.h>
#include <cstdint>
#include <string>


namespace duk::detail
{


// inspectObject

static std::string inspectObject(duk_context* ctx, duk_idx_t objIdx)
{
  std::string objectInfo;

  duk::scoped_pop _(ctx); // duk_enum
  duk_enum(ctx, objIdx, DUK_ENUM_INCLUDE_NONENUMERABLE | DUK_ENUM_INCLUDE_HIDDEN | DUK_ENUM_INCLUDE_SYMBOLS);

  while (duk_next(ctx, -1, 1))
  {
    duk::scoped_pop _(ctx, 2); // duk_next
    objectInfo += std::string("[") + duk_safe_to_string(ctx, -2) + "] => [" + duk_safe_to_string(ctx, -1) + "]\n";
  }

  return objectInfo;
}


// inspectCurrentActivationStack

static std::string inspectCurrentActivationStack(duk_context* ctx)
{
  static constexpr const char* duktapeTypeIdToTypeName[] = {
    "DUK_TYPE_NONE",
    "DUK_TYPE_UNDEFINED",
    "DUK_TYPE_NULL",
    "DUK_TYPE_BOOLEAN",
    "DUK_TYPE_NUMBER",
    "DUK_TYPE_STRING",
    "DUK_TYPE_OBJECT",
    "DUK_TYPE_BUFFER",
    "DUK_TYPE_POINTER",
    "DUK_TYPE_LIGHTFUNC"
  };

  std::string stackInfo;

  for (duk_idx_t idx = 0; idx < duk_get_top(ctx); ++idx)
  {
    auto heapPtr = duk_get_heapptr(ctx, idx);

    stackInfo += "--- ";

    duk::scoped_pop _(ctx); // duk_inspect_value
    duk_inspect_value(ctx, idx);

    duk::scoped_pop __(ctx); // duk_get_prop_literal
    if (duk_get_prop_literal(ctx, -1, "type"))
    {
      auto objectType = duk_get_int(ctx, -1);
      stackInfo += std::to_string(idx) + " -> " + duktapeTypeIdToTypeName[objectType] +
                   " (heap_ptr: " + std::to_string(reinterpret_cast<std::uintptr_t>(heapPtr)) + ")\n";

      if(objectType == DUK_TYPE_OBJECT)
        stackInfo += inspectObject(ctx, idx);
    }
  }

  return stackInfo;
}


} // namespace duk::detail


#endif // DUK_DETAIL_DEBUG_H
