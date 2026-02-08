#include <stdio.h>
#include <string.h>
#include "md5.h"

int errors = 0;

void print_digest(unsigned char *digest) {
    int i;
    for(i = 0; i < 16; i++) {
        printf("%02x", digest[i]);
    }
}

void verify_md5_bytes(const unsigned char *bytes, uint32_t len, const char *label, const char *expected) {
    MD5_CTX context;
    unsigned char digest[16];
    char output[33];
    int i;

    MD5Init(&context);
    MD5Update(&context, bytes, len);
    MD5Final(digest, &context);

    // Format digest into output string
    for(i = 0; i < 16; i++) {
        sprintf(output + (i * 2), "%02x", digest[i]);
    }
    output[32] = 0;

    printf("MD5(%s) = ", label);
    print_digest(digest);

    if (strcmp(output, expected) == 0) {
        printf(" [PASS]\n");
    } else {
        printf(" [FAIL]\n");
        printf("Expected: %s\n", expected);
        errors++;
    }
}

int main() {
    unsigned char a_byte[] = { 0x61 }; // 'a' in ASCII
    unsigned char abc_bytes[] = { 0x61, 0x62, 0x63 }; // "abc" in ASCII
    unsigned char msg_bytes[] = { 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 
                                  0x64, 0x69, 0x67, 0x65, 0x73, 0x74 }; // "message digest" in ASCII
    printf("MD5 Test Suite\n");
    printf("--------------\n");

#ifdef MD5_DEBUG
    MD5_Internal_Tests();
#endif

    verify_md5_bytes((unsigned char*)"", 0, "\"\"", "d41d8cd98f00b204e9800998ecf8427e");
    verify_md5_bytes(a_byte, 1, "\"a\"", "0cc175b9c0f1b6a831c399e269772661");
    verify_md5_bytes(abc_bytes, 3, "\"abc\"", "900150983cd24fb0d6963f7d28e17f72");
    verify_md5_bytes(msg_bytes, 14, "\"message digest\"", "f96b697d7cb7938d525a2f31aaf161d0");

    if (errors == 0) {
        printf("\nAll tests passed!\n");
    } else {
        printf("\n%d tests failed.\n", errors);
    }

    return 0;
}
