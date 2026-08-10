#define _POSIX_C_SOURCE 200809L   /* needed for sleep() under -std=c99 */

#include "common.h"
#include "monitor.h"
#include "medicine.h"
#include <pthread.h>
#include <unistd.h>

/* The actual thread body. Runs forever in a loop: sleep, then
   re-check expiry and low stock, same logic the menu's own "Low
   Stock Alerts" / "Expiry" screens use (checkExpiry() and
   lowStockAlert() in medicine.c) - so there's exactly one place the
   expiry/low-stock rules live, whether triggered by a person or by
   this timer. Both of those functions take medicineLock internally,
   so nothing extra is needed here to stay safe against the main
   thread adding/editing/deleting medicines at the same time. */
static void *monitorLoop(void *arg) {
    (void)arg;   /* unused - required signature for pthread_create */

    while (1) {
        sleep(MONITOR_INTERVAL_SEC);

        printf("\n[background monitor] periodic stock/expiry check:\n");
        checkExpiry();
        lowStockAlert();
        printf("[background monitor] check complete. (This may appear "
               "mid-prompt if you're typing - that's just console output "
               "interleaving between the two threads, not a bug.)\n");
    }

    return NULL;   /* unreachable - kept so the function has a clean exit path */
}

void startMonitorThread(void) {
    pthread_t tid;
    int result;

    result = pthread_create(&tid, NULL, monitorLoop, NULL);
    if (result != 0) {
        printf("Warning: could not start the background monitor thread.\n");
        return;
    }

    /* Detach: we never need to pthread_join() this thread, it just
       runs for the program's whole lifetime and the OS cleans it up
       when the process exits. */
    pthread_detach(tid);
}
