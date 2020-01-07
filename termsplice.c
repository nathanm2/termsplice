
#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include <glib.h>
#include <glib-unix.h>

typedef struct {
    int pty_count;
    int *pty_fds;  /* Array of master PTY handles. */
} Context;

typedef struct {
    Context *ctx;
    int index;
    char* pty_name;
} PtyContext;

gboolean slave_read(int fd, GIOCondition condition, void* _ctx)
{
    PtyContext *pctx = (PtyContext *)_ctx;
    char buf;
    int rc = read(fd, &buf, 1);

    if (rc < 0) {
        if (errno == EIO)
            printf("[%d] Slave closed.\n", pctx->index);
        else
            printf("[%d] errno=%d %s\n", pctx->index, errno, strerror(errno));
        return FALSE;
    }

    printf("[%d] 0x%x\n", pctx->index, buf);
    return TRUE;
}

void pty_context_free(void* _ctx) {
    PtyContext *pctx = (PtyContext *)_ctx;
    if(pctx) {
        g_free(pctx->pty_name);
        g_free(pctx);
    }
}

bool raw_mode(int fd) {
    return true;
}

bool register_pty(int index, bool keep_alive, Context* ctx)
{
    char *pts_name = NULL;
    int ptm, pts;
    PtyContext* pctx = g_new0(PtyContext, 1);
    pctx->ctx = ctx;
    pctx->index = index;

    /* Create the pty master: */
    ptm = posix_openpt(O_RDWR | O_NOCTTY );
    grantpt(ptm);
    unlockpt(ptm);

    pctx->pty_name = g_strdup(ptsname(ptm));
    printf("PTY Slave[%i]: %s\n", index, pctx->pty_name);

    /* EIO is returned when the last slave handle has been closed.  To prevent
     * this keep a slave handle open. */
    if (keep_alive) {
        open(pctx->pty_name, O_RDWR);
    }

    /* Change to raw mode */
    raw_mode(ptm);

    g_unix_fd_add_full(G_PRIORITY_DEFAULT, ptm, G_IO_IN, slave_read, pctx,
                       pty_context_free);
    ctx->pty_fds[index] = ptm;
    return true;
}

int main(int argc, char* argv[])
{
    int opt;
    int i, count;
    bool keep_alive = true;
    Context ctx;

    /* Option parsing: */
    while ((opt = getopt(argc, argv, "n:vk")) != -1) {
        switch(opt) {
        case 'n':
            count = atoi(optarg);
            break;
        case 'k':
            keep_alive = false;
            break;
        default:
            fprintf(stderr, "Usage: %s [-n COUNT] [-v] [-k] [/dev/ttyXXX]\n",
                    argv[0]);
            exit(-1);
        }
    }


    if (count == 0) {
        fprintf(stderr, "COUNT must be greater than zero.\n");
        exit(-1);
    }

    ctx.pty_fds = g_new(int, count);
    ctx.pty_count = count;

    for(i = 0; i < count; i++) {
        register_pty(i, keep_alive, &ctx);
    }

    GMainLoop *loop = g_main_loop_new(NULL, false);
    g_main_loop_run(loop);

    return 0;
}
