#include <gtest/gtest.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <string>
extern "C" {
#include <sync/spin.h>
#include <runner/thread.h>
#include <xio/poll.h>
#include <xio/socket.h>
#include <xio/cmsghdr.h>
}

using namespace std;
extern int randstr(char *buf, int len);


TEST(xmsg, outofband) {
    int oob_count = -1;
    struct xcmsg ent = {};
    char *xbuf = xallocubuf(12);
    char *xbuf2;
    char oob1[12];
    char oob2[12];
    char oob3[12];


    BUG_ON(xmsgctl(xbuf, XMSG_CLONE, &xbuf2));
    BUG_ON(xmsgctl(xbuf2, XMSG_CMSGNUM, &oob_count));
    BUG_ON(oob_count != 0);
    xfreeubuf(xbuf2);
    
    randstr(xbuf, 12);
    randstr(oob1, sizeof(oob1));
    randstr(oob2, sizeof(oob2));
    randstr(oob3, sizeof(oob3));

    ent.idx = 0;
    ent.outofband = xallocubuf(sizeof(oob1));
    memcpy(ent.outofband, oob1, sizeof(oob1));
    BUG_ON(xmsgctl(xbuf, XMSG_SETCMSG, &ent) != 0);

    ent.idx = 2;
    ent.outofband = xallocubuf(sizeof(oob3));
    memcpy(ent.outofband, oob3, sizeof(oob3));
    BUG_ON(xmsgctl(xbuf, XMSG_SETCMSG, &ent) != 0);

    ent.idx = 1;
    ent.outofband = xallocubuf(sizeof(oob2));
    memcpy(ent.outofband, oob2, sizeof(oob2));
    BUG_ON(xmsgctl(xbuf, XMSG_SETCMSG, &ent) != 0);



    BUG_ON(xmsgctl(xbuf, XMSG_CLONE, &xbuf2));
    BUG_ON(xmsgctl(xbuf2, XMSG_CMSGNUM, &oob_count));
    BUG_ON(oob_count != 3);
    
    ent.idx = 2;
    BUG_ON(xmsgctl(xbuf, XMSG_GETCMSG, &ent) != 0);
    BUG_ON(memcmp(ent.outofband, oob3, sizeof(oob3)) != 0);
    BUG_ON(xmsgctl(xbuf2, XMSG_GETCMSG, &ent) != 0);
    BUG_ON(memcmp(ent.outofband, oob3, sizeof(oob3)) != 0);

    
    ent.idx = 1;
    BUG_ON(xmsgctl(xbuf, XMSG_GETCMSG, &ent) != 0);
    BUG_ON(memcmp(ent.outofband, oob2, sizeof(oob2)) != 0);
    BUG_ON(xmsgctl(xbuf2, XMSG_GETCMSG, &ent) != 0);
    BUG_ON(memcmp(ent.outofband, oob2, sizeof(oob2)) != 0);

    ent.idx = 0;
    BUG_ON(xmsgctl(xbuf, XMSG_GETCMSG, &ent) != 0);
    BUG_ON(memcmp(ent.outofband, oob1, sizeof(oob1)) != 0);
    BUG_ON(xmsgctl(xbuf2, XMSG_GETCMSG, &ent) != 0);
    BUG_ON(memcmp(ent.outofband, oob1, sizeof(oob1)) != 0);

    xfreeubuf(xbuf);
    xfreeubuf(xbuf2);
}

