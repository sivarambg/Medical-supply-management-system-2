#if !defined(_WIN32) && !defined(_MSC_VER)
/* Must be defined before any system header is pulled in (including
   indirectly through common.h's <stdio.h>) so that fileno() and
   flock() are visible under strict -std=c99. */
#define _POSIX_C_SOURCE 200809L
#endif

#include "common.h"
#include "utils.h"

#if !defined(_WIN32) && !defined(_MSC_VER)
#include <sys/file.h>   /* flock() - POSIX file locking */
#define MEDISTORE_HAVE_FLOCK 1
#endif

const char *logFileName = "medistore_log.txt";

/* ============================================================
   MULTI-INSTANCE FILE LOCKING (see utils.h for the full picture)
   ============================================================ */

FILE *openLocked(const char *filename, const char *mode) {
    FILE *fp;
    fp = fopen(filename, mode);
    if (fp == NULL) {
        return NULL;
    }
#ifdef MEDISTORE_HAVE_FLOCK
    {
        /* "r" only needs a shared lock (many readers can hold this
           together); "w"/"a" need an exclusive lock (only one writer
           at a time, and it also blocks any reader mid-write so
           nobody reads a half-written file). flock() blocks here
           until whichever lock is needed becomes free. */
        int lockType;
        lockType = (mode[0] == 'r') ? LOCK_SH : LOCK_EX;
        flock(fileno(fp), lockType);
    }
#endif
    return fp;
}

void closeLocked(FILE *fp) {
    if (fp != NULL) {
#ifdef MEDISTORE_HAVE_FLOCK
        flock(fileno(fp), LOCK_UN);
#endif
        fclose(fp);
    }
}

/* Appends one line of text to the log file. Uses "a" (append) mode
   so every call adds a new line without erasing old data. */
void writeLog(const char *text) {
    FILE *fp;
    fp = fopen(logFileName, "a");
    if (fp != NULL) {
        fprintf(fp, "%s\n", text);
        fclose(fp);
    }
}

/* Gets today's date using the standard library (time.h). */
void getCurrentDate(int *day, int *month, int *year) {
    time_t t;
    const struct tm *now;
    t = time(NULL);
    now = localtime(&t);
    *day = now->tm_mday;
    *month = now->tm_mon + 1;
    *year = now->tm_year + 1900;
}

int readInt(void) {
    int value;
    int result;
    int ch;

    result = scanf("%d", &value);
    if (result != 1) {
        value = 0;   /* EOF or non-numeric input: fall back to a safe default */
    }

    /* discard the rest of the line so leftover text is never re-read
       by the next input call - this is what stops a bad entry (or a
       closed input stream) from causing an infinite loop */
    ch = getchar();
    while (ch != '\n' && ch != EOF) {
        ch = getchar();
    }

    return value;
}

int readWord(char *buf, int maxlen) {
    char fmt[16];
    int result;

    if (maxlen <= 1) {
        buf[0] = '\0';
        return 0;
    }

    /* build a width-limited "%Ns" format at runtime so the read can
       never overflow buf, no matter what maxlen is passed in */
    sprintf(fmt, "%%%ds", maxlen - 1);
    result = scanf(fmt, buf);

    if (result != 1) {
        strcpy(buf, "0");   /* EOF/failure: behave like the user typed the "stop" sentinel */
        return 0;
    }
    return 1;
}
