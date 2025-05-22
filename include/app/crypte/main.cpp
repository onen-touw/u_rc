#include <iostream>
#include <stdint.h>
#include "encrypte.h"
#include "decrypte.h"

enum class tcmd_e : uint8_t {
    tcmd0,
    tcmd1,
    tcmd2,
    tcmd3,
};



// enum class cmd_t : uint8_t {
// 	none,
// 	request_ask,
// 	request_err,

// 	ack = 5,
	
// 	remote_trpy = 10,
// 	// remote_arm,
	
// };


int8_t a1 = 65;
int16_t a2 = 128*128;

int main(){
    crt::encrypte_t<tcmd_e> encr;
    encr.pack(tcmd_e::tcmd0, a1, a2);
    encr.log();

    uint8_t* bf = encr.get();
    uint16_t bf_sz = encr.size();

    crt::decrypte_t<tcmd_e>::unpack(
        bf,
        bf_sz,
        [](tcmd_e cmd, uint8_t* buf){
            if (cmd == tcmd_e::tcmd0)
            {
                auto aa1 = crt::gget_arg<crt::arg_1, int8_t>(buf);
                auto aa2 = crt::gget_arg<crt::arg_2, int16_t>(buf);
                printf("cmd0\n");
                // printf("args:\n\t %u, %u\n", 
                //     aa1, aa2);
                // printf("base:args:\n\t %u, %u\n", 
                //     a1, a2);
                // printf("\t%u, %u\n", aa1 == a1, aa2 == a2);
                printf("args:\n\t %i, %i\n", 
                    aa1, aa2);
                printf("base:args:\n\t %i, %i\n", 
                    a1, a2);
                printf("\t%i, %i\n", aa1 == a1, aa2 == a2);
                
            }
            else if (cmd == tcmd_e::tcmd1)
            {
                printf("cmd1\n");
            }
        });

    printf("sizes:\n\ti8: %lu\n\ti16: %lu\n\ti32: %lu\n\tf: %lu\n", sizeof(uint8_t), sizeof(uint16_t),sizeof(uint32_t),sizeof(float));
        
    int16_t gi = a2;
    uint8_t i1 = a2 & 255;
    uint8_t i2 = (a2 >> 8) & 255;
    int16_t ig = i1 << 8 | i2;
    printf("%i\n", ig);

    return 0;
}
