/* Tests for steady-array.c.   gcc -O2 -o tests tests.c && ./tests
 *
 * steady-array.c stays one self-contained file so it can go to Kattis as is,
 * which means there is no header to include. So we include the .c file itself
 * and rename its main out of the way, because this file needs its own main.
 */
#define main steady_array_main
#include "steady-array.c"
#undef main

#define MAXINDEX 2147483647u    /* the largest index according to lab */

static int checks = 0;
static int failures = 0;

static void eq(int got, int want, const char *what)
{
    checks++;
    if(got != want){
        failures++;
        printf("  FAIL  %s: got %d, expected %d\n", what, got, want);
    }
}

/* for checks that are just true or false */
static void ok(int cond, const char *what)
{
    checks++;
    if(!cond){
        failures++;
        printf("  FAIL  %s\n", what);
    }
}

/* ---- the four commands, exactly as main runs them ---- */

static void reset(void)
{
    nodeCount = 1;      /* newNode clears each slot as it hands it out, */
    version   = 1;      /* so leftovers from an earlier test are fine   */
    roots[0]  = 0;
    levels[0] = 0;
}

static void t_set(unsigned int index, int val)
{
    int root  = roots[version - 1];
    int level = levels[version - 1];

    level = bigger(level, minBits(index));
    root  = grow(root, levels[version - 1], level);

    roots[version]  = set(root, level, index, val);
    levels[version] = level;
    version++;
}

static int t_get(unsigned int index)
{
    int root  = roots[version - 1];
    int level = levels[version - 1];

    if(fits(index, level)){
        return get(root, level, index);
    }
    return 0;
}

static int t_max(unsigned int left, unsigned int right)
{
    return maxInInterval(roots[version - 1], levels[version - 1], left, right);
}

static void t_unset(void)
{
    if(version > 1){
        version--;
    }
}

/* ================= the eleven cases ================= */

/* 01  The example printed in the lab text. If this breaks, everything is
       wrong. It also covers an overwrite, an interval query and three
       unsets. */
static void test_lab_sample(void)
{
    reset();
    t_set(3, 17);
    t_set(3, 4711);
    eq(t_get(3), 4711, "01 get 3");
    t_set(2, 20);
    eq(t_max(1, 3), 4711, "01 maxininterval 1 3");
    t_unset();
    t_set(3, 1000);
    t_unset();
    eq(t_get(3), 4711, "01 get 3 after unset");
    t_unset();
    eq(t_get(3), 17, "01 get 3 after two unsets");
}

/* 02  Every query must survive root == 0 and level == 0 before anything is
       stored, and unset on an empty array must not step below version 0. */
static void test_empty(void)
{
    reset();
    eq(t_get(0), 0, "02 get 0 on empty");
    eq(t_get(5), 0, "02 get 5 on empty");
    eq(t_max(0, MAXINDEX), 0, "02 max over everything on empty");
    t_unset();
    eq(t_get(0), 0, "02 get after unset on empty");
    eq(t_max(7, 7), 0, "02 max on empty");
}

/* 03  set 0 42 builds a tree with no inner nodes at all: the root is a leaf.
       Code that assumes the root has children breaks here. The last three
       lines store the value 0. That has to read back the same as an index
       nobody ever wrote to. */
static void test_height_zero(void)
{
    reset();
    t_set(0, 42);
    eq(t_get(0), 42, "03 get 0");
    eq(t_max(0, 0), 42, "03 max 0 0");
    eq(t_get(1), 0, "03 get 1 is outside the tree");
    eq(t_max(0, 1000), 42, "03 max over a clipped interval");
    t_set(0, 0);
    eq(t_get(0), 0, "03 get 0 after storing 0");
    eq(t_max(0, 0), 0, "03 max after storing 0");
}

/* 04  Overwriting with a smaller value has to lower the max, so line 106
       recomputes from both children. Folding the new value into the old max
       passes every other test in this file and fails this one. */
static void test_overwrite_smaller(void)
{
    reset();
    t_set(5, 100);
    t_set(5, 5);
    eq(t_get(5), 5, "04 get 5");
    eq(t_max(0, 7), 5, "04 max dropped to 5");
    t_set(5, 100);
    t_set(5, 0);
    eq(t_max(0, 7), 0, "04 max dropped to 0");
}

/* 05  The cached max is versioned along with the values. If any node were
       written to instead of copied, this would fail immediately. */
static void test_unset_restores_max(void)
{
    reset();
    t_set(5, 100);
    t_set(5, 5);
    eq(t_max(0, 7), 5, "05 max now");
    t_unset();
    eq(t_max(0, 7), 100, "05 max after one unset");
    t_unset();
    eq(t_max(0, 7), 0, "05 max after two unsets");
}

/* 06  The growth example from the lab text. Growing has to keep the values
       that were already there. The height is versioned alongside the root,
       and the trailing unset is what checks that. */
static void test_dynamic_height(void)
{
    reset();
    t_set(1, 17);
    eq(levels[version - 1], 1, "06 height 1 after set 1 17");
    t_set(3, 13);
    eq(levels[version - 1], 2, "06 height 2 after set 3 13");
    eq(t_get(1), 17, "06 old value survived growth");
    eq(t_get(3), 13, "06 new value stored");
    eq(t_max(0, 3), 17, "06 max over the whole tree");
    eq(t_max(2, 3), 13, "06 max over the right half");
    t_unset();
    eq(levels[version - 1], 1, "06 unset restored the height");
    eq(t_get(3), 0, "06 index 3 no longer fits");
    eq(t_max(0, 3), 17, "06 max after unset");
}

/* 07  Both guards in maxInInterval: clipping right down to the tree width,
       and rejecting an out of range left outright. Without the clip,
       maxSegment reads a bit that is not part of the tree. */
static void test_beyond_tree(void)
{
    reset();
    t_set(3, 9);
    eq(t_max(0, MAXINDEX), 9, "07 right end clipped to the tree");
    eq(t_max(4, 100), 0, "07 interval starts past the tree");
    eq(t_get(100), 0, "07 index past the tree");
    eq(t_max(100, 200), 0, "07 interval entirely past the tree");
}

/* 08  The largest index the spec allows. Builds a 31 level tree, which is
       where the shift arithmetic gets tested: 1u << 31 on line 187.
       The third query misses the only stored element by one. */
static void test_boundary_index(void)
{
    reset();
    t_set(MAXINDEX, 5);
    eq(levels[version - 1], 31, "08 height 31");
    eq(t_get(MAXINDEX), 5, "08 get at the largest index");
    eq(t_max(0, MAXINDEX), 5, "08 max over everything");
    eq(t_max(0, MAXINDEX - 1), 0, "08 interval stops one short");
    eq(t_max(MAXINDEX, MAXINDEX), 5, "08 single element interval");
}

/* 09  maxRight and maxLeft have to leave out the elements that fall outside
       the interval. Handing back a whole subtree max would be wrong here.
       Values sit at both ends, so the four queries catch one, the other,
       both, neither. */
static void test_gaps(void)
{
    reset();
    t_set(0, 50);
    t_set(7, 60);
    eq(t_max(1, 6), 0, "09 interval misses both");
    eq(t_max(1, 7), 60, "09 interval catches the right one");
    eq(t_max(0, 6), 50, "09 interval catches the left one");
    eq(t_max(0, 7), 60, "09 interval catches both");
}

/* 10  Covers all five cases of maxSegment on one tree.
       2..5 splits at the root (case 5); 1..2 goes left first (case 3);
       5..6 goes right first (case 4); 3..3 hits an empty subtree (case 1);
       and any walk down to a stored leaf is case 2. */
static void test_five_cases(void)
{
    reset();
    t_set(1, 10);
    t_set(2, 20);
    t_set(5, 30);
    t_set(6, 40);
    eq(t_max(2, 5), 30, "10 case 5, split at the root");
    eq(t_max(1, 2), 20, "10 case 3 then a split");
    eq(t_max(5, 6), 40, "10 case 4 then a split");
    eq(t_max(3, 3), 0, "10 case 1, nothing stored there");
    eq(t_max(0, 7), 40, "10 whole tree");
}

/* 11  The spec says unset on a fresh array does nothing. Two extra unsets
       must not underflow the version counter. The final set at the end
       checks the array still works afterwards. */
static void test_unset_underflow(void)
{
    reset();
    t_unset();
    t_unset();
    eq(t_get(0), 0, "11 get after unsets on empty");
    t_set(1, 3);
    t_unset();
    t_unset();
    eq(t_get(1), 0, "11 back to the empty array");
    t_set(1, 9);
    eq(t_get(1), 9, "11 still usable afterwards");
}

/* ================= what the output cannot show ================= */

/* A program that copied the whole array on every set would pass all eleven
   tests above. These look inside instead. A set touches one node per level
   and shares everything else, and the old versions stay intact. */
static void test_sharing(void)
{
    int r2, r3, before;
    int v2_inner, v3_inner;

    reset();
    t_set(3, 17);
    t_set(3, 4711);
    r2 = roots[version - 1];

    before = nodeCount;
    t_set(2, 20);
    r3 = roots[version - 1];

    eq(nodeCount - before, 3, "sharing: 3 nodes for a height 2 set");
    ok(r2 != r3, "sharing: the new version has its own root");

    /* one step down each version: root -> right child */
    v2_inner = pool[r2].child[1];
    v3_inner = pool[r3].child[1];

    /* that node was on the path to index 2, so it had to be copied */
    ok(v2_inner != v3_inner, "sharing: the node on the path was copied");

    /* but its right child, the leaf holding index 3, is one and the same */
    eq(pool[v3_inner].child[1], pool[v2_inner].child[1],
       "sharing: the leaf for index 3 is the same node in both versions");

    eq(get(r2, 2, 3), 4711, "sharing: version 2 keeps its value");
    eq(get(r2, 2, 2), 0, "sharing: version 2 never saw index 2");
}

/* Growing puts the old root on the left, because every index already stored
   gains a leading zero bit. This also shows the one node each growth wastes:
   grow builds a new top node, then set has to copy it instead of writing
   into it. */
static void test_growth_shape(void)
{
    reset();
    t_set(1, 17);
    eq(nodeCount - 1, 2, "growth: root and leaf");
    t_set(3, 13);
    eq(nodeCount - 1, 6, "growth: 3 from set, 1 wasted by grow");
    eq(pool[roots[version - 1]].child[0], 1, "growth: old root is child 0");
}

/* A full height set costs one node per level. That sets the pool budget. */
static void test_node_budget(void)
{
    int before;

    reset();
    before = nodeCount;
    t_set(MAXINDEX, 5);
    eq(nodeCount - before, 32, "budget: 32 nodes for a height 31 set");

    printf("  note: the pool holds %d nodes, about %d full height sets,\n",
           MAX_NODES, MAX_NODES / 32);
    printf("        but MAX_VERSIONS allows %d set operations.\n",
           MAX_VERSIONS - 1);
    printf("        Check the operation limit in the Kattis statement.\n");
}

int main(void)
{
    printf("steady-array tests\n\n");

    test_lab_sample();
    test_empty();
    test_height_zero();
    test_overwrite_smaller();
    test_unset_restores_max();
    test_dynamic_height();
    test_beyond_tree();
    test_boundary_index();
    test_gaps();
    test_five_cases();
    test_unset_underflow();

    test_sharing();
    test_growth_shape();
    test_node_budget();

    printf("\n%d checks, %d failures\n", checks, failures);
    if(failures == 0){
        printf("ALL PASSED\n");
    }
    return failures != 0;
}
