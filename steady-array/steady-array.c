

#include <stdio.h>

#define MAX_NODES 50000000

#define MAX_VERSIONS 1000001

struct Node{
    int max;
    int child[2];
};  

struct Node pool[MAX_NODES];
int nodeCount = 1;

int roots[MAX_VERSIONS];
int levels[MAX_VERSIONS];
int version = 1;

int newNode(){
    int n = nodeCount;
    nodeCount++;
    pool[n].max = 0;
    pool[n].child[0] = 0;
    pool[n].child[1] = 0;
    return n;
}

int bit(unsigned int x, int k){
    return (x >> k) & 1;
}

int fits(unsigned int index, int level){
    return (index >> level) == 0;
}

int maxSub(int node){
    if(node == 0){
        return -1;
    }
    return pool[node].max;
}

int bigger(int a, int b){
    if(a > b){
        return a;
    }
    return b;
}

int set(int old, int level, unsigned int index, int val){
    int n = newNode();
    int bitN;
    int oldChild;

    if(level == 0){
        pool[n].max = val;
        return n;
    }

    bitN = bit(index, level -1);
    oldChild = 0;
    if(old != 0){
        oldChild = pool[old].child[bitN];
    }

    pool[n].child[bitN] = set(oldChild, level - 1, index, val);

    if(old != 0){
        pool[n].child[1 - bitN] = pool[old].child[1 - bitN];
    }

    pool[n].max = bigger(maxSub(pool[n].child[0]), maxSub(pool[n].child[1]));
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
    return pool[node].max;
}
int main(){
    char input[20];
    roots[0] = 0;
    levels[0] = 0;

    while(scanf("%19s", input) == 1){

        int root = roots[version -1];
        int level = levels[version-1];

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
                    pool[n].max = pool[root].max;
                    root = n;
                }
                level++;
            }
            roots[version] = set(root, level, index, val);
            levels[version] = level;
            version++;
        }else if(input[0] == 'g'){
                unsigned int index;
                scanf("%u", &index);
                if(fits(index, level)){
                    printf("%d\n", get(root, level, index));
                } else {
                    printf("0\n");
                }
            } else if(input[0] == 'u'){
            if(version > 1){
                version--;
            }
        } else if(input[0] == 'm'){
            printf("%d\n", bigger(maxSub(root), 0));
        }
    }     
    return 0; 
}