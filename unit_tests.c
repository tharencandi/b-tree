#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>

#include <stdio.h>


#include "cmocka.h"
#include "btreestore.h"
static void bt_decrypt_test_basic();
static void encrypt_decrypt_tea_ctr_test_basic();
static void encrypt_decrypt_tea_ctr_test_many_blocks();
static void bt_decrypt_test_large();


#define STR_SIZE 15
#define STR_LARGE 1000000
static void bt_decrypt_test_basic() {

    void * helper = init_store(4, 4);
    
    char plaintext[STR_SIZE] = "hello bro\n";
    char output[STR_SIZE] = {0};
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    btree_insert(10, plaintext, sizeof(plaintext), encrption_key, nonce, helper);

    btree_decrypt(10, output, helper);
    close_store(helper);
    assert_string_equal(plaintext, output);    

}

static void bt_decrypt_test_large() {

    void * helper = init_store(4, 4);
    
    char plaintext[STR_LARGE];
    char output[STR_LARGE];

    for (int i = 0; i < STR_LARGE; i ++) {
        plaintext[i] = 'a';
        output[i] = 0;
        if ( i == STR_LARGE -1) {
            plaintext[i] = '\0';
        }
    }

    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    btree_insert(10, plaintext, STR_LARGE, encrption_key, nonce, helper);

    btree_decrypt(10, output, helper);
    close_store(helper);
    assert_string_equal(plaintext, output);

}

static void encrypt_decrypt_tea_ctr_test_basic() {

    uint64_t cipher[10];
    uint64_t plain[10];
    uint64_t original[10];

    uint32_t key[4] = {4,7,3,102};
    uint64_t nonce = 2435;
    uint32_t num_blocks = 10;

    for (int i = 0; i < num_blocks; i ++) {
        original[i] = i;
        plain[i] = 0;
        cipher[i] = 0;

    }

   
    encrypt_tea_ctr(original, key, nonce, cipher, num_blocks);
    decrypt_tea_ctr(cipher, key, nonce, plain, num_blocks);
  
    
   
    assert_memory_equal(original, plain, num_blocks);
    
}

#define MANY_BLOCKS (65536)
static void encrypt_decrypt_tea_ctr_test_many_blocks() {
    

    uint64_t cipher[MANY_BLOCKS];
    uint64_t plain[MANY_BLOCKS];
    uint64_t original[MANY_BLOCKS];

    uint32_t key[4] = {4,7,3,102};
    uint64_t nonce = 2435;
    uint32_t num_blocks = MANY_BLOCKS;

    for (int i = 0; i < num_blocks; i ++) {
        original[i] = i;
        plain[i] = 0;
        cipher[i] = 0;

    }

    encrypt_tea_ctr(original, key, nonce, cipher, num_blocks);
    decrypt_tea_ctr(cipher, key, nonce, plain, num_blocks);
 
    
   
    assert_memory_equal(original, plain, num_blocks);

}





int main(void) {

    const struct CMUnitTest tests[] = {

        cmocka_unit_test(encrypt_decrypt_tea_ctr_test_many_blocks),
        cmocka_unit_test(bt_decrypt_test_basic),
        cmocka_unit_test(encrypt_decrypt_tea_ctr_test_basic),
        cmocka_unit_test(bt_decrypt_test_large),


    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}


