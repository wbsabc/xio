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

#include <xio/sp.h>
#include "sp_module.h"

struct epsk *sp_generic_join(struct epbase *ep, int fd) {
    int socktype;
    int optlen = sizeof(socktype);
    struct epsk *nsk = epsk_new();

    if (!nsk)
	return 0;
    BUG_ON(xgetopt(fd, XL_SOCKET, XSOCKTYPE, &socktype, &optlen));
    nsk->fd = fd;
    nsk->owner = ep;
    nsk->ent.fd = fd;
    nsk->ent.self = nsk;
    nsk->ent.care = (socktype == XLISTENER) ?
	XPOLLIN|XPOLLERR : XPOLLIN|XPOLLOUT|XPOLLERR;
    mutex_lock(&ep->lock);
    switch (socktype) {
    case XLISTENER:
	list_add_tail(&nsk->item, &ep->listeners);
	ep->listener_num++;
	break;
    case XCONNECTOR:
	list_add_tail(&nsk->item, &ep->connectors);
	ep->connector_num++;
	break;
    default:
	BUG_ON(1);
    }
    mutex_unlock(&ep->lock);
    sg_add_sk(nsk);
    return nsk;
}

int sp_add(int eid, int fd) {
    struct epbase *ep = eid_get(eid);
    int rc, on = 1;
    int optlen = sizeof(on);

    if (!ep) {
	errno = EBADF;
	eid_put(eid);
	return -1;
    }
    BUG_ON(xsetopt(fd, XL_SOCKET, XNOBLOCK, &on, optlen));
    rc = ep->vfptr.join(ep, 0, fd);
    eid_put(eid);
    return rc;
}
