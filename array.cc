#include <cstdio>
#include <cstdlib>
#include "array.h"

// Testet, ob i im Bereich min..max liegt.
// Wenn nicht, schreibe Fehlermeldung und liefere "false",
// sonst liefere "true".
// static Qualifikation: Funktion ist nur in dieser Datei "sichtbar"
static bool check(int i, int min, int max)
{
  if( (i < min) || (i > max) ){
    printf("checkInterval: Wert %d liegt ausserhalb von (%d, %d)\n",
           i, min, max);
    return false;
  }
  return true;
}

// Implementation der Funktion "fill" aus Klass "IntArray2"
void IntArray2::fill(int z)
{
  for(int i=0; i < n1_*n2_; ++i){
    data_[i] = z;
  }
}
  
void IntArray2::allocate(const int n, const int m)
{
  free();
  const int maxGroesse = 100000;
  if( !check(n, 0, maxGroesse) || !check(m, 0, maxGroesse) ){
    // Wenn etwas mit der Groesse nicht stimmt, brechen wir das
    // Programm ab. Einfach aber brutal.
    exit(1);
  }
  data_ = new int[n*m];
  n1_ = n;
  n2_ = m;
}

void IntArray2::free()
{
  delete[] data_;
  data_ =  nullptr;
  n1_ = 0;
  n2_ = 0;
}
