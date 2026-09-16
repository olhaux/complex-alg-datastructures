/* Labb 2 i DD2350 Algoritmer, datastrukturer och komplexitet    */
/* Se labbinstruktionerna i kursrummet i Canvas                  */
/* Ursprunglig författare: Viggo Kann KTH viggo@kth.se           */
import java.util.LinkedList;
import java.util.List;

public class ClosestWords {
  LinkedList<String> closestWords = null;

  int closestDistance = -1;

  // v1: partDist med dynamisk programmering istället för rekursion.
  // M[i][j] = editeringsavståndet mellan de första i bokstäverna i w2
  // och de första j bokstäverna i w1. Matrisen fylls rad för rad, och inom
  // en rad kolumn för kolumn, eftersom Java lagrar varje rad som en
  // sammanhängande array. Då ligger M[i-1][j-1], M[i-1][j] och M[i][j-1]
  // nära den cell som skrivs.
  int partDist(String w1, String w2, int w1len, int w2len) {
    int[][] M = new int[w2len + 1][w1len + 1];
    for (int j = 0; j <= w1len; j++)
      M[0][j] = j;
    for (int i = 1; i <= w2len; i++) {
      M[i][0] = i;
      char c = w2.charAt(i - 1);
      for (int j = 1; j <= w1len; j++) {
        int res = M[i - 1][j - 1] + (w1.charAt(j - 1) == c ? 0 : 1);
        int addLetter = M[i - 1][j] + 1;
        if (addLetter < res)
          res = addLetter;
        int deleteLetter = M[i][j - 1] + 1;
        if (deleteLetter < res)
          res = deleteLetter;
        M[i][j] = res;
      }
    }
    return M[w2len][w1len];
  }

  int distance(String w1, String w2) {
    return partDist(w1, w2, w1.length(), w2.length());
  }

  public ClosestWords(String w, List<String> wordList) {
    for (String s : wordList) {
      int dist = distance(w, s);
      // System.out.println("d(" + w + "," + s + ")=" + dist);
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
