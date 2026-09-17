# Lab 2: diffs between versions

Each section is the diff from one version to the next, with notes on what to look for. Only files that changed are shown: `ClosestWords.java` in every step, and `Main.java` only in v3 → v4. Lines starting with `-` were removed, lines starting with `+` were added. Times are from the Kattis-sized stress test in `RESULTS.md`.

## v0-original → v1-dynprog

Time: 23.3 s on the stress test (v0 would take ~45 h on large4)

Only `partDist` changes. The three recursive calls are replaced by lookups in the matrix `M`:

- `partDist(w1, w2, w1len - 1, w2len - 1)` becomes `M[i - 1][j - 1]`
- `partDist(w1, w2, w1len - 1, w2len)` becomes `M[i - 1][j]`
- `partDist(w1, w2, w1len, w2len - 1)` becomes `M[i][j - 1]`

The base cases `if (w1len == 0) return w2len` and `if (w2len == 0) return w1len` become row 0 (`M[0][j] = j`) and column 0 (`M[i][0] = i`). The outer loop runs over rows `i` and the inner over columns `j`, so the matrix is filled row by row. Nothing else in the file changes, and `Main.java` is identical.

```diff
--- a/v0-original/ClosestWords.java
+++ b/v1-dynprog/ClosestWords.java
@@ -10,18 +10,24 @@ public class ClosestWords {
   int closestDistance = -1;
 
+  // fyller rad för rad, bättre minneslokalitet
   int partDist(String w1, String w2, int w1len, int w2len) {
-    if (w1len == 0)
-      return w2len;
-    if (w2len == 0)
-      return w1len;
-    int res = partDist(w1, w2, w1len - 1, w2len - 1) + 
-	(w1.charAt(w1len - 1) == w2.charAt(w2len - 1) ? 0 : 1);
-    int addLetter = partDist(w1, w2, w1len - 1, w2len) + 1;
-    if (addLetter < res)
-      res = addLetter;
-    int deleteLetter = partDist(w1, w2, w1len, w2len - 1) + 1;
-    if (deleteLetter < res)
-      res = deleteLetter;
-    return res;
+    int[][] M = new int[w2len + 1][w1len + 1];
+    for (int j = 0; j <= w1len; j++)
+      M[0][j] = j;
+    for (int i = 1; i <= w2len; i++) {
+      M[i][0] = i;
+      char c = w2.charAt(i - 1);
+      for (int j = 1; j <= w1len; j++) {
+        int res = M[i - 1][j - 1] + (w1.charAt(j - 1) == c ? 0 : 1);
+        int addLetter = M[i - 1][j] + 1;
+        if (addLetter < res)
+          res = addLetter;
+        int deleteLetter = M[i][j - 1] + 1;
+        if (deleteLetter < res)
+          res = deleteLetter;
+        M[i][j] = res;
+      }
+    }
+    return M[w2len][w1len];
   }
 
```

## v1-dynprog → v2-reuse-matrix

Time: 23.3 s → 7.6 s

Look for three things:

1. `M` moves from a local variable inside `partDist` to a field, and is created in the constructor. With 500,000 words and 100 queries, v1 ran `new int[w2len + 1][w1len + 1]` 50 million times; v2 runs it about once per query, plus whenever a longer dictionary word needs more rows (`ensureRows`).
2. `w1chars = w.toCharArray()` is done once per query, and the inner loop reads `a[j - 1]` instead of `w1.charAt(j - 1)`.
3. `int[] prev = M[i - 1]` and `int[] cur = M[i]` are looked up once per row, so the inner loop reads `prev[j]` instead of `M[i - 1][j]`.

`partDist` now takes only the dictionary word, since the misspelled word is stored in `w1chars`. The `distance` helper is gone.

```diff
--- a/v1-dynprog/ClosestWords.java
+++ b/v2-reuse-matrix/ClosestWords.java
@@ -10,21 +10,37 @@ public class ClosestWords {
   int closestDistance = -1;
 
-  // fyller rad för rad, bättre minneslokalitet
-  int partDist(String w1, String w2, int w1len, int w2len) {
-    int[][] M = new int[w2len + 1][w1len + 1];
-    for (int j = 0; j <= w1len; j++)
-      M[0][j] = j;
+  // återanvänds för alla ord i listan
+  int[][] M = new int[1][1];
+  char[] w1chars;
+
+  void ensureRows(int rows) {
+    if (rows <= M.length)
+      return;
+    int cols = w1chars.length + 1;
+    int[][] bigger = new int[rows][cols];
+    for (int j = 0; j < cols; j++)
+      bigger[0][j] = j;
+    for (int i = 0; i < rows; i++)
+      bigger[i][0] = i;
+    M = bigger;
+  }
+
+  int partDist(String w2, int w2len) {
+    ensureRows(w2len + 1);
+    char[] a = w1chars;
+    int w1len = a.length;
     for (int i = 1; i <= w2len; i++) {
-      M[i][0] = i;
+      int[] prev = M[i - 1];
+      int[] cur = M[i];
       char c = w2.charAt(i - 1);
       for (int j = 1; j <= w1len; j++) {
-        int res = M[i - 1][j - 1] + (w1.charAt(j - 1) == c ? 0 : 1);
-        int addLetter = M[i - 1][j] + 1;
+        int res = prev[j - 1] + (a[j - 1] == c ? 0 : 1);
+        int addLetter = prev[j] + 1;
         if (addLetter < res)
           res = addLetter;
-        int deleteLetter = M[i][j - 1] + 1;
+        int deleteLetter = cur[j - 1] + 1;
         if (deleteLetter < res)
           res = deleteLetter;
-        M[i][j] = res;
+        cur[j] = res;
       }
     }
@@ -32,12 +48,11 @@ public class ClosestWords {
   }
 
-  int distance(String w1, String w2) {
-    return partDist(w1, w2, w1.length(), w2.length());
-  }
-
   public ClosestWords(String w, List<String> wordList) {
+    w1chars = w.toCharArray();
+    M = new int[1][w1chars.length + 1];
+    for (int j = 0; j <= w1chars.length; j++)
+      M[0][j] = j;
     for (String s : wordList) {
-      int dist = distance(w, s);
-      // System.out.println("d(" + w + "," + s + ")=" + dist);
+      int dist = partDist(s, s.length());
       if (dist < closestDistance || closestDistance == -1) {
         closestDistance = dist;
```

## v2-reuse-matrix → v3-prefix

Time: 7.6 s → 5.2 s

The smallest step. The new field `prevWord` remembers the last dictionary word. At the start of `partDist`, a `while` loop counts how many letters (`p`) the new word shares with `prevWord`, and the row loop starts at `i = p + 1` instead of `i = 1`.

Example: after `abbedissa` comes `abbedissan`. `p` = 9, so only row 10 is computed.

`prevWord = ""` in `ensureRows` handles the case where the matrix was just replaced by a bigger one: the old rows are gone, so nothing can be reused.

```diff
--- a/v2-reuse-matrix/ClosestWords.java
+++ b/v3-prefix/ClosestWords.java
@@ -10,8 +10,10 @@ public class ClosestWords {
   int closestDistance = -1;
 
-  // återanvänds för alla ord i listan
   int[][] M = new int[1][1];
   char[] w1chars;
 
+  // raderna för gemensamt prefix med förra ordet behöver inte räknas om
+  String prevWord = "";
+
   void ensureRows(int rows) {
     if (rows <= M.length)
@@ -24,11 +26,16 @@ public class ClosestWords {
       bigger[i][0] = i;
     M = bigger;
+    prevWord = "";
   }
 
   int partDist(String w2, int w2len) {
     ensureRows(w2len + 1);
+    int p = 0;
+    int maxP = Math.min(w2len, prevWord.length());
+    while (p < maxP && w2.charAt(p) == prevWord.charAt(p))
+      p++;
     char[] a = w1chars;
     int w1len = a.length;
-    for (int i = 1; i <= w2len; i++) {
+    for (int i = p + 1; i <= w2len; i++) {
       int[] prev = M[i - 1];
       int[] cur = M[i];
@@ -45,4 +52,5 @@ public class ClosestWords {
       }
     }
+    prevWord = w2;
     return M[w2len][w1len];
   }
```

## v3-prefix → v4-precomputed-prefix

Time: 5.2 s → 3.0 s

The diff is big, but most of it is code moving. Look for these parts:

1. **New class `WordList`.** Its constructor runs once per input. It does the same prefix-counting `while` loop as v3, but stores the result in `prefix[k]` for every word instead of redoing it for each misspelled word. It also stores each word as a `char[]` (`chars[k]`) and records `maxLength`.
2. **The constructor now takes a `WordList`.** The matrix is created at full height (`dict.maxLength + 1`), so `ensureRows` and `prevWord` are no longer needed. Column 0 is filled for all rows up front.
3. **The DP loop moved into the constructor.** Its body is unchanged from v3. The only differences are `for (int i = dict.prefix[k] + 1; ...)` and `char c = b[i - 1]` instead of `w2.charAt(i - 1)`.
4. **`Main.java` changes in two places,** the only time it changes. `readWordList(...)` is wrapped in `new ClosestWords.WordList(...)`, both for normal input and in `-t` test mode.

```diff
--- a/v3-prefix/ClosestWords.java
+++ b/v4-precomputed-prefix/ClosestWords.java
@@ -6,68 +6,74 @@ import java.util.List;
 
 public class ClosestWords {
-  LinkedList<String> closestWords = null;
-
-  int closestDistance = -1;
-
-  int[][] M = new int[1][1];
-  char[] w1chars;
+  // räknas ut en gång per ordlista
+  static class WordList {
+    final String[] words;
+    final char[][] chars;
+    // prefix[k] = gemensamt prefix med ord k-1
+    final int[] prefix;
+    final int maxLength;
 
-  // raderna för gemensamt prefix med förra ordet behöver inte räknas om
-  String prevWord = "";
-
-  void ensureRows(int rows) {
-    if (rows <= M.length)
-      return;
-    int cols = w1chars.length + 1;
-    int[][] bigger = new int[rows][cols];
-    for (int j = 0; j < cols; j++)
-      bigger[0][j] = j;
-    for (int i = 0; i < rows; i++)
-      bigger[i][0] = i;
-    M = bigger;
-    prevWord = "";
-  }
-
-  int partDist(String w2, int w2len) {
-    ensureRows(w2len + 1);
-    int p = 0;
-    int maxP = Math.min(w2len, prevWord.length());
-    while (p < maxP && w2.charAt(p) == prevWord.charAt(p))
-      p++;
-    char[] a = w1chars;
-    int w1len = a.length;
-    for (int i = p + 1; i <= w2len; i++) {
-      int[] prev = M[i - 1];
-      int[] cur = M[i];
-      char c = w2.charAt(i - 1);
-      for (int j = 1; j <= w1len; j++) {
-        int res = prev[j - 1] + (a[j - 1] == c ? 0 : 1);
-        int addLetter = prev[j] + 1;
-        if (addLetter < res)
-          res = addLetter;
-        int deleteLetter = cur[j - 1] + 1;
-        if (deleteLetter < res)
-          res = deleteLetter;
-        cur[j] = res;
+    WordList(List<String> list) {
+      int n = list.size();
+      words = list.toArray(new String[n]);
+      chars = new char[n][];
+      prefix = new int[n];
+      int max = 0;
+      char[] prev = new char[0];
+      for (int k = 0; k < n; k++) {
+        char[] cur = words[k].toCharArray();
+        int p = 0;
+        int maxP = Math.min(cur.length, prev.length);
+        while (p < maxP && cur[p] == prev[p])
+          p++;
+        chars[k] = cur;
+        prefix[k] = p;
+        if (cur.length > max)
+          max = cur.length;
+        prev = cur;
       }
+      maxLength = max;
     }
-    prevWord = w2;
-    return M[w2len][w1len];
   }
 
-  public ClosestWords(String w, List<String> wordList) {
-    w1chars = w.toCharArray();
-    M = new int[1][w1chars.length + 1];
-    for (int j = 0; j <= w1chars.length; j++)
+  LinkedList<String> closestWords = null;
+
+  int closestDistance = -1;
+
+  public ClosestWords(String w, WordList dict) {
+    char[] a = w.toCharArray();
+    int w1len = a.length;
+    int[][] M = new int[dict.maxLength + 1][w1len + 1];
+    for (int j = 0; j <= w1len; j++)
       M[0][j] = j;
-    for (String s : wordList) {
-      int dist = partDist(s, s.length());
+    for (int i = 0; i <= dict.maxLength; i++)
+      M[i][0] = i;
+
+    for (int k = 0; k < dict.chars.length; k++) {
+      char[] b = dict.chars[k];
+      int w2len = b.length;
+      for (int i = dict.prefix[k] + 1; i <= w2len; i++) {
+        int[] prev = M[i - 1];
+        int[] cur = M[i];
+        char c = b[i - 1];
+        for (int j = 1; j <= w1len; j++) {
+          int res = prev[j - 1] + (a[j - 1] == c ? 0 : 1);
+          int addLetter = prev[j] + 1;
+          if (addLetter < res)
+            res = addLetter;
+          int deleteLetter = cur[j - 1] + 1;
+          if (deleteLetter < res)
+            res = deleteLetter;
+          cur[j] = res;
+        }
+      }
+      int dist = M[w2len][w1len];
       if (dist < closestDistance || closestDistance == -1) {
         closestDistance = dist;
         closestWords = new LinkedList<String>();
-        closestWords.add(s);
+        closestWords.add(dict.words[k]);
       }
       else if (dist == closestDistance)
-        closestWords.add(s);
+        closestWords.add(dict.words[k]);
     }
   }
--- a/v3-prefix/Main.java
+++ b/v4-precomputed-prefix/Main.java
@@ -37,5 +37,5 @@ public class Main {
     // Säkrast att specificera att UTF-8 ska användas, för vissa system har annan
     // standardinställning för teckenkodningen.
-    List<String> wordList = readWordList(stdin);
+    ClosestWords.WordList wordList = new ClosestWords.WordList(readWordList(stdin));
     String word;
     while ((word = stdin.readLine()) != null) {
@@ -118,7 +118,7 @@ public class Main {
       // Säkrast att specificera att UTF-8 ska användas, för vissa system har annan
     // standardinställning för teckenkodningen.
-    List<String> wordList = null;
+    ClosestWords.WordList wordList = null;
     try {
-      wordList = readWordList(inFile);
+      wordList = new ClosestWords.WordList(readWordList(inFile));
     } catch (Exception e) {
       System.out.println("Could not read the wordList of this testcase.");
```

## v4-precomputed-prefix → v5-pruning

Time: 3.0 s → 1.39 s

Four additions to the loop over dictionary words:

1. **`validRows`.** v4 could trust `prefix[k]` because it always computed every row of the previous word. v5 sometimes skips a word or stops partway, so rows further down may belong to an older word. `validRows` is the number of rows that are still correct, and the loop starts at `validRows + 1`. `if (dict.prefix[k] < validRows) validRows = dict.prefix[k]` keeps it within the prefix shared with the current word.
2. **Length skip.** `lengthDiff > closestDistance` means `continue` before any DP. Example: best distance 2, misspelled word 5 letters, dictionary word 9 letters, so 4 > 2 and the word is skipped.
3. **`rowMin`.** It tracks the smallest value in the current row. If it exceeds `closestDistance` after the row, the word can't reach the best distance, so `aborted = true; break;`, then `continue`.
4. Both tests use `>`, not `>=`, so words at exactly the best distance are still printed.

```diff
--- a/v4-precomputed-prefix/ClosestWords.java
+++ b/v5-pruning/ClosestWords.java
@@ -50,11 +50,23 @@ public class ClosestWords {
       M[i][0] = i;
 
+    // rader som fortfarande stämmer (vi hoppar ju över/avbryter ibland)
+    int validRows = 0;
     for (int k = 0; k < dict.chars.length; k++) {
       char[] b = dict.chars[k];
       int w2len = b.length;
-      for (int i = dict.prefix[k] + 1; i <= w2len; i++) {
+      if (dict.prefix[k] < validRows)
+        validRows = dict.prefix[k];
+
+      // avståndet är minst längdskillnaden
+      int lengthDiff = w2len > w1len ? w2len - w1len : w1len - w2len;
+      if (closestDistance != -1 && lengthDiff > closestDistance)
+        continue;
+
+      boolean aborted = false;
+      for (int i = validRows + 1; i <= w2len; i++) {
         int[] prev = M[i - 1];
         int[] cur = M[i];
         char c = b[i - 1];
+        int rowMin = cur[0];
         for (int j = 1; j <= w1len; j++) {
           int res = prev[j - 1] + (a[j - 1] == c ? 0 : 1);
@@ -66,6 +78,17 @@ public class ClosestWords {
             res = deleteLetter;
           cur[j] = res;
+          if (res < rowMin)
+            rowMin = res;
+        }
+        validRows = i;
+        // hela raden redan sämre, ge upp
+        if (closestDistance != -1 && rowMin > closestDistance) {
+          aborted = true;
+          break;
         }
       }
+      if (aborted)
+        continue;
+
       int dist = M[w2len][w1len];
       if (dist < closestDistance || closestDistance == -1) {
```
