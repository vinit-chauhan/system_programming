#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

void
print_byte(uint32_t byte) {
    const char* bit_rep[16] = {
        [0] = "0000",  [1] = "0001",  [2] = "0010",  [3] = "0011",
        [4] = "0100",  [5] = "0101",  [6] = "0110",  [7] = "0111",
        [8] = "1000",  [9] = "1001",  [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
    };
    printf("%s%s %s%s %s%s %s%s\n", bit_rep[byte >> 28 & 0x0F],
           bit_rep[byte >> 24 & 0x0F], bit_rep[byte >> 20 & 0x0F],
           bit_rep[byte >> 16 & 0x0F], bit_rep[byte >> 12 & 0x0F],
           bit_rep[byte >> 8 & 0x0F], bit_rep[byte >> 4 & 0x0F],
           bit_rep[byte & 0x0F]);
}

int
main() {

    u_int32_t addr = htonl(INADDR_LOOPBACK);
    int addr2 = inet_addr("127.0.0.1");

    print_byte(addr);
    print_byte(addr2);

    return 0;
}