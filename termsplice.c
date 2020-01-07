
#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include <glib.h>
#include <glib-unix.h>

int verbosity = 0;

typedef struct {
    int index;
    char* slave;
} PtyContext;

gboolean slave_read(int fd, GIOCondition condition, void* _ctx)
{
    PtyContext *ctx = (PtyContext *)_ctx;
    char buf;
    int rc;

    rc = read(fd, &buf, 1);
    if (rc <= 0) {
        printf("PTY[%d]: Error encountered: %d\n", ctx->index, rc);
        return FALSE;
    } else {
        printf("[%d] 0x%x\n", ctx->index, buf);
    }
    return TRUE;
}

void pty_context_free(void* _ctx) {
    PtyContext *ctx = (PtyContext *)_ctx;
    if(ctx) {
        g_free(ctx->slave);
        g_free(ctx);
    }
}

bool register_pty(int index)
{
    char *pts_name = NULL;
    int ptm, pts;
    PtyContext* ctx = g_new0(PtyContext, 1);
    ctx->index = index;

    /* Create the pty master: */
    ptm = posix_openpt(O_RDWR | O_NOCTTY);
    grantpt(ptm);
    unlockpt(ptm);

    ctx->slave = g_strdup(ptsname(ptm));
    printf("PTY Slave[%i]: %s\n", index, ctx->slave);
    open(ctx->slave, O_RDWR);

    g_unix_fd_add_full(G_PRIORITY_DEFAULT, ptm, G_IO_IN, slave_read, ctx,
                       pty_context_free);
    return true;
}

int main(int argc, char* argv[])
{
    int opt;
    int i, count = 2;

    /* Option parsing: */
    while ((opt = getopt(argc, argv, "n:v")) != -1) {
        switch(opt) {
        case 'n':
            count = atoi(optarg);
            break;
        case 'v':
            verbosity++;
            break;
        default:
            fprintf(stderr, "Usage: %s [-n COUNT] [-v] [/dev/ttyXXX]\n",
                    argv[0]);
            exit(-1);
        }
    }


    if (count == 0) {
        fprintf(stderr, "COUNT must be greater than zero.\n");
        exit(-1);
    }

    for(i = 0; i < count; i++) {
        register_pty(i);
    }

    GMainLoop *loop = g_main_loop_new(NULL, false);
    g_main_loop_run(loop);

    return 0;
}
