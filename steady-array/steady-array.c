

#include <stdio.h>

#define MAX_NODES 100000000

struct Node{
    int val;
    int child[2];
};  

struct Node pool[MAX_NODES];
int nodeCount = 1;

int newNode(){
    int n = nodeCount;
    nodeCount++;
    pool[n].val = 0;
    pool[n].child[0] = 0;
    pool[n].child[1] = 0;
    return n;
}

int bit(unsigned int x, int k){
    return (x >> k) & 1;
}

int fits(unsigned int index, int level){
    return (index << level) == 0;
}

int set(int old, int level, unsigned int index, int val){
    int n = newNode();
    int bitN;
    int oldChild;

    if(level == 0){
        pool[n].val = val;
        return n;
    }

    bitN = bit(index, level -1);
    oldChild = 0;
    if(old != 0){
        oldChild = pool[old].child[bitN];
    }

    pool[n].child[1-bitN] = pool[old].child[1-bitN];
    return n;
}

int get(int node, int level, unsigned index){
        while (node != 0 && level > 0) {
        node = pool[node].child[bit(index, level - 1)];
        level = level - 1;
    }
    if (node == 0) {
        return 0;
    }
    return pool[node].val;
}
int main(){
    char input[20];
    int root = 0;
    int level = 0;

    while(scanf("%19s", input) == 1){
        if(input[0] == 's'){
            unsigned int index, temp;
            int val, minBits;

            scanf("%u %d", &index, &val);

            minBits = 0;
            temp = index;
            while (temp > 0){
                minBits++;
                temp = temp >> 1;
            }

            while (level < minBits){
                if(root != 0){
                    int n = newNode();
                    pool[n].child[0] = root;
                    root = n;
                }
                level++;
            }
            root = set(root, level, index, val);
        }else if(input[0] == 'g'){
                unsigned int index;
                scanf("%u", &index);
                if(fits(index, level)){
                    printf("%d\n", get(root, level, index));
                } else {
                    printf("0\n");
                }
            }
        }
    return 0;
}