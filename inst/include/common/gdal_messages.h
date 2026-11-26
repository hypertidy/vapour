#ifndef VAPOUR_GDAL_MESSAGES_H
#define VAPOUR_GDAL_MESSAGES_H

// GDAL errors and warnings as R conditions.
//
// GDAL reports problems through an error handler, and the default handler
// writes to stderr. That text is invisible to R: it cannot be caught,
// suppressed or tested, and it is lost in front ends that do not show stderr.
//
// A MessageScope collects what GDAL reports while it is alive. Nothing is
// signalled from inside the handler, because the handler can run on a GDAL
// worker thread and because an R condition raised there would jump out through
// GDAL's own frames. The messages are turned into R warnings by gdal_call()
// once the GDAL work has returned, or folded into the error by stop_gdal().

#include <algorithm>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <cpp11.hpp>
#include "cpl_error.h"
#include "gdal_version.h"

namespace gdalcommon {

struct GDALMessage {
  CPLErr cls;
  int num;
  std::string text;
};

// What a scope collects lives on the heap and is handed to GDAL as the
// handler's user data. If R ever jumped out past a scope without unwinding it,
// the handler would still be installed: it then finds this state intact and
// not a dead stack frame, and the next scope puts the handlers right.
struct MessageState {
  std::mutex mutex;
  std::vector<GDALMessage> messages;
  size_t dropped = 0;
  std::thread::id owner;
  bool open = false;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  CPLErrorHandler prev_global = nullptr;
  void *prev_global_data = nullptr;
#endif
};

class MessageScope {
public:
  MessageScope() : state_(new MessageState) {
    // scopes do not nest, so one that is still live was jumped over: take its
    // handlers off. It is not freed, since nothing says its owner is gone.
    if (live() != nullptr) {
      shut(live());
      live() = nullptr;
    }
    state_->owner = std::this_thread::get_id();
    CPLErrorReset();
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
    // the handler that is current before ours goes on: the global one when
    // nothing is pushed on this thread, which is the usual case
    void *seen_data = nullptr;
    CPLErrorHandler seen = CPLGetErrorHandler(&seen_data);
#endif
    // this thread
    CPLPushErrorHandlerEx(&MessageScope::handler, state_);
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
    // GDAL worker threads have no handler of their own and use the global
    // one. It is swapped for the lifetime of the scope only, and put back,
    // because the global handler is shared with every package that uses the
    // same libgdal.
    state_->prev_global = CPLSetErrorHandlerEx(&MessageScope::handler, state_);
    state_->prev_global_data = (seen == state_->prev_global) ? seen_data : nullptr;
#endif
    state_->open = true;
    live() = state_;
  }

  ~MessageScope() {
    close();
    delete state_;
  }

  MessageScope(const MessageScope &) = delete;
  MessageScope &operator=(const MessageScope &) = delete;

  // stop collecting and put the previous handlers back
  void close() noexcept {
    shut(state_);
    if (live() == state_) live() = nullptr;
  }

  // hand over what has been collected so far
  std::vector<GDALMessage> take() { return take(state_); }

  // what the live scope has collected, empty when there is no scope
  static std::vector<GDALMessage> take_live() {
    if (live() == nullptr) return std::vector<GDALMessage>();
    return take(live());
  }
  static bool is_live() { return live() != nullptr; }

private:
  static const size_t max_messages = 50;

  static MessageState *&live() {
    static MessageState *state = nullptr;
    return state;
  }

  static void shut(MessageState *state) noexcept {
    if (state == nullptr || !state->open) return;
    state->open = false;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
    CPLSetErrorHandlerEx(state->prev_global, state->prev_global_data);
#endif
    CPLPopErrorHandler();
  }

  static std::vector<GDALMessage> take(MessageState *state) {
    std::lock_guard<std::mutex> lock(state->mutex);
    std::vector<GDALMessage> out;
    out.swap(state->messages);
    if (state->dropped > 0) {
      out.push_back(GDALMessage{CE_Warning, 0,
                    std::to_string(state->dropped) + " further GDAL messages were not kept"});
      state->dropped = 0;
    }
    return out;
  }

  static void CPL_STDCALL handler(CPLErr cls, CPLErrorNum num, const char *msg) {
    MessageState *state = static_cast<MessageState *>(CPLGetErrorHandlerUserData());
    if (state == nullptr || !state->open) {
      CPLDefaultErrorHandler(cls, num, msg);
      return;
    }
    if (msg == nullptr) msg = "";
    if (cls == CE_None) return;
    if (cls == CE_Debug) {
      // CPL_DEBUG output is wanted as it happens. The R console is only safe
      // from the R thread, other threads keep GDAL's own stderr.
      if (std::this_thread::get_id() == state->owner) {
        REprintf("%s\n", msg);
      } else {
        CPLDefaultErrorHandler(cls, num, msg);
      }
      return;
    }
    std::lock_guard<std::mutex> lock(state->mutex);
    for (const GDALMessage &m : state->messages) {
      if (m.cls == cls && m.num == static_cast<int>(num) && m.text == msg) return;
    }
    if (state->messages.size() >= max_messages) {
      state->dropped++;
      return;
    }
    state->messages.push_back(GDALMessage{cls, static_cast<int>(num), std::string(msg)});
  }

  MessageState *state_;
};

// Silence GDAL on this thread for the lifetime of the object, for a call
// whose failure is an expected answer and not something to report. The
// handler comes off again however the scope is left.
class QuietErrors {
public:
  QuietErrors() { CPLPushErrorHandler(CPLQuietErrorHandler); }
  ~QuietErrors() { CPLPopErrorHandler(); }
  QuietErrors(const QuietErrors &) = delete;
  QuietErrors &operator=(const QuietErrors &) = delete;
};

// R warnings for collected messages. A GDAL failure that did not stop the
// call is a warning too: the caller got a result, and should know.
inline void emit_warnings(const std::vector<GDALMessage> &messages) {
  for (const GDALMessage &m : messages) {
    if (m.cls == CE_Warning) {
      cpp11::warning("GDAL warning %d: %s", m.num, m.text.c_str());
    } else if (m.cls == CE_Failure || m.cls == CE_Fatal) {
      cpp11::warning("GDAL error %d: %s", m.num, m.text.c_str());
    }
  }
}

// Stop with an R error that carries GDAL's own account of what went wrong.
// Use this in place of cpp11::stop() when the failure came from a GDAL call.
[[noreturn]] inline void stop_gdal(const std::string &what) {
  std::string reason;
  std::vector<GDALMessage> warnings;
  if (MessageScope::is_live()) {
    for (const GDALMessage &m : MessageScope::take_live()) {
      if (m.cls == CE_Failure || m.cls == CE_Fatal) {
        if (!reason.empty()) reason += "; ";
        reason += m.text;
      } else {
        warnings.push_back(m);
      }
    }
  } else {
    const char *last = CPLGetLastErrorMsg();
    if (last != nullptr) reason = last;
  }
  // a trailing newline from GDAL would end the R message early
  while (!reason.empty() && (reason.back() == '\n' || reason.back() == ' ')) reason.pop_back();
  std::string msg = what;
  if (!reason.empty()) msg += ": " + reason;
  emit_warnings(warnings);
  cpp11::stop("%s", msg.c_str());
}

// Run GDAL work with its messages collected, then signal them as R warnings.
// Every entry point from R that reaches GDAL goes through this.
template <typename Fn>
inline auto gdal_call(Fn &&fn) -> decltype(fn()) {
  MessageScope scope;
  auto out = fn();
  std::vector<GDALMessage> messages = scope.take();
  scope.close();
  emit_warnings(messages);
  return out;
}

} // namespace gdalcommon
#endif
