// "Include-Guard": Bei mehrfachem Einbinden der Header-Datei
// wir der Code nur einmal eingefuegt
#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>

/** Eindimensionales Array als Klassen-Template. */
template<class T>
class Array1
{
public:

  // Konstruktor: Erzeugt leeres Array 
  Array1() : data_(nullptr), n_(0) {}

  // Konstruktor: Allokiert Array auf Groesse n
  Array1(int n) : data_(nullptr), n_(0) { allocate(n); }

  // Destruktor: Wird beim Loeschen eines Objekts aufgerufen.
  // Wenn noch nicht explizit de-allokiert wurde, so geschieht das jetzt.
  ~Array1() { free(); }
  
  // Referenz auf ein Element
  // Die Indizes laufen hier von 0 .. n_
  T& element(int i) { return data_[ i ]; }

  // Diese Funktion ist const-qualifiert: Sie kann auch auf konstante
  // array-Objekte angewendet werden. 
  T element(int i) const { return data_[ i ]; }

  // Referenz auf ein Element, in ()-Operator-Klammerschreibweise.
  // Deligiert an element(...)
  T& operator()(int i) { return element(i); }

  // Gleichfalls, als const-Operator.
  // Dieser deligiert an element(...)const
  T operator()(int i) const { return element(i); }

  // Zugang zum Pointer auf Anfang des Arrays
  T* data() { return data_; }
  
  // Fuelle Array mit Wert z.
  // Diese Funktion ist in Datei array.cc implementiert.
  void fill(int z) { for(int i=0; i < n_; ++i){ data_[i] = z; } }

  // Allokiere Speicher fuer n Elemente
  void allocate(const int n){
    assert( n >=0 && ( n < 10000000 ) );
    free();
    data_ = new T[n];
    n_ = n;
  }

  // De-Allokation (Speicherfreigabe)
  void free()
  {
    delete[] data_;
    data_ =  nullptr;
    n_ = 0;
  }

  // Allocated size of this array
  int size() const { return n_; }
  
private:
  // Ein Pointer auf die Array-Daten. Wenn das Array nicht allokiert ist,
  // ist er gleich nullptr.
  T* data_;

  // Groesse des Arrays
  int n_;

  // Kopieren verboten: Der "Copy-Konstruktor" soll nicht bereitgestellt werden.
  Array1(const Array1&) = delete;
};


class IntArray2
{
public:

  // Konstruktor: Erzeugt leeres Array 
  IntArray2() : data_(nullptr), n1_(0), n2_(0) {}

  // Konstruktor: Allokiert Array auf Groesse m x n
  IntArray2(int m, int n) : data_(nullptr), n1_(0), n2_(0)
  { allocate(m, n); }

  // Destruktor: Wird beim Loeschen eines Objekts aufgerufen.
  // Wenn noch nicht explizit de-allokiert wurde, so geschieht das jetzt.
  ~IntArray2() { free(); }
  
  // Referenz auf ein Element
  // Die Indizes laufen hier von 0 .. n_
  int& element(int i1, int i2) { return data_[ i1 + i2*n1_ ]; }

  // Diese Funktion ist const-qualifiert: Sie kann auch auf konstante
  // array-Objekte angewendet werden. 
  int element(int i1, int i2) const { return data_[ i1 + i2*n1_ ]; }

  // Referenz auf ein Element, in ()-Operator-Klammerschreibweise.
  // Deligiert an element(...)
  int& operator()(int i1, int i2) { return element(i1, i2); }

  // Gleichfalls, als const-Operator.
  // Dieser deligiert an element(...)const
  int operator()(int i1, int i2) const { return element(i1, i2); }

  // Fuelle Array mit Wert z.
  // Diese Funktion ist in Datei array.cc implementiert.
  void fill(int z);

  // Allokiere Speicher fuer m-mal-n Elemente
  void allocate(const int m, const int n);

  // De-Allokation (Speicherfreigabe)
  void free();
  
private:
  // private-Daten und Funktionen sind nur aus anderen Array-Funktionen
  // heraus aufrufbar. Dadurch werden sie gegen inkonsistente Modifikation
  // geschuetzt.

  // Ein Pointer auf die Array-Daten. Wenn das Array nicht allokiert ist,
  // ist er gleich nullptr.
  int* data_;

  // Groesse des Arrays in erster Dimension 
  int n1_;

  // ... bzw. in zweiter Dimension.
  int n2_;

  // Kopieren verboten: Der "Copy-Konstruktor" soll nicht bereitgestellt werden.
  IntArray2(const IntArray2&) = delete;
};

#endif
