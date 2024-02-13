#include "monetra_api.h"
#include "mstdlib/mstdlib_tls.h"

static M_thread_once_t LM_once = M_THREAD_ONCE_STATIC_INITIALIZER;

static void LM_init_destructor(void *arg)
{
    (void)arg;
    M_thread_once_reset(&LM_once);
}

static void LM_init_int(M_uint64 flags)
{
    M_tls_init((flags & LM_INIT_SSLLOCK_EXTERNAL)?M_TLS_INIT_EXTERNAL:M_TLS_INIT_NORMAL);
    M_library_cleanup_register(LM_init_destructor, NULL);
}

M_bool LM_SPEC LM_init(M_uint64 flags)
{
    if (M_thread_once(&LM_once, LM_init_int, flags))
        return M_TRUE;
    return M_FALSE;
}

