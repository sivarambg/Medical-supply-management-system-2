#ifndef MONITOR_H
#define MONITOR_H

/* How often (in seconds) the background thread re-checks for expired
   and low-stock medicines while the app is running. */
#define MONITOR_INTERVAL_SEC 30

/* Starts the background alert-monitor thread and returns immediately
   (the thread is detached - nothing needs to join it later, it just
   runs for the life of the program). Call once, after login, before
   handing control to the main menu loop. */
void startMonitorThread(void);

#endif /* MONITOR_H */
