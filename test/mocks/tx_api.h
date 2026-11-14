#ifndef TEST_MOCK_TX_API_H
#define TEST_MOCK_TX_API_H

#include <stdint.h>
#include <stddef.h>

typedef void VOID;
typedef char CHAR;
typedef unsigned char UCHAR;
typedef int INT;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long ULONG;
typedef short SHORT;
typedef unsigned short USHORT;

#define TX_SUCCESS              0x00U
#define TX_NO_WAIT              0x00000000UL
#define TX_WAIT_FOREVER         0xFFFFFFFFUL
#define TX_INHERIT              1U
#define TX_NO_TIME_SLICE        0U
#define TX_DONT_START           0U
#define TX_AUTO_START           1U
#define TX_NULL                 ((void *)0)
#define TX_TIMER_TICKS_PER_SECOND 100UL

typedef struct TX_THREAD_STRUCT {
    const char *name;
    UINT tx_thread_priority;
} TX_THREAD;

typedef struct TX_MUTEX_STRUCT {
    const char *name;
    int locked;
} TX_MUTEX;

typedef struct TX_SEMAPHORE_STRUCT {
    const char *name;
    ULONG count;
} TX_SEMAPHORE;

UINT tx_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr,
                      VOID (*entry_function)(ULONG), ULONG entry_input,
                      VOID *stack_start, ULONG stack_size,
                      UINT priority, UINT preempt_threshold,
                      ULONG time_slice, UINT auto_start);
UINT tx_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state,
                        ULONG *run_count, UINT *priority, UINT *preemption_threshold,
                        ULONG *time_slice, TX_THREAD **next_thread,
                        TX_THREAD **next_suspended_thread);
UINT tx_thread_reset(TX_THREAD *thread_ptr);
UINT tx_thread_resume(TX_THREAD *thread_ptr);
UINT tx_thread_suspend(TX_THREAD *thread_ptr);
UINT tx_thread_terminate(TX_THREAD *thread_ptr);
UINT tx_thread_sleep(ULONG timer_ticks);
ULONG tx_time_get(VOID);

UINT tx_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit);
UINT tx_mutex_get(TX_MUTEX *mutex_ptr, ULONG wait_option);
UINT tx_mutex_put(TX_MUTEX *mutex_ptr);

UINT tx_semaphore_create(TX_SEMAPHORE *semaphore_ptr, CHAR *name_ptr, ULONG initial_count);
UINT tx_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option);
UINT tx_semaphore_put(TX_SEMAPHORE *semaphore_ptr);

#endif
