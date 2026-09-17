/* Labb 2 i DD2350 Algoritmer, datastrukturer och komplexitet    */
/* Se labbinstruktionerna i kursrummet i Canvas                  */
/* Ursprunglig författare: Viggo Kann KTH viggo@kth.se           */
import java.util.LinkedList;
import java.util.List;

public class ClosestWords {
  // räknas ut en gång per ordlista
  static class WordList {
    final String[] words;
    final char[][] chars;
    // prefix[k] = gemensamt prefix med ord k-1
    final int[] prefix;
    final int maxLength;

    WordList(List<String> list) {
      int n = list.size();
      words = list.toArray(new String[n]);
      chars = new char[n][];
      prefix = new int[n];
      int max = 0;
      char[] prev = new char[0];
      for (int k = 0; k < n; k++) {
        char[] cur = words[k].toCharArray();
        int p = 0;
        int maxP = Math.min(cur.length, prev.length);
        while (p < maxP && cur[p] == prev[p])
          p++;
        chars[k] = cur;
        prefix[k] = p;
        if (cur.length > max)
          max = cur.length;
        prev = cur;
      }
      maxLength = max;
    }
  }

  LinkedList<String> closestWords = null;

  int closestDistance = -1;

  public ClosestWords(String w, WordList dict) {
    char[] a = w.toCharArray();
    int w1len = a.length;
    int[][] M = new int[dict.maxLength + 1][w1len + 1];
    for (int j = 0; j <= w1len; j++)
      M[0][j] = j;
    for (int i = 0; i <= dict.maxLength; i++)
      M[i][0] = i;

    // rader som fortfarande stämmer (vi hoppar ju över/avbryter ibland)
    int validRows = 0;
    for (int k = 0; k < dict.chars.length; k++) {
      char[] b = dict.chars[k];
      int w2len = b.length;
      if (dict.prefix[k] < validRows)
        validRows = dict.prefix[k];

      // avståndet är minst längdskillnaden
      int lengthDiff = w2len > w1len ? w2len - w1len : w1len - w2len;
      if (closestDistance != -1 && lengthDiff > closestDistance)
        continue;

      boolean aborted = false;
      for (int i = validRows + 1; i <= w2len; i++) {
        int[] prev = M[i - 1];
        int[] cur = M[i];
        char c = b[i - 1];
        int rowMin = cur[0];
        for (int j = 1; j <= w1len; j++) {
          int res = prev[j - 1] + (a[j - 1] == c ? 0 : 1);
          int addLetter = prev[j] + 1;
          if (addLetter < res)
            res = addLetter;
          int deleteLetter = cur[j - 1] + 1;
          if (deleteLetter < res)
            res = deleteLetter;
          cur[j] = res;
          if (res < rowMin)
            rowMin = res;
        }
        validRows = i;
        // hela raden redan sämre, ge upp
        if (closestDistance != -1 && rowMin > closestDistance) {
          aborted = true;
          break;
        }
      }
      if (aborted)
        continue;

      int dist = M[w2len][w1len];
      if (dist < closestDistance || closestDistance == -1) {
        closestDistance = dist;
        closestWords = new LinkedList<String>();
        closestWords.add(dict.words[k]);
      }
      else if (dist == closestDistance)
        closestWords.add(dict.words[k]);
    }
  }

  int getMinDistance() {
    return closestDistance;
  }

  List<String> getClosestWords() {
    return closestWords;
  }
}
