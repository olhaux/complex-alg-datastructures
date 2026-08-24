#include <stdio.h>

#define MAX_NODES 20000000

#define MAX_VERSIONS 1000001

struct Node{
    int max;        // in a leaf this is the value and in an inner node the max below it
    int child[2];   // child 0 is left and child 1 is right where 0 means no child
};

struct Node pool[MAX_NODES];
int nodeCount = 1;      // slot 0 is our null so never hand it out

int roots[MAX_VERSIONS];    // roots[v] and levels[v] describe version v
int levels[MAX_VERSIONS];
int version = 1;            // version 0 is the empty array

int newNode(){
    int n = nodeCount;
    nodeCount++;
    pool[n].max = 0;
    pool[n].child[0] = 0;
    pool[n].child[1] = 0;
    return n;
}

// bit number k of x where bit 0 is the smallest one
// the bits of an index are the path down and we never compare indices
int bit(unsigned int x, int k){
    return (x >> k) & 1;
}

// true if the index needs at most level bits so the tree can hold it
int fits(unsigned int index, int level){
    return (index >> level) == 0;
}

int maxSub(int node){
    if(node == 0){
        // empty so -1 which loses every comparison since values are never negative
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

// how many bits the index needs which is also the height we need
int minBits(unsigned int index){
    int bits = 0;
    while(index > 0){
        bits++;
        index = index >> 1;
    }
    return bits;
}

// tree too short so stack new roots on top until it reaches target
int grow(int root, int level, int target){
    while(level < target){
        if(root != 0){
            int n = newNode();
            pool[n].child[0] = root;        // old root ends up on the left
            pool[n].max = pool[root].max;   // same values under it so same max
            root = n;
        }
        level++;
    }
    return root;
}

// returns the root of the new version
// only the path down to index is copied and that is the whole persistence trick
int set(int old, int level, unsigned int index, int val){
    int n = newNode();
    int bitN;
    int oldChild;

    if(level == 0){
        pool[n].max = val;      // a leaf holds the value itself
        return n;
    }

    bitN = bit(index, level - 1);   // 0 means go left and 1 means go right
    oldChild = 0;
    if(old != 0){
        oldChild = pool[old].child[bitN];
    }

    pool[n].child[bitN] = set(oldChild, level - 1, index, val);

    if(old != 0){
        // we do not copy this side since it points into the old tree
        pool[n].child[1 - bitN] = pool[old].child[1 - bitN];
    }

    // careful here
    // recompute from the children and never compare with the old max
    // the new value can be smaller than the one it replaced
    pool[n].max = bigger(maxSub(pool[n].child[0]), maxSub(pool[n].child[1]));
    return n;
}

int get(int node, int level, unsigned int index){
    // walk down one bit per level
    while(node != 0 && level > 0){
        node = pool[node].child[bit(index, level - 1)];
        level = level - 1;
    }
    if(node == 0){
        return 0;       // nothing was ever stored here
    }
    return pool[node].max;
}

// largest value with index at least left
int maxRight(int node, int level, unsigned int left){
    if(node == 0){
        return -1;
    }
    if(level == 0){
        return pool[node].max;
    }
    if(bit(left, level - 1) == 1){
        // whole left half sits before left so skip it
        return maxRight(pool[node].child[1], level - 1, left);
    }
    // whole right half is inside so it can answer with its max
    return bigger(maxRight(pool[node].child[0], level - 1, left), maxSub(pool[node].child[1]));
}

// largest value with index at most right
// mirror image of maxRight
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

// largest value in the interval and -1 if there is nothing there
// same five cases as the hint in the lab description
int maxSegment(int node, int level, unsigned int left, unsigned int right){
    int leftBit, rightBit;

    if(node == 0){
        return -1;                  // case 1 nothing here at all
    }
    if(level == 0){
        return pool[node].max;      // case 2 we hit a leaf
    }

    // which half each end of the interval goes into
    leftBit = bit(left, level - 1);
    rightBit = bit(right, level - 1);

    if(leftBit == 0 && rightBit == 0){
        // case 3 both ends go left so the answer must be down there
        return maxSegment(pool[node].child[0], level - 1, left, right);
    }
    if(leftBit == 1 && rightBit == 1){
        // case 4 both ends go right
        return maxSegment(pool[node].child[1], level - 1, left, right);
    }
    // case 5 the ends split up so ask each side on its own and keep the best
    return bigger(maxRight(pool[node].child[0], level - 1, left), maxLeft(pool[node].child[1], level - 1, right));
}

int maxInInterval(int root, int level, unsigned int left, unsigned int right){
    if(!fits(left, level)){
        return 0;       // left starts past the end of the tree so nothing can be there
    }
    if(!fits(right, level)){
        // interval runs past the tree so cut it where the tree ends
        right = (1u << level) - 1;
    }
    if(left > right){
        return 0;
    }
    return bigger(maxSegment(root, level, left, right), 0);      // -1 is printed as 0
}

int main(){
    char input[20];
    roots[0] = 0;
    levels[0] = 0;

    while(scanf("%19s", input) == 1){

        // always work from the newest version since unset only moves this back
        int root = roots[version - 1];
        int level = levels[version - 1];

        switch(input[0]){

            case 's': {
                unsigned int index;
                int val;

                scanf("%u %d", &index, &val);

                level = bigger(level, minBits(index));
                root = grow(root, levels[version - 1], level);

                // push it as a new version and leave the old one alone
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
                    printf("0\n");      // index is outside the tree
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
                // step back one version and do nothing if we are already empty
                if(version > 1){
                    version--;
                }
                break;
            }
        }
    }
    return 0;
}