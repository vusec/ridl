#pragma once

#if defined(_WIN32)
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <process.h>

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif
