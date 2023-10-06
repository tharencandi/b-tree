#include "btreestore.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MOD_VAL (4294967296)
#define GET_MEDIAN(n) ((ceil( (double) n/2) -1 ))

#define DELTA (0x9E3779B9)
#define ENC_DEC_ITER (1024)
#define DEC_SUM (0xDDE6E400)
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


/*
    struct that holds (key,value) pair in the dictionary.
*/
typedef struct key_val {
    uint32_t key;
    void * value;
    uint32_t byte_size;
    uint64_t nonce;
    uint32_t encryption_key[4];
    uint32_t padding;

} key_val;


/*
    elements is an array of key_val struct pointers with size b
    children is an array of internal_node pointers with size b
    parent is a pointer to parent internal_node.
*/
typedef struct internal_node {

    int num_children;
    int num_keys;

    key_val ** elements;
    struct internal_node ** children;
    struct internal_node * parent;

} internal_node;


typedef struct tree_data {
    int threads;
    uint16_t branching_factor;
    internal_node * head;
    uint64_t num_nodes;
    pthread_t * my_threads;

} tree_data;

/*
    initisialises tree data structures.
    branching - the branching factor, b, of the trree
    n_processors - the amount of threads (unused)
*/
void * init_store(uint16_t branching, uint8_t n_processors) {
    
    pthread_mutex_lock(&lock);
    int threads = n_processors;
    tree_data * tree = malloc(sizeof(tree_data));
    tree->head = malloc(sizeof(internal_node));
    tree->head->children = (internal_node **) malloc(sizeof(internal_node*) * (branching+1));
    tree->head->elements = (key_val **) malloc(sizeof(key_val*) * branching);
    tree->head->parent = NULL;
    tree->head->num_children = 0;
    tree->head->num_keys = 0;
    tree->branching_factor = branching;
    tree->num_nodes = 1;
    tree->threads = threads;
    tree->my_threads = (pthread_t *) malloc(sizeof(pthread_t) * threads);

   
    pthread_mutex_unlock(&lock);
    return tree;
}



/*
    used to test node order. not accessible outside .c file.
*/
void testing(internal_node * node) {

    
    printf("keys: ");
    printf("[");
    for (int i = 0; i < node->num_keys; i ++) {
        printf("%d, ", node->elements[i]->key);
    }
     printf("]\n");

    if (node->children == 0) {
        return;
    } else {
        printf("has children %d:\n", node->num_children);
        for (int i = 0; i < node->num_children; i ++) {
            printf("    %p\n",node->children[i] );
            if (node->children[i]->parent != node) {
                printf("INCORRECT PARENT\n");
            }
            
            testing(node->children[i]);
        }
    }
}




/*
    recursive function to delete all nodes in the tree safley
*/
void delete_nodes(internal_node * node) {
    
    if (node == NULL) {
        return;
    }

    if (node->num_children > 0) {
       for (int i = 0; i <node->num_children; i ++) {
        delete_nodes(node->children[i]);
        }   
    }

    for (int i = 0; i < node->num_keys; i ++) {

        free(node->elements[i]->value);
        free((void*)node->elements[i]);
    }

    free((void*)node->elements);
    free((void*)node->children);
    free((void*)node);

}


/*
    deletes all nodes and another other information in tree_data
*/
void close_store(void * helper) {
    pthread_mutex_lock(&lock);
    internal_node * head = ((tree_data *) helper)->head;
    delete_nodes(head);
    free(((tree_data*)helper)->my_threads);
    free(helper);
    pthread_mutex_unlock(&lock);
    return;
}

/*
    returns index in key array
    -1 if key does not exist.
*/
int find_key(uint32_t key, key_val ** elements, int length) {
    
    if (elements == NULL)
        return -1;


    for (int i = 0; i < length; i ++ ) {
        if (elements[i]->key == key) {
            return i;
        }
    }
    return -1;
}


/*
    returns the index of input pointer in an array of pointers
    -1 if it is not found
*/
int find_index(void * element, void ** array, int size) {

    if (element == NULL || array == NULL) {
        return -1;
    }

    for (int i = 0; i < size; i ++) {   
        if (array[i] == element) {
            return i;
        }
    }

    return -1;

}


/*
    returns the correct index position to insert a child in parent children array based on its key
        - key: key of child that you want to insert
        - parent: the node that holds the children array
        - length: length of children array
*/
int get_correct_child(uint32_t key, internal_node * parent, int length) {

    if (parent == NULL)
        return -1;

    int child_index = -1;
    key_val ** elements = parent->elements;

    if (key < (elements[0]->key)) {
        child_index = 0;
     
    } else if (key > elements[length - 1]->key) {
      
        child_index = length;

    } else {

        for (int i = 1; i < length; i ++) {
            if ((key > elements[i-1]->key) && (key < elements[i]->key)) 
                child_index = i;
        }
    }

    return child_index;
}   


/*
    searches entire tree for node that contains the input key.
    returns -1 if the key was not found 
    returns 1 if there was an error
    returns 0 if it actually found the key
*/
int search(uint32_t key,  internal_node ** node, void * helper) {

    *node = NULL;
    tree_data * data = (tree_data *) helper;
    internal_node * current_node = data->head;
   
    int key_index = 0;
    int child_index = -1;
    while (1) {
        key_index = find_key(key, current_node->elements, current_node->num_keys);    
     
        if (key_index > -1) {
            //found correct node
            *node = current_node;
            return 0;
        }
        if (current_node->num_children == 0) {
            //at leaf (not found)
            *node = current_node;
            return -1;
        }
        /*
            identify correct child Ci to follow.
            ki-1 < k < ki
        */
        if ((child_index = get_correct_child(key, current_node, current_node->num_keys)) == -1) {
            //fprintf(stderr,"error finding child\n");
            return 1;
        }
        current_node = current_node->children[child_index];

    }

    return 0;

}


/*
    insert as position (index) void * element. the array is void **, with length size+1
*/
int array_insert(int index, void * element, void ** array, int size) {

    if (element == NULL  || array == NULL || index > size) {
        return -1;
    }
    
    for (int i = size; i >= 0; i --) {

        if ( i == index) {
            array[i] = element;
            return 0;
        }

        if (i > 0) {
            array[i] = array[i-1]; 
        }
    }
    return -1;
}


/*
    function will split node into left and right and redistribute keys and children appropriatley
    function will also create a new root node if it needs to (split + new root).
*/
void split_node(int middle_index, internal_node * node, tree_data * data ) {

    uint16_t branching = data->branching_factor;
    //create left and right
    //printf("creating right \n");

    
    internal_node * left_node = node;
    
    internal_node * right_node = (internal_node *) malloc(sizeof(internal_node));
     
    right_node->elements = (key_val **) malloc(sizeof(key_val*)*branching);
    
    right_node->children = (internal_node **) malloc(sizeof(internal_node*) *(branching+1));    
    right_node->num_children = left_node->num_children;
    
    
    /*
        create new root if the root is being split. 
    */
    if (node->parent == NULL) {
        //printf("creating new root\n");
        internal_node * new_root = (internal_node *) malloc(sizeof(internal_node));
        new_root->elements = (key_val **) malloc(sizeof(key_val*) * branching);
        new_root->children = (internal_node **) malloc(sizeof(internal_node*) *(branching+1));
        new_root->parent = NULL;
        new_root->num_children = 1;
        new_root->children[0] = left_node;
        new_root->num_keys = 0;
        left_node->parent = new_root; 
        data->head = new_root;
    }
 
    /*
        re distribute key-vals
    */

  
    for (int i = 0; i < left_node->num_keys; i ++) {

        if (i >= middle_index) {    
            if (i > middle_index) {
                right_node->elements[i - middle_index - 1] = left_node->elements[i];   
            }
            left_node->elements[i] = NULL;
        }

    }

    int old_keys = left_node->num_keys;
    right_node->parent = left_node->parent;
    int right_num_keys = left_node->num_keys - (middle_index + 1);
    left_node->num_keys =  (left_node->num_keys-1) - right_num_keys;
    right_node->num_keys = right_num_keys;
   

    /*
        re distribute children 
            - middle_index + 1 children go to left
            - num_children - (middle_index + 1) go to right
    */
    if (left_node->num_children > 0 ) {
        
        for (int i = 0; i < right_node->num_children; i ++) {
            right_node->children[i] = NULL;
        }

        for (int i = 0; i < left_node->num_children; i ++) {

            if (i >= middle_index + 1) {
                right_node->children[i - (middle_index + 1)] = left_node->children[i];
                right_node->children[i - (middle_index + 1)]->parent = right_node;
                left_node->children[i] == NULL;
            }
        }

        left_node->num_children = middle_index + 1;
        right_node->num_children = right_node->num_children  -left_node->num_children;

    }
 
    
    int index = find_index((void *)left_node, (void **)left_node->parent->children, branching);
    if (array_insert(index+1, (void*) right_node, (void**)node->parent->children, branching) == -1) {
        //fprintf(stderr, "error insert right node into parent children array\n");
        return;
    }


    // add right child to children arr of parent
    left_node->parent->num_children = left_node->parent->num_children + 1;

}


/*
    returns the index that the key should be inserted into
    -1 if error
*/
int get_index_for_key_insert(key_val * new_element, key_val ** elements, int length) {

    if (new_element == NULL || elements == NULL || length < 0)
        return -1;

    int ret_index = -1;
    if ((new_element->key) < (elements[0]->key)) {
     
        ret_index = 0;
    } else if (new_element->key > elements[length-1]->key) {
        ret_index = length;
    }
    else {
        for (int i = 0; i < length; i ++) {
            if (elements[i]->key == new_element->key) {
                return -1;
            }
            if ((elements[i]->key) < (new_element->key) && elements[i+1]->key > (new_element->key)) {
                ret_index = i + 1;
            }
        }
    }

    return ret_index;
}


/*
    inserts key into the appropriate index of the key_val array
*/
int insert_key(key_val * elem, internal_node * node, tree_data * data ) {

    if (elem == NULL || node == NULL) 
        return 1;
    
    if (node->num_keys == 0) {     
        node->elements[0] = elem;
        node->num_keys = node->num_keys + 1;
        return 0;
    }

    int cur = -1;
    if ((cur = get_index_for_key_insert(elem, node->elements, node->num_keys)) == -1) {
        printf("no index found\n");
        return 1;
    }
        
    if (array_insert(cur, (void*)elem, (void**)node->elements, node->num_keys) != -1) {
        node->num_keys = node->num_keys + 1;
    } else {
        return 1;
    }
    

    if (node->num_keys > data->branching_factor - 1) {
        
        int middle_index = GET_MEDIAN(node->num_keys);
        key_val * middle_key =  node->elements[middle_index];

        if (node->parent == NULL) {
           data->num_nodes ++;
        }
   
        split_node(middle_index, node, data);      
        data->num_nodes ++;
      
        return insert_key(middle_key, node->parent, data);
    } 

  
    return 0;

}


/*
    encrypt plaintext, creates key and puts it in the tree
*/
int btree_insert(uint32_t key, void * plaintext, size_t count, uint32_t encryption_key[4], uint64_t nonce, void * helper) {
 
    if (helper == NULL || plaintext == NULL || encryption_key == NULL)
        return 1;

    pthread_mutex_lock(&lock);
    tree_data * data = (tree_data *)(helper);
    internal_node * leaf;
    if (search(key, &leaf, helper) != -1) {
        //fprintf(stderr, "error. key %d is already in the dictionary\n", key);
        pthread_mutex_unlock(&lock);
        return 1;
    }
    

    /*
        create new key_val data type and fill all fields
    */
    unsigned long remainder = count % sizeof(uint64_t);
    size_t padded_length = count;
    if (remainder != 0) {
        padded_length += (sizeof(uint64_t) - remainder);
    }

    
    key_val * new_elem = (key_val*) malloc(sizeof(key_val));
    new_elem->key = key;
    new_elem->value = NULL;
    new_elem->nonce = nonce;
    new_elem->byte_size =  count;
    new_elem->padding = padded_length - count;
    void * padded_plaintext = malloc(padded_length);

    memset(padded_plaintext, 0, padded_length);
    memcpy(padded_plaintext, plaintext, count);
    memcpy(new_elem->encryption_key,encryption_key, sizeof(uint32_t) * 4);

    uint64_t * cipher = (uint64_t *) malloc(padded_length );
   
    // encrypt plain text to stor
    encrypt_tea_ctr((uint64_t*)padded_plaintext, encryption_key, nonce, cipher, padded_length/sizeof(uint64_t));
    free(padded_plaintext);

    new_elem->value = (void*) (cipher);

    /*
        insert k into leaf
    */
    
    insert_key(new_elem, leaf, data);

    //testing(data->head);
    pthread_mutex_unlock(&lock);
    return 0;
    

}



/*
    expexts two arrays and an index of the keys swapping in both arrays.
*/
int swap_keys(key_val ** elements_1, int key_index_1, key_val ** elements_2, int key_index_2 ) {

    if ((elements_1 == NULL) || (elements_2 == NULL) 
    || ( key_index_1 < 0) || ( key_index_2 < 0) ) {
        
        return 1;
    }
        

    key_val * temp_key = elements_1[key_index_1];
    elements_1[key_index_1] = elements_2[key_index_2];
    elements_2[key_index_2] = temp_key;
    return 0;
}


/*
    removes key @ node_index and updates num keys 
*/
int delete_key(internal_node * node, int node_index) {

    if (node == NULL || node_index < 0)
        return 1;

    if (node->num_keys == 1 && node_index == 0) {
        node->elements[0] = NULL;
        node->num_keys = 0;
        return 0;
    }

    for (int i = node_index; i < (node->num_keys)-1; i ++) {
        node->elements[i] = node->elements[i+1];
    }

    node->elements[node->num_keys-1] = NULL;
    node->num_keys = node->num_keys -1;

    return 0;

}

/*
    will delete child at specified index in the child array of an input node
    will update other struct paramaters accordingly
*/
int delete_child(internal_node * node, int child_index) {

    if (node == NULL || child_index < 0)
        return 1;

    for (int i = child_index; i < (node->num_children)-1; i ++) {
        node->children[i] = node->children[i+1];
    }

    node->children[node->num_children-1] = NULL;
    node->num_children = node->num_children -1;

    return 0;
}



/*
    node_1 <- node_1 + node_2

    - move all keys and children from the node_2 to node_1
    - move parent key that separates siblings into node_1 (kleft if left sibling...)
 */           
int merge_siblings(internal_node * node_1, internal_node * node_2, int node_1_index, int is_left, tree_data * data ) {

    /*
        merge keys
    */
    for (int i = 0; i < node_2->num_keys; i ++) {
        if (insert_key(node_2->elements[i], node_1, data) == 1) {
            return 1;
        }
    }
    
    /*
        merge children
    */
    int offset = 0;
    if (is_left == 0)
        offset = node_1->num_children;
    
    for (int i = 0; i < node_2->num_children; i ++) {
        if (array_insert(i + offset, (void *) node_2->children[i], (void **) node_1->children, data->branching_factor) == 1) {
            return 1;
        }
        node_2->children[i]->parent = node_1;
        
    }

    node_1->num_children = node_1->num_children + node_2->num_children;    
    
    /*
        merge parent key
    */
    if (is_left == 1) {
        if (insert_key(node_1->parent->elements[node_1_index-1],node_1, data) == 1)
            return 1;
        
        if (delete_key(node_1->parent,node_1_index-1) == 1)
            return 1;
        
        if (delete_child(node_1->parent, node_1_index-1) == 1)
            return 1;
        
    } else {
       
        if (insert_key(node_1->parent->elements[node_1_index],node_1, data) == 1)
            return 1;
        
        if (delete_key(node_1->parent,node_1_index) == 1)
            return 1;

        if (delete_child(node_1->parent, node_1_index+1) == 1)
            return 1;

             
    }

    /*
        free sibling that was merged
    */
    
    data->num_nodes --;
    free(node_2->children);
    free(node_2->elements);
    free(node_2);
    
    return 0;


}



/*
    rebalancing function after a key has been deleted
*/
void delete_recurse(internal_node * target_node, tree_data * data) {

     /*
        if leaf (target) has < minkeys 

            - check if left sibling of target node has > minkeys
                Y: replace parent K left of target node with max(key) in left sibling 
            - else check if right sibling has > minkeys 
                Y:  - move smallest key in right to parent K right of target node
                    - move old parent k right of target node into target node
                    - 
            - complete if one of these cases was met 
    */
    

    //find index of target node in its parents children list

    
   
    int parents_index_of_target = find_index((void*) target_node, (void **)target_node->parent->children, target_node->parent->num_children);
    //printf("parent: %d\n", parents_index_of_target);
    if (parents_index_of_target == -1) {
        //printf("error finding node in parents children list\n");
        return;
    }
 
    //check left sibling
    if (parents_index_of_target != 0) {
        internal_node * left_sibling = target_node->parent->children[parents_index_of_target -1];
        
      
        if (((left_sibling->num_children == 0) && (left_sibling->num_keys > 1))
            || ((left_sibling->num_children) > 0 && (left_sibling->num_keys > (GET_MEDIAN(data->branching_factor/2))+1))
        ) {

            pthread_mutex_lock(&lock);
            // can make swap
            key_val * old_parent = target_node->parent->elements[parents_index_of_target-1];
            swap_keys(target_node->parent->elements, parents_index_of_target-1, left_sibling->elements, left_sibling->num_keys-1);            
            insert_key(old_parent, target_node, data);


            //deal with subtrees - CRIGHT TO LEFT MOST CHILD OF TARGET
            if (left_sibling->num_children > 0) {

                internal_node * left_sibling_child = left_sibling->children[left_sibling->num_children-1];
                array_insert(0, (void *) left_sibling_child, (void **) target_node->children, target_node->num_children);
                left_sibling_child->parent = target_node;
                delete_child(left_sibling, left_sibling->num_children-1);
                target_node->num_children ++;

            }

            // now remove from child:
            delete_key(left_sibling, left_sibling->num_keys-1);
            pthread_mutex_unlock(&lock);

            return;
   
        }

    //check right sibling
    }
    if(parents_index_of_target < target_node->parent->num_children -1) {
        internal_node * right_sibling = target_node->parent->children[parents_index_of_target + 1];
        
        if (((right_sibling->num_children == 0) && (right_sibling->num_keys > 1)) 
            || ((right_sibling->num_children) > 0 && (right_sibling->num_keys > (GET_MEDIAN(data->branching_factor/2))+1))
        ) { 
            pthread_mutex_lock(&lock);
            // can make swap
            key_val * old_parent = target_node->parent->elements[parents_index_of_target]; //kright
            swap_keys(target_node->parent->elements, parents_index_of_target, right_sibling->elements, 0);  //swapping parent to be kright child   
            insert_key(old_parent, target_node, data);      // kright goes to target

            //deal with subtrees - CRIGHT TO RIGHT MOST CHILD OF TARGET
            if (right_sibling->num_children > 0) {
                internal_node * right_sibling_child = right_sibling->children[0];
                array_insert(target_node->num_children, (void *) right_sibling_child, (void **) target_node->children, target_node->num_children);
                right_sibling_child->parent = target_node;
                delete_child(right_sibling, 0);
                target_node->num_children ++;

            }
            
            // now remove from child:
            delete_key(right_sibling, 0);
            pthread_mutex_unlock(&lock);
            return;
        } 
        
    } 

 
     /*
        if no left or right siblings with > minkey
        
        - merge target node with sibling (priority left)
           
            - if parent has < minkeys: recurse 
   */

    if (parents_index_of_target != 0) {
        pthread_mutex_lock(&lock);
       // printf("MERGING LEFT\n");
        merge_siblings(target_node, target_node->parent->children[parents_index_of_target-1],parents_index_of_target, 1, data);
        pthread_mutex_unlock(&lock);
    } else {
       
        pthread_mutex_lock(&lock);
        merge_siblings(target_node, target_node->parent->children[parents_index_of_target+1], parents_index_of_target, 0, data);
        pthread_mutex_unlock(&lock);
    }

    /*
        target node is root
            - delete root node
            - new root is merged child
    */
    if (target_node->parent->parent == NULL && target_node->parent->num_keys < 1) {
 
        pthread_mutex_lock(&lock);
        data->head = target_node;
        data->num_nodes --;
        free(target_node->parent->children);
        free(target_node->parent->elements);
        free(target_node->parent);
        target_node->parent =NULL;
        pthread_mutex_unlock(&lock);
        
        return;
        

    } else if ((target_node->parent->parent != NULL )&& (target_node->parent->num_keys < (GET_MEDIAN(data->branching_factor/2))+1)) {
        
        delete_recurse(target_node->parent, data);
    }

    return;
}



/*
    sends key to leaf, deletes key and rebalances the tree if it needs to
*/
int btree_delete(uint32_t key, void * helper) {

    if (helper == NULL) {
        return 1;
    }

    tree_data * data = (tree_data *)(helper);
    internal_node * target_node;
    if (search(key, &target_node, helper) == -1) {
        //fprintf(stderr, "error. key %d is not in the dictionary\n", key);
        return 1;
    }

 

    /*
        If node is internal - 
            - Swap K with largest key in the ENTIRE SUBTREE rooted at left child separating K (will be right-most leaf of subtree.
            
    */
    int target_key_index = find_key(key, target_node->elements, target_node->num_keys);
    
    if (target_node->num_children > 0) {
        
        // rule: left child of key i is ith child, right child of key i is i+1th child.
        internal_node * subtree_node = target_node->children[target_key_index];

        while(subtree_node->num_children > 0) {
            subtree_node = subtree_node->children[subtree_node->num_children-1];
        }

        //swapping keys
        pthread_mutex_lock(&lock);
        swap_keys(target_node->elements, target_key_index, subtree_node->elements, subtree_node->num_keys-1);
        
        target_node = subtree_node;
        target_key_index = subtree_node->num_keys-1;
        pthread_mutex_unlock(&lock);
    }


    /*
        delete K from the leaf node
            -complete if leaf keys >= minkeys
    */
    //printf("target node key: %d\n", target_node->elements[0]->key);
    pthread_mutex_lock(&lock);
    free(target_node->elements[target_key_index]->value);
    free(target_node->elements[target_key_index]);
    //printf("deleting keu\n");
    delete_key(target_node, target_key_index);
    pthread_mutex_unlock(&lock);
   
    
    if (target_node->num_keys > 0 || (target_node->parent == NULL && target_node->num_keys >= 0)) {  
        return 0;
    }

    /*
        rebalance
    */
    delete_recurse(target_node, data);
    
    return 0;
}



/*
    retreives the element of a node by searching the tree with an input key
*/
int get_element(uint32_t key, void * helper, key_val ** element) {

    internal_node * node = NULL;
    if (search(key, &node, helper) == -1) {
        //fprintf(stderr,"error. key %d is not in the dictionary\n", key);
        return 1;
    }
    
    if (node == NULL) {
        //fprintf(stderr,"error finding node\n");
        return 1;
    }

    *element = NULL;

    for (int i = 0; i < node->num_keys; i ++) {
       
        if (node->elements[i]->key == key) {
            *element = node->elements[i];
            break;
        }
    }   

    if (*element == NULL) {
        //fprintf(stderr,"error finding key\n");
        return 1;
    }

}


/*
    formats output info of a key
*/
int btree_retrieve(uint32_t key, struct info * found, void * helper) {
    
    if (helper == NULL)
        return 1;

    key_val * element;
    if (get_element(key, helper, &element) == 1)
        return 1;

    found->data = element->value;
    memcpy(found->key, element->encryption_key, sizeof(uint32_t)*4);
    found->nonce = element->nonce;
    found->size = element->byte_size;
    return 0;
    
}


/*
    search for key and decrypt its value.

    ret 
        - 0 for success
        - 1 forerror
*/
int btree_decrypt(uint32_t key, void * output, void * helper) {

    if (helper == NULL)
        return 1;

    key_val * element;
    if (get_element(key, helper, &element) == 1)
        return 1;

    void * plaintext = malloc(element->byte_size + element->padding);
    memset(plaintext, 0, element->byte_size + element->padding);

    if (element->padding != 0) {

        void * cipher = malloc(element->byte_size + element->padding);
        memset(cipher, 0, element->byte_size + element->padding );
        memcpy(cipher, element->value, element->byte_size);
        decrypt_tea_ctr(element->value, element->encryption_key, element->nonce, plaintext, (element->byte_size+element->padding)/sizeof(uint64_t));
        free(cipher);
    } else {

        decrypt_tea_ctr(element->value, element->encryption_key, element->nonce, plaintext, (element->byte_size+element->padding)/sizeof(uint64_t));

    }
   
    memcpy(output, plaintext, (element->byte_size));
   
    free(plaintext);

    
    return 0;

}



/*
    adds all nodes in pre order to the list
*/
void pre_order_traversal(internal_node * tree_node, struct node * list, int * count, uint64_t num_nodes) {
    
    struct node * output_node  = &(list[*count]);
    uint32_t * keys = (uint32_t *) malloc(sizeof(uint32_t) * tree_node->num_keys);
   
    output_node->keys = keys;
    output_node->num_keys = tree_node->num_keys;
    
  
    for (int i = 0; i < output_node->num_keys; i ++ ) {
        output_node->keys[i] = tree_node->elements[i]->key;
    }
    
    (*count) = (*count) +1;
    if (*count >= num_nodes-1) {
        return;
    } else if (tree_node->num_children > 0) {
        for (int i =0; i < tree_node->num_children; i ++) {
            
            pre_order_traversal(tree_node->children[i], list, count, num_nodes);
        }
    }
}


uint64_t btree_export(void * helper, struct node ** list) {

    if (helper == NULL || list == NULL)
        return 0;

    pthread_mutex_lock(&lock);
    tree_data * data = (tree_data *) helper;
 
    *list = (struct node *) malloc(sizeof(struct node) *  data->num_nodes );
    int count = 0;
    pre_order_traversal(data->head, *list, &count,data->num_nodes);    
    pthread_mutex_unlock(&lock);
    return data->num_nodes;
}



void encrypt_tea(uint32_t plain[2], uint32_t cipher[2], uint32_t key[4]) {
    
    if (plain == NULL || cipher == NULL || key == NULL)
        return;

    uint64_t sum = 0;
    uint64_t delta = DELTA;
    cipher[0] = plain[0];
    cipher[1] = plain[1];


    uint64_t tmp1;
    uint64_t tmp2; 
    uint64_t tmp3; 
    uint64_t tmp4;
    uint64_t tmp5;
    uint64_t tmp6;

    for (int i = 0; i < ENC_DEC_ITER; i ++) {

        sum = (sum + delta) % MOD_VAL;

        tmp1 = ((cipher[1] << 4) + key[0]) % MOD_VAL;
        tmp2 = (cipher[1] + sum) % MOD_VAL;
        tmp3 = ((cipher[1] >> 5) + key[1]) % MOD_VAL;
        
        cipher[0] = (cipher[0] + (tmp1^tmp2^tmp3)) % MOD_VAL;
        
        tmp4 = ((cipher[0] << 4) + key[2]) % MOD_VAL;
        tmp5 = (cipher[0] + sum) % MOD_VAL;
        tmp6 = ((cipher[0] >> 5) + key[3]) % MOD_VAL;
        cipher[1] = (cipher[1] + (tmp4^tmp5^tmp6)) % MOD_VAL;
    }
    
    return;
}


void decrypt_tea(uint32_t cipher[2], uint32_t plain[2], uint32_t key[4]) {
    
    if (plain == NULL || cipher == NULL || key == NULL)
        return;
    
    uint64_t sum = DEC_SUM;
    uint64_t delta = DELTA;

    uint64_t tmp1;
    uint64_t tmp2; 
    uint64_t tmp3; 
    uint64_t tmp4;
    uint64_t tmp5;
    uint64_t tmp6;
    for (int i = 0; i < ENC_DEC_ITER; i ++) {
        
        tmp4 = ((cipher[0] << 4) + key[2]) % MOD_VAL;
        tmp5 = (cipher[0] + sum) % MOD_VAL;
        tmp6 = ((cipher[0] >> 5) + key[3]) % MOD_VAL;
        
        cipher[1] = (cipher[1] - (tmp4^tmp5^tmp6)) % MOD_VAL;
        
        tmp1 = ((cipher[1] << 4) + key[0]) % MOD_VAL;
        tmp2 = (cipher[1] + sum) % MOD_VAL;
        tmp3 = ((cipher[1] >> 5) + key[1]) % MOD_VAL;
        
        cipher[0] = (cipher[0] - (tmp1^tmp2^tmp3)) % MOD_VAL;
        
        sum = (sum - delta) % MOD_VAL;
    }
    plain[0] = cipher[0];
    plain[1] = cipher[1];
    return;
}

void encrypt_tea_ctr(uint64_t * plain, uint32_t key[4], uint64_t nonce, uint64_t * cipher, uint32_t num_blocks) {
    
    if (plain == NULL || cipher == NULL || key == NULL)
        return;

    //plain is an array of uint64 of size num_blocks

    uint64_t tmp1;
    uint64_t tmp2;
    for (int i = 0; i < num_blocks; i ++) {
   
        tmp1 = i^nonce;
        encrypt_tea((uint32_t *) &tmp1, (uint32_t *) &tmp2, key);
       
        cipher[i] = plain[i]^tmp2;
    }

    return;
}




void decrypt_tea_ctr(uint64_t * cipher, uint32_t key[4], uint64_t nonce, uint64_t * plain, uint32_t num_blocks) {
    
    if (plain == NULL || cipher == NULL || key == NULL)
        return;

    uint64_t tmp1;
    uint64_t tmp2;
    for (int i = 0; i < num_blocks; i ++) {
        tmp1 = i^nonce;
        encrypt_tea((uint32_t *) &tmp1, (uint32_t *) &tmp2, key);
        plain[i] = cipher[i]^tmp2;
    }

    return;
}