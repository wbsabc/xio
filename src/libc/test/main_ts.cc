#include <gtest/gtest.h>
#include <sys/signal.h>
extern "C" {
#include <base.h>
}

int gargc = 0;
char **gargv = NULL;

int randstr(char *buf, int len) {
    int i, idx;
    char token[] = "qwertyuioplkjhgfdsazxcvbnm1234567890";
    for (i = 0; i < len; i++) {
	idx = rand() % strlen(token);
	buf[i] = token[idx];
    }
    return 0;
}

int runall_gtest() {
    int rc;
    testing::InitGoogleTest(&gargc, gargv);
    rc = RUN_ALL_TESTS();
    return rc;
}

int main(int argc, char **argv) {

    int rc;
    
    gargc = argc;
    gargv = argv;

    base_init();
    if (SIG_ERR == signal(SIGPIPE, SIG_IGN)) {
        fprintf(stderr, "signal SIG_IGN");
        return -1;
    }
    rc = runall_gtest();
    base_exit();
    return rc;
}
