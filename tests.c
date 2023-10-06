#include "btreestore.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <string.h>
void print_tree(void *, char *, char *);

int insert_case_basic_1(char *);
int insert_case_basic_2(char *);

int insert_large_tree(char *);
int insert_complex_ord_5(char *);
int insert_many_keys_ord_6(char *);


int test_delete_all_root(char *);
int test_delete_medium(char *);
int delete_left_merge(char *);
int delete_basic_1(char *);
int delete_basic_2(char *);
int delete_many_keys_ord_6(char *);
int multithread_delete_2(char *);
int multithread_delete_1(char *);


pthread_t my_thread1;
pthread_t my_thread2;
pthread_t my_thread3;


int main(int argc, char ** argv) {

    char *test_directory = ".";
    if (argc >= 2) {
        test_directory = argv[1];
    }



  


   
    insert_case_basic_1(test_directory);
    
    
    insert_large_tree(test_directory);
    insert_complex_ord_5(test_directory);
    insert_many_keys_ord_6(test_directory);
   
    
    test_delete_all_root(test_directory);
    delete_basic_1(test_directory);
    delete_basic_2(test_directory);
    delete_left_merge(test_directory);
    delete_many_keys_ord_6(test_directory);
    
    multithread_delete_1(test_directory);
    test_delete_medium(test_directory);
    multithread_delete_2(test_directory);

    
    
    return 0;

}


int insert_case_basic_1(char * test_directory) {

    void * helper = init_store(3, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;
    btree_insert((uint32_t) 2, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 3, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 1, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    
    print_tree(helper, "insert_case_basic_1.out", test_directory);
  
    close_store(helper);
    return 0;


}
int insert_case_basic_2(char * test_directory) {
    void * helper = init_store(3, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;
     
    btree_insert((uint32_t) 7, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 24, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 13, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 19, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 30, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 9, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 11, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 17, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 20, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 21, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 22, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    
    print_tree(helper, "insert_case_basic_1.out", test_directory);
 

    close_store(helper);
    return 0;
}

int delete_left_merge(char * test_directory) {
    void * helper = init_store(4, 4);
    

    /*
    INSERT CALLED : 2
    INSERT CALLED : 3
    INSERT CALLED : 1
    INSERT CALLED : 8
    INSERT CALLED : 80
    INSERT CALLED : 5
    INSERT CALLED : 6
    */

    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;
    btree_insert((uint32_t) 2, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 3, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 1, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 8, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 80, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 5, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 6, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);


    print_tree(helper, "delete_left_merge_pre.out", test_directory);
    
    btree_delete(80, helper);
    print_tree(helper, "delete_left_merge_post.out", test_directory);
    

    close_store(helper);
    return 0;
}
int delete_basic_1(char * test_directory) {

    /*
    INSERT: 2
    INSERT: 3
    INSERT: 1
    INSERT: 8
    INSERT: 80
    INSERT: 5
    INSERT: 6
    INSERT: 4
    INSERT: 20
    INSERT: 21
    INSERT: 22
    DELETE: 6
    DELETE: 2
    DELETE: 5
    */
    void * helper = init_store(4, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    btree_insert((uint32_t) 2, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 3, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 1, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 8, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 80, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 5, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 6, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 4, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 20, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 21, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 22, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);

    print_tree(helper, "delete_basic_1_pre.out", test_directory);
   


    btree_delete(6,helper);
    btree_delete(2,helper);
    btree_delete(5,helper);

    print_tree(helper, "delete_basic_1_post.out", test_directory);


    close_store(helper);
    return 0;

}

int delete_basic_2(char * test_directory) {
    /*
    b: 4
    INSERT CALLED : 3
    INSERT CALLED : 4
    INSERT CALLED : 20
    INSERT CALLED : 22
    INSERT CALLED : 21
    INSERT CALLED : 80
    INSERT CALLED : 1
    DELETE CALLED : 20
    DELETE CALLED : 4
    DELETE CALLED : 21
    DELETE CALLED : 22
    */

    void * helper = init_store(4, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    btree_insert((uint32_t) 3, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 4, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 20, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 22, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 21, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 80, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    btree_insert((uint32_t) 1, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
   
  
    print_tree(helper, "delete_basic_2_pre.out", test_directory);
 
    btree_delete(20,helper);
    btree_delete(4,helper);
    btree_delete(21,helper);
    btree_delete(22,helper);
    
    print_tree(helper, "delete_basic_2_post.out", test_directory);

    close_store(helper);
    return 0;

}

int insert_large_tree(char * test_directory) {
    void * helper = init_store(4, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;
    for (int i = 0; i < 12; i++) {
        btree_insert((uint32_t) i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
        btree_insert((uint32_t) 25 + i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }


    print_tree(helper, "insert_large_tree.out", test_directory);

    close_store(helper);
    return 0;
}

int test_delete_all_root( char * test_directory) {

    void * helper = init_store(255, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;
    for (int i = 1; i < 12; i++) {
        btree_insert(i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
        
    }

    print_tree(helper, "test_delete_all_root_pre.out", test_directory);

    for (int i = 1; i < 12; i++ ) {
        btree_delete(i,helper);  
    }
    print_tree(helper, "test_delete_all_root_post.out", test_directory);
 
    close_store(helper);
    return 0;
}


int test_delete_medium(char * test_directory) {
  
    void * helper = init_store(5, 4);
    
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    for (int i = 1; i <= 50; i ++) {
        btree_insert(i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }
    print_tree(helper, "test_delete_medium_pre.out", test_directory);

    for (int i = 50; i > 0; i --) {
       btree_delete(i, helper);
        
    }
    
    print_tree(helper, "test_delete_medium_post.out", test_directory);

    close_store(helper);

}

int insert_complex_ord_5(char * test_directory) {

    /*
    ORDER OF INSERT:
        21,48,32,56,7,5,6,3,20,50,22,24,25,26,27,
        1,2,4,57,58,59,60,61,62,63,64,65,66,67,68,
        69,70,71,72,73,74,75,76,77,78,79,80,81,82,
        8,9,10,11,12,13,14,15,16
    */

    void * helper = init_store(5, 4);
    char * plaintext = "hello bro\n";
    void * ptr = plaintext;
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    size_t size = sizeof(plaintext);
    btree_insert(21, ptr, size , encrption_key, nonce, helper);
    btree_insert(48, ptr, size , encrption_key, nonce, helper);
    btree_insert(32, ptr, size , encrption_key, nonce, helper);
    btree_insert(56, ptr, size , encrption_key, nonce, helper);
    btree_insert(7, ptr, size , encrption_key, nonce, helper);
    btree_insert(5, ptr, size , encrption_key, nonce, helper);
    btree_insert(6, ptr, size , encrption_key, nonce, helper);
    btree_insert(3, ptr, size , encrption_key, nonce, helper);
    btree_insert(20, ptr, size , encrption_key, nonce, helper);
    btree_insert(50, ptr, size , encrption_key, nonce, helper);
    btree_insert(22, ptr, size , encrption_key, nonce, helper);
    btree_insert(24, ptr, size , encrption_key, nonce, helper);
    btree_insert(25, ptr, size , encrption_key, nonce, helper);
    btree_insert(26, ptr, size , encrption_key, nonce, helper);
    btree_insert(27, ptr, size , encrption_key, nonce, helper);
    btree_insert(1, ptr, size , encrption_key, nonce, helper);
    btree_insert(2, ptr, size , encrption_key, nonce, helper);
    btree_insert(4, ptr, size , encrption_key, nonce, helper);

    for (int i = 57; i <= 82; i ++) {
        btree_insert(i, ptr, size , encrption_key, nonce, helper);
    }

    for ( int i = 8; i <=16; i++) {
        btree_insert(i, ptr, size , encrption_key, nonce, helper);
    }

    print_tree(helper, "insert_complex_ord_5.out", test_directory);

    close_store(helper);
    return 0;
}


int insert_many_keys_ord_6(char * test_directory) {
   
    void * helper = init_store(6, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    for (int i = 1; i < 100; i++) {
        btree_insert((uint32_t) i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
        
        btree_insert((uint32_t) 25 + i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }

    print_tree(helper, "insert_many_keys_ord_6.out", test_directory);
  
    close_store(helper);
    return 0;
}

int delete_many_keys_ord_6(char * test_directory) {

    void * helper = init_store(6, 4);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    for (int i = 1; i < 100; i++) {
        btree_insert((uint32_t) i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
        btree_insert((uint32_t) 25 + i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }

    for (int i = 1; i < 90; i++) {
        btree_delete(i,helper);
    }

    print_tree(helper, "delete_many_keys_ord_6.out", test_directory);

    close_store(helper);
    return 0;
}


void* insert_thread_1(void * helper) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

     for (int i = 1; i <= 10; i ++) {
         btree_insert(i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }
   
    pthread_mutex_unlock(&lock);
}
void * insert_thread_2(void * helper) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    for (int i = 11; i <= 20; i ++) {
         btree_insert(i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }
  
    
 
    pthread_mutex_unlock(&lock);

}


void * insert_thread_3(void * helper) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    for (int i = 0; i <= 50; i ++) {
         btree_insert(i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
         btree_insert(50 + i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }       
  
  
    pthread_mutex_unlock(&lock);

}
void* delete_thread_1(void * helper) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);

    
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    for (int i = 1; i <= 20; i ++) {

        btree_insert(i, (void*) plaintext, sizeof(plaintext), encrption_key, nonce, helper);
    }

    for (int i = 1; i <=20; i ++) {
        btree_delete(i, helper);
        
    }

    pthread_mutex_unlock(&lock);
}

void* delete_thread_2(void * helper) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);

    
    char * plaintext = "hello bro\n";
    uint32_t encrption_key[4] = {1,2,3,4};  
    uint64_t nonce = 10;

    for (int i = 50; i >=50; i --) {
        btree_delete(i, helper);
        
    }

    pthread_mutex_unlock(&lock);
}


int multithread_delete_1(char * test_directory) {

    void * helper = init_store(4, 4);

    pthread_create(&my_thread1,NULL,insert_thread_1,helper);
    pthread_create(&my_thread2,NULL,delete_thread_1,helper);
    pthread_create(&my_thread3,NULL,insert_thread_2,helper);
    pthread_join(my_thread1, NULL);
    pthread_join(my_thread2, NULL);
    pthread_join(my_thread3, NULL);
    
    char file[200];
    sprintf(file, "%s/%s", test_directory, "multithread_delete_1.out");
    printf("writing to: %s\n", file);
    
    FILE * out_file = fopen(file, "w");

    char output[] = "no errors\n";
    fwrite(output, 1, sizeof(output), out_file);
    
    close_store(helper);
    
    return 0;
}

int multithread_delete_2(char * test_directory) {

    void * helper = init_store(4, 4);
    pthread_create(&my_thread1,NULL,insert_thread_3,helper);
    pthread_create(&my_thread2,NULL,delete_thread_2,helper);
    pthread_join(my_thread1, NULL);
    pthread_join(my_thread2, NULL);

    char file[200];
    sprintf(file, "%s/%s", test_directory, "multithread_delete_2.out");
    printf("writing to: %s\n", file);
    
    FILE * out_file = fopen(file, "w");
    
    char output[] = "no errors\n";
    fwrite(output, 1, sizeof(output), out_file);
  
    close_store(helper);
    
    return 0;
}

void print_tree(void * helper, char * file_name, char * test_directory) {


    char file[200];
    sprintf(file, "%s/%s", test_directory, file_name);
    printf("writing to: %s\n", file);
    char buffer[200];

    FILE * out_file = fopen(file, "w");

    struct node * list = NULL;
    uint64_t num =  btree_export(helper, &list);
    for ( int i =0; i < num; i ++) {
        fwrite("[", 1, 1, out_file );
        //printf("[");
        for (int j = 0; j < (list+i)->num_keys; j ++) {
            sprintf(buffer, "%d", (list+i)->keys[j]);
            fwrite(buffer,1, strlen(buffer), out_file);
          
            //printf("%d", (list+i)->keys[j]);

            if (j != (list+i)->num_keys -1) {
                sprintf(buffer, ",");
                fwrite(buffer,1, strlen(buffer), out_file);
                //printf(",");
            }
               
        }
        sprintf(buffer,"]\n" );
        fwrite(buffer,1, strlen(buffer), out_file);
        //printf("]\n");
    }

    fclose(out_file);
    for (int i = 0; i < num; i ++) {
    
        free((list+i)->keys);
    }
 
    free(list);
    

    
    return;

}
