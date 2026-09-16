/* Labb 2 i DD2350 Algoritmer, datastrukturer och komplexitet    */
/* Se labbinstruktionerna i kursrummet i Canvas                  */
/* Ursprunglig författare: Viggo Kann KTH viggo@kth.se           */
import java.util.LinkedList;
import java.util.List;

public class ClosestWords {
  // v4: allt som bara beror på ordlistan räknas ut en gång, inte en gång
  // per felstavat ord.
  static class WordList {
    final String[] words;
    final char[][] chars;
    // prefix[k] = antal inledande bokstäver som ord k har gemensamt med ord k-1
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
    // M[i][j] = editeringsavståndet mellan de första i bokstäverna i
    // ordlistordet och de första j bokstäverna i det felstavade ordet.
    // Matrisen får plats med det längsta ordet och skapas en gång per fråga.
    int[][] M = new int[dict.maxLength + 1][w1len + 1];
    for (int j = 0; j <= w1len; j++)
      M[0][j] = j;
    for (int i = 0; i <= dict.maxLength; i++)
      M[i][0] = i;

    for (int k = 0; k < dict.chars.length; k++) {
      char[] b = dict.chars[k];
      int w2len = b.length;
      // Raderna 0..prefix[k] är kvar från förra ordet (teorifråga 9-10).
      for (int i = dict.prefix[k] + 1; i <= w2len; i++) {
        int[] prev = M[i - 1];
        int[] cur = M[i];
        char c = b[i - 1];
        for (int j = 1; j <= w1len; j++) {
          int res = prev[j - 1] + (a[j - 1] == c ? 0 : 1);
          int addLetter = prev[j] + 1;
          if (addLetter < res)
            res = addLetter;
          int deleteLetter = cur[j - 1] + 1;
          if (deleteLetter < res)
            res = deleteLetter;
          cur[j] = res;
        }
      }
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
