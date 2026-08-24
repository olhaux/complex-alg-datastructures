#include <stdio.h>

#define MAX_NODES 20000000

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

int minBits(unsigned int index){
    int bits = 0;
    while(index > 0){
        bits++;
        index = index >> 1;
    }
    return bits;
}

int grow(int root, int level, int target){
    while(level < target){
        if(root != 0){
            int n = newNode();
            pool[n].child[0] = root;
            pool[n].max = pool[root].max;
            root = n;
        }
        level++;
    }
    return root;
}

int set(int old, int level, unsigned int index, int val){
    int n = newNode();
    int bitN;
    int oldChild;

    if(level == 0){
        pool[n].max = val;
        return n;
    }

    bitN = bit(index, level - 1);
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

int get(int node, int level, unsigned int index){
    while(node != 0 && level > 0){
        node = pool[node].child[bit(index, level - 1)];
        level = level - 1;
    }
    if(node == 0){
        return 0;
    }
    return pool[node].max;
}

int maxRight(int node, int level, unsigned int left){
    if(node == 0){
        return -1;
    }
    if(level == 0){
        return pool[node].max;
    }
    if(bit(left, level - 1) == 1){
        return maxRight(pool[node].child[1], level - 1, left);
    }
    return bigger(maxRight(pool[node].child[0], level - 1, left), maxSub(pool[node].child[1]));
}

int maxLeft(int node, int level, unsigned int right){
    if(node == 0){
        return -1;
    }
    if(level == 0){
        return pool[node].max;
    }
    if(bit(right, level - 1) == 0){
        return maxLeft(pool[node].child[0], level - 1, right);
    }
    return bigger(maxSub(pool[node].child[0]), maxLeft(pool[node].child[1], level - 1, right));
}

int maxSegment(int node, int level, unsigned int left, unsigned int right){
    int leftBit, rightBit;

    if(node == 0){
        return -1;
    }
    if(level == 0){
        return pool[node].max;
    }

    leftBit = bit(left, level - 1);
    rightBit = bit(right, level - 1);

    if(leftBit == 0 && rightBit == 0){
        return maxSegment(pool[node].child[0], level - 1, left, right);
    }
    if(leftBit == 1 && rightBit == 1){
        return maxSegment(pool[node].child[1], level - 1, left, right);
    }
    return bigger(maxRight(pool[node].child[0], level - 1, left), maxLeft(pool[node].child[1], level - 1, right));
}

int maxInInterval(int root, int level, unsigned int left, unsigned int right){
    if(!fits(left, level)){
        return 0;
    }
    if(!fits(right, level)){
        right = (1u << level) - 1;
    }
    if(left > right){
        return 0;
    }
    return bigger(maxSegment(root, level, left, right), 0);
}

int main(){
    char input[20];
    roots[0] = 0;
    levels[0] = 0;

    while(scanf("%19s", input) == 1){

        int root = roots[version - 1];
        int level = levels[version - 1];

        switch(input[0]){

            case 's': {
                unsigned int index;
                int val;

                scanf("%u %d", &index, &val);

                level = bigger(level, minBits(index));
                root = grow(root, levels[version - 1], level);

                roots[version] = set(root, level, index, val);
                levels[version] = level;
                version++;
                break;
            }

            case 'g': {
                unsigned int index;

                scanf("%u", &index);

                if(fits(index, level)){
                    printf("%d\n", get(root, level, index));
                } else {
                    printf("0\n");
                }
                break;
            }

            case 'm': {
                unsigned int left, right;

                scanf("%u %u", &left, &right);

                printf("%d\n", maxInInterval(root, level, left, right));
                break;
            }

            case 'u': {
                if(version > 1){
                    version--;
                }
                break;
            }
        }
    }
    return 0;
}