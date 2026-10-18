#include <iostream>
#include <vector>
#include <string>
#include "bdd.h"

const unsigned N_VAR = 144;
const unsigned N = 9;
const unsigned M = 4;
const unsigned LOG_N = 4;
const unsigned SIDE = 3; //сторона прямоугольника 3 х 3

enum Pos {
  LEFT,
  RIGHT
};

void init(bdd p[M][N][N]) {
  unsigned I = 0;
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      for (unsigned s = 0; s < M; s++) {
        p[s][i][j] = bddtrue;
        for (unsigned k = 0; k < LOG_N; k++) {
          p[s][i][j] &= ((j >> k) & 1) ? bdd_ithvar(I + s * LOG_N + k) : bdd_nithvar(I + s * LOG_N + k);
        }
      }
    }
    I += LOG_N * M;
  }
}

char var[N_VAR];
void print(void)
{
  for (unsigned i = 0; i < N; i++)
  {
    std::cout << i << ": ";
    for (unsigned j = 0; j < M; j++)
    {
      unsigned J = i * M * LOG_N + j * LOG_N;
      unsigned num = 0;
      for (unsigned k = 0; k < LOG_N; k++) num += (unsigned)(
        var[J + k] << k);
      std::cout << num << ' ';
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void build(char* varset, unsigned n, unsigned I)
{
  if (I == n - 1)
  {
    if (varset[I] >= 0)
    {
      var[I] = varset[I];
      print();
      return;
    }
    var[I] = 0;
    print();
    var[I] = 1;
    print();
    return;
  }
  if (varset[I] >= 0)
  {
    var[I] = varset[I];
    build(varset, n, I + 1);
    return;
  }
  var[I] = 0;
  build(varset, n, I + 1);
  var[I] = 1;
  build(varset, n, I + 1);
}

void fun(char* varset, int size) {
  build(varset, size, 0);
}

void cond_1(bdd& task, bdd p[M][N][N], unsigned k, unsigned i, unsigned j) {
  task &= p[k][i][j];
}

void cond_2(bdd& task, bdd p[M][N][N], unsigned k1, unsigned j1, unsigned k2, unsigned j2) {
  bdd cond_2 = bddtrue;
  for (unsigned i = 0; i < N; i++) {
    cond_2 &= !(p[k1][i][j1] ^ p[k2][i][j2]);
  }
  task &= cond_2;
}

void cond_3(bdd& task, bdd p[M][N][N], unsigned kCurrent, unsigned jCurrent, unsigned kNei, unsigned jNei, Pos pos, bool merging) {
  bdd cond_3 = bddtrue;
  switch (pos) {
  case LEFT:
    if (merging == false) {  //без склейки
      for (unsigned i = 1; i < N - SIDE; i++) {
        if (i % SIDE != 0) {
          cond_3 &= !(p[kCurrent][i][jCurrent] ^ p[kNei][i + SIDE - 1][jNei]);
        }
      }
    }
    else { //со склейкой
      for (unsigned i = 0; i < N - SIDE; i++) {
        if (i % SIDE == 0) {
          cond_3 &= !(p[kCurrent][i][jCurrent] ^ p[kNei][i + 2 * SIDE - 1][jNei]);
        }
        else {
          cond_3 &= !(p[kCurrent][i][jCurrent] ^ p[kNei][i + SIDE - 1][jNei]);
        }
      }
    }
    break;
  case RIGHT:
    if (merging == false) { //без склейки
      for (unsigned i = SIDE; i < N - 1; i++) {
        if (i % SIDE != SIDE - 1) {
          cond_3 &= !(p[kCurrent][i][jCurrent] ^ p[kNei][i - SIDE + 1][jNei]);
        }
      }
    }
    else { //со склейкой
      // У верхнего ряда нет соседа справа-сверху: цикл начинается с i = SIDE.
      // Раньше он начинался с 0, и индекс i - SIDE + 1 уходил за границы массива.
      for (unsigned i = SIDE; i < N; i++) {
        if (i % SIDE == SIDE - 1) {
          cond_3 &= !(p[kCurrent][i][jCurrent] ^ p[kNei][i - 2 * SIDE + 1][jNei]);
        }
        else {
          cond_3 &= !(p[kCurrent][i][jCurrent] ^ p[kNei][i - SIDE + 1][jNei]);
        }
      }
    }
    break;
  }
  task &= cond_3;
}

void cond_4(bdd& task, bdd p[M][N][N], unsigned kCurrent, unsigned jCurrent, unsigned kNei, unsigned jNei, bool merging) {
  bdd left = bddtrue;
  bdd right = bddtrue;
  cond_3(left, p, kCurrent, jCurrent, kNei, jNei, LEFT, merging);
  cond_3(right, p, kCurrent, jCurrent, kNei, jNei, RIGHT, merging);
  task &= (left | right);
}

void cond_5(bdd& task, const bdd p[M][N][N])
{
  for (unsigned m = 0; m < M; ++m) {
    for (unsigned i = 0; i < N - 1; ++i) {
      for (unsigned j = i + 1; j < N; ++j) {
        for (unsigned k = 0; k < N; ++k) {
          task &= p[m][i][k] >> !p[m][j][k];
        }
      }
    }
  }
}

void cond_6(bdd& task, bdd p[M][N][N])
{
  for (unsigned i = 0; i < N; ++i) {
    for (unsigned k = 0; k < M; ++k) {
      bdd temp = bddfalse;

      for (size_t j = 0; j < N; ++j) {
        temp |= p[k][i][j];
      }

      task &= temp;
    }
  }
}

int main() {

  bdd_init(100000, 10000);
  bdd_setvarnum(N_VAR);
  bdd p[M][N][N];

  init(p);

  bdd task = bddtrue;
 
  //прописываю ограничения фукнции
  //first_condition n1 = 7
  cond_1(task, p, 0, 3, 0); 
  cond_1(task, p, 0, 4, 5);
  cond_1(task, p, 0, 5, 8);
  cond_1(task, p, 0, 6, 7);
  cond_1(task, p, 0, 7, 3);
  cond_1(task, p, 0, 8, 1);
  cond_1(task, p, 1, 0, 0);
  cond_1(task, p, 1, 3, 6); 
  cond_1(task, p, 1, 5, 2); 
  cond_1(task, p, 1, 6, 5); 
  cond_1(task, p, 1, 7, 4); 
  cond_1(task, p, 1, 8, 8);

  std::cout << "after all 1 conditions: " << bdd_satcount(task) << " solutions\n";

  //second_condition n2 = 5
  cond_2(task, p, 0, 2, 2, 1); 
  cond_2(task, p, 0, 5, 2, 4);
  cond_2(task, p, 0, 7, 2, 8);
  cond_2(task, p, 0, 3, 2, 2);
  cond_2(task, p, 0, 1, 2, 0);
  cond_2(task, p, 1, 3, 3, 5); 
  cond_2(task, p, 1, 6, 3, 4);
  cond_2(task, p, 1, 2, 3, 6);
  cond_2(task, p, 1, 5, 3, 7);
  cond_2(task, p, 1, 4, 3, 1);

  std::cout << "after all 2 conditions: " << bdd_satcount(task) << " solutions\n";

  //third_condition n3 = 5
  cond_3(task, p, 0, 2, 0, 0, LEFT, false);
  cond_3(task, p, 2, 6, 2, 4, LEFT, false);
  cond_3(task, p, 1, 6, 1, 3, RIGHT, false);
  cond_3(task, p, 3, 7, 3, 0, RIGHT, false); 
  cond_3(task, p, 3, 0, 3, 3, RIGHT, false);
  
  std::cout << "Do you want merging?\n";
  std::string answer;
  std::cin >> answer;
  if (answer == "yes") {
    cond_3(task, p, 2, 5, 2, 7, RIGHT, true);
  }
  
  std::cout << "after all 3 conditions: " << bdd_satcount(task) << " solutions\n";
  //fourth_condition n4 = 4
  cond_4(task, p, 0, 5, 0, 6, false);
  cond_4(task, p, 1, 1, 1, 5, false);
  cond_4(task, p, 2, 5, 2, 2, false);
  cond_4(task, p, 3, 1, 3, 6, false);

  std::cout << "after all 4 conditions: " << bdd_satcount(task) << " solutions\n";
  //fifth_condition
  cond_5(task, p);

  std::cout << "after all 5 conditions: " << bdd_satcount(task) << " solutions\n";
  //sixth_condition
  cond_6(task, p);
  
  std::cout << "after all 6 conditions: " << bdd_satcount(task) << " solutions\n";
  std::cout << "RESULT: " << bdd_satcount(task) << " solutions\n";

  unsigned satcount = (unsigned)bdd_satcount(task);
  std::cout << satcount << " solutions:\n" << std::endl;
  if (satcount) bdd_allsat(task, fun);
  bdd_done();
  return 0;
}