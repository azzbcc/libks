/*
 * Copyright (c) 2018-2023 SignalWire, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "libks/ks.h"
#include <tap.h>

static int test_ws(char *url) {
    kws_t *kws = NULL;
    ks_pool_t *pool;
    kws_opcode_t oc;
    uint8_t *rdata;

    ks_json_t *req = ks_json_create_object();
    ks_json_add_string_to_object(req, "url", url);

    ks_global_set_log_level(7);

    ks_pool_open(&pool);
    ks_assert(pool);

    ok(kws_connect_ex(&kws, req, KWS_BLOCK | KWS_CLOSE_SOCK, pool, NULL, 3000) == KS_STATUS_SUCCESS);
    printf("websocket connected to [%s]\n", url);
    ks_json_delete(&req);

    ks_ssize_t bytes;
    int32_t poll_flags = 0;

    while (1) {
        poll_flags = kws_wait_sock(kws, 50, KS_POLL_READ | KS_POLL_ERROR);
        if (poll_flags == KS_POLL_ERROR) break;
        if (poll_flags == KS_POLL_READ) {

            bytes = kws_read_frame(kws, &oc, &rdata);
            if (oc == WSOC_CLOSE) {
                break;
            } else if (oc == WSOC_TEXT) {
                printf("read text bytes=%d oc=%d [%s]\n", (int) bytes, oc, (char *) rdata);
            } else if (oc == WSOC_BINARY) {
                printf("read binarary bytes=%d oc=%d\n", (int) bytes, oc);
            } else if (oc == WSOC_PING) {
                printf("read ping\n");
            } else if (oc == WSOC_PONG) {
                printf("read pong\n");
            } else if (oc == WSOC_CONTINUATION) {
                printf("read continuation\n");
            } else {
                printf("read invalid\n");
            }

        }
    }

    kws_destroy(&kws);

    ks_pool_close(&pool);

    return 1;
}

int main(int argc, char *argv[]) {
    ks_init();
    ks_log_jsonify();

    plan(1);

    test_ws(argv[1]);

    ks_shutdown();

    done_testing();
}
