/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/logging.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xam/xam_private.h"
#include "xenia/kernel/xnotifylistener.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
namespace xam {

dword_result_t XamNotifyCreateListenerInternal(qword_t mask) {
  auto listener =
      object_ref<XNotifyListener>(new XNotifyListener(kernel_state()));
  listener->Initialize(mask);

  // Handle ref is incremented, so return that.
  uint32_t handle = listener->handle();

  return handle;
}
DECLARE_XAM_EXPORT1(XamNotifyCreateListenerInternal, kNone, kImplemented);

dword_result_t XamNotifyCreateListener(qword_t mask) {
  return XamNotifyCreateListenerInternal(mask);
}
DECLARE_XAM_EXPORT1(XamNotifyCreateListener, kNone, kImplemented);

// https://github.com/CodeAsm/ffplay360/blob/master/Common/AtgSignIn.cpp
dword_result_t XNotifyGetNext(dword_t handle, dword_t match_id,
                              lpdword_t id_ptr, lpdword_t param_ptr) {
  if (param_ptr) {
    *param_ptr = 0;
  }

  if (!id_ptr) {
    return X_ERROR_INVALID_PARAMETER;
  }
  *id_ptr = 0;
  // Grab listener.
  auto listener =
      kernel_state()->object_table()->LookupObject<XNotifyListener>(handle);
  if (!listener) {
    return X_ERROR_INVALID_HANDLE;
  }

  bool dequeued = false;
  uint32_t id = 0;
  uint32_t param = 0;
  if (match_id) {
    // Asking for a specific notification
    id = match_id;
    dequeued = listener->DequeueNotification(match_id, &param);
  } else {
    // Just get next.
    dequeued = listener->DequeueNotification(&id, &param);
  }

  *id_ptr = dequeued ? id : 0;
  // param_ptr may be null - Ghost Recon Advanced Warfighter 2 Demo explicitly
  // passes nullptr in the code.
  // https://github.com/xenia-project/xenia/pull/1577
  if (param_ptr) {
    *param_ptr = dequeued ? param : 0;
  }
  return dequeued ? 1 : 0;
}
DECLARE_XAM_EXPORT1(XNotifyGetNext, kNone, kImplemented);

dword_result_t XNotifyDelayUI(dword_t delay_ms) {
  // Ignored.
  return 0;
}
DECLARE_XAM_EXPORT1(XNotifyDelayUI, kNone, kStub);

void XNotifyPositionUI(dword_t position) {
  // Ignored.
}
DECLARE_XAM_EXPORT1(XNotifyPositionUI, kNone, kStub);

void RegisterNotifyExports(xe::cpu::ExportResolver* export_resolver,
                           KernelState* kernel_state) {}

}  // namespace xam
}  // namespace kernel
}  // namespace xe