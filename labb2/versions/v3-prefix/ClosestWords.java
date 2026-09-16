/* Labb 2 i DD2350 Algoritmer, datastrukturer och komplexitet    */
/* Se labbinstruktionerna i kursrummet i Canvas                  */
/* Ursprunglig författare: Viggo Kann KTH viggo@kth.se           */
import java.util.LinkedList;
import java.util.List;

public class ClosestWords {
  LinkedList<String> closestWords = null;

  int closestDistance = -1;

  // M[i][j] är avståndet mellan ordlistordets första i bokstäver och det
  // felstavade ordets första j.
  int[][] M = new int[1][1];
  char[] w1chars;

  // Rad i beror bara på ordlistordets första i bokstäver. Delar ordet sina
  // första p bokstäver med prevWord kan vi börja på rad p+1.
  String prevWord = "";

  void ensureRows(int rows) {
    if (rows <= M.length)
      return;
    int cols = w1chars.length + 1;
    int[][] bigger = new int[rows][cols];
    for (int j = 0; j < cols; j++)
      bigger[0][j] = j;
    for (int i = 0; i < rows; i++)
      bigger[i][0] = i;
    M = bigger;
    prevWord = ""; // bara rad 0 är ifylld i nya matrisen
  }

  int partDist(String w2, int w2len) {
    ensureRows(w2len + 1);
    int p = 0;
    int maxP = Math.min(w2len, prevWord.length());
    while (p < maxP && w2.charAt(p) == prevWord.charAt(p))
      p++;
    char[] a = w1chars;
    int w1len = a.length;
    for (int i = p + 1; i <= w2len; i++) {
      int[] prev = M[i - 1];
      int[] cur = M[i];
      char c = w2.charAt(i - 1);
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
    prevWord = w2;
    return M[w2len][w1len];
  }

  public ClosestWords(String w, List<String> wordList) {
    w1chars = w.toCharArray();
    M = new int[1][w1chars.length + 1];
    for (int j = 0; j <= w1chars.length; j++)
      M[0][j] = j;
    for (String s : wordList) {
      int dist = partDist(s, s.length());
      if (dist < closestDistance || closestDistance == -1) {
        closestDistance = dist;
        closestWords = new LinkedList<String>();
        closestWords.add(s);
      }
      else if (dist == closestDistance)
        closestWords.add(s);
    }
  }

  int getMinDistance() {
    return closestDistance;
  }

  List<String> getClosestWords() {
    return closestWords;
  }
}
