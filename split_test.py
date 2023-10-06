

import math


class node:
    def __init__(self, children, keys):
        self.children = children
        self.keys = keys
        self.num_children= len(children)
        self.num_keys = len(keys)



num_keys = 4
num_children = 5



n = node([2,5,11,17,20],[3,7,13,19])

left = node([2,5,11,17,20],[3,7,13,19])
right = node([2,5,11,17,20],[3,7,13,19])


middle_index = math.ceil(n.num_keys/2) - 1

for i in range(right.num_keys):
    right.keys[i] = None

for i in range(left.num_keys):
    if (i >= middle_index):
        if (i > middle_index):
            right.keys[i - middle_index - 1] = left.keys[i]
        left.keys[i] = None



for i in range(right.num_children):
    right.children[i] = None

for i in range(left.num_children):
    if ( i >= middle_index +1):
        right.children[i-(middle_index+1)] = left.children[i]
        left.children[i] = None




print("old keys:",n.keys)
print("old kids:",n.children)
print()
print("right keys:",right.keys)
print("right kids",right.children)
print()
print("left keys",left.keys)
print("left kids:",left.children)

