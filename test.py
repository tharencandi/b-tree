


k = 4

A = [1,3,6,8,10, None]

print(A)
cur = -1
for i in range(len(A)-1):
    if A[i] < k and A[i+1] > k:
        cur = i+1


for i in reversed(range(len(A))):

    A[i] = A[i -1]

    if i == cur:
        A[i] = k
        break

print(A)