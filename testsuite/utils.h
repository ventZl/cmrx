#pragma once

#ifdef OS_THREAD_CREATE
#undef OS_THREAD_CREATE
#endif

#define OS_THREAD_CREATE(application, entrypoint, data, priority) \
_OS_THREAD_CREATE(application, entrypoint, data, priority, 0)

