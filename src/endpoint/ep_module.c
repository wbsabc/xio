/*
  Copyright (c) 2013-2014 Dong Fang. All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include <stdio.h>
#include <xio/socket.h>
#include <xio/poll.h>
#include "ep_struct.h"

struct xep_global epgb = {};

void xep_module_init() {
    int i;
    epgb.exiting = true;
    mutex_init(&epgb.lock);

    for (i = 0; i < XIO_MAX_ENDPOINTS; i++) {
	epgb.unused[i] = i;
    }
    epgb.nendpoints = 0;
}

void xep_module_exit() {
    epgb.exiting = true;
    mutex_destroy(&epgb.lock);
}

int eid_alloc() {
    int eid;
    mutex_lock(&epgb.lock);
    BUG_ON(epgb.nendpoints >= XIO_MAX_ENDPOINTS);
    eid = epgb.unused[epgb.nendpoints++];
    mutex_unlock(&epgb.lock);
    return eid;
}

void eid_free(int eid) {
    mutex_lock(&epgb.lock);
    epgb.unused[--epgb.nendpoints] = eid;
    mutex_unlock(&epgb.lock);
}

struct endpoint *eid_get(int eid) {
    return &epgb.endpoints[eid];
}

void accept_endsocks(int eid) {
    struct endpoint *ep = eid_get(eid);
    int tmp, s;
    struct endsock *es, *next_es;

    xendpoint_walk_sock(es, next_es, &ep->bsocks) {
	if (xselect(XPOLLIN|XPOLLERR, 1, &es->sockfd, 1, &tmp) == 0)
	    continue;
	BUG_ON(es->sockfd != tmp);
	if ((s = xaccept(es->sockfd)) < 0) {
	    if (errno != EAGAIN)
		list_move_tail(&es->link, &ep->bad_socks);
	    DEBUG_OFF("listener %d bad status", es->sockfd);
	    continue;
	}
	if (xep_add(eid, s) < 0) {
	    xclose(s);
	    continue;
	}
	DEBUG_OFF("accept new endsock %d", s);	
    }
}
