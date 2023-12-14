#ifndef DUKCPP_SCOPED_POP_H
#define DUKCPP_SCOPED_POP_H

#include <duktape.h>


namespace duk
{


class scoped_pop final
{
public:
  scoped_pop(duk_context* ctx, duk_idx_t count = 1) :
    ctx_(ctx),
    count_(count)
  {
  }

  ~scoped_pop()
  {
    duk_pop_n(ctx_, count_);
  }

  scoped_pop(const scoped_pop&) = delete;
  scoped_pop(scoped_pop&&) = delete;

  scoped_pop& operator=(const scoped_pop&) = delete;
  scoped_pop& operator=(scoped_pop&&) = delete;

private:
  duk_context* ctx_ = nullptr;
  duk_idx_t count_ = 1;
};


} // namespace duk


#endif // DUKCPP_SCOPED_POP_H
