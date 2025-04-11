#ifdef NDEBUG
#undef NDEBUG
#endif

#include "ma.h"
#include "memory_tests.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


/** MAKRA SKRACAJĄCE IMPLEMENTACJĘ TESTÓW **/

// To są możliwe wyniki testu.
#define PASS 0
#define FAIL 1
#define WRONG_TEST 2

// Oblicza liczbę elementów tablicy x.
#define SIZE(x) (sizeof x / sizeof x[0])

#define ASSERT(f)                        \
  do {                                   \
    if (!(f))                            \
      return FAIL;                       \
  } while (0)

#define V(code, where) (((unsigned long)code) << (3 * where))

/** WŁAŚCIWE TESTY **/

void t_one(uint64_t *next_state, uint64_t const *input,
           uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] + input[0];
}

void y_one(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] + 1;
}

static void t_two(uint64_t *next_state, uint64_t const *input,
                  uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] ^ input[0];
}

// Testuje jeden automat wykonujący jakieś dodawania.
static int one(void) {
  const uint64_t q1 = 1, x3 = 3, *y;

  moore_t *a = ma_create_full(64, 64, 64, t_one, y_one, &q1);
  assert(a);

  y = ma_get_output(a);
  ASSERT(y != NULL);
  ASSERT(ma_set_input(a, &x3) == 0);
  ASSERT(y[0] == 2);
  ASSERT(ma_step(&a, 1) == 0);
  ASSERT(y[0] == 5);
  ASSERT(ma_step(&a, 1) == 0);
  ASSERT(y[0] == 8);
  ASSERT(ma_set_input(a, &q1) == 0);
  ASSERT(ma_set_state(a, &x3) == 0);
  ASSERT(y[0] == 4);
  ASSERT(ma_step(&a, 1) == 0);
  ASSERT(y[0] == 5);
  ASSERT(ma_step(&a, 1) == 0);
  ASSERT(y[0] == 6);

  ma_delete(a);
  return PASS;
}

// Testuje dwa automaty tworzące dwubitowy licznik binarny.
static int two(void) {
  uint64_t x = 1;
  const uint64_t *y[2];
  moore_t *a[2];

  a[0] = ma_create_simple(1, 1, t_two);
  a[1] = ma_create_simple(1, 1, t_two);
  assert(a[0]);
  assert(a[1]);

  y[0] = ma_get_output(a[0]);
  y[1] = ma_get_output(a[1]);
  ASSERT(y[0] != NULL);
  ASSERT(y[1] != NULL);

  // Na początku licznik ma wartość 00.
  ASSERT(ma_set_input(a[0], &x) == 0);
  ASSERT(ma_connect(a[1], 0, a[0], 0, 1) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 0);

  // Po jednym kroku licznik ma wartość 01.
  ASSERT(ma_step(a, 2) == 0);
  ASSERT(y[0] != NULL);	// ..
  ASSERT(y[1] != NULL); // temporary for testing
  ASSERT(y[1][0] == 0 && y[0][0] == 1);

  // Po dwóch krokach licznik ma wartość 10.
  ASSERT(ma_step(a, 2) == 0);
  ASSERT(y[1][0] == 1 && y[0][0] == 0);

  // Po trzech krokach licznik ma wartość 11.
  ASSERT(ma_step(a, 2) == 0);
  ASSERT(y[1][0] == 1 && y[0][0] == 1);

  // Po czterech krokach licznik ma wartość 00.
  ASSERT(ma_step(a, 2) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 0);
  ASSERT(ma_step(a, 2) == 0);

  // Po pięciu krokach licznik ma wartość 01.
  ASSERT(y[1][0] == 0 && y[0][0] == 1);

  // Po rozłączeniu automatów starczy bit licznika przestaje się zmieniać.
  ASSERT(ma_disconnect(a[1], 0, 1) == 0);
  x = 0;
  ASSERT(ma_set_input(a[1], &x) == 0);
  ASSERT(ma_step(a, 2) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 0);
  ASSERT(ma_step(a, 2) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 1);

  ma_delete(a[0]);
  ma_delete(a[1]);
  return PASS;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
// Błąd alokacji jest zgłaszany raz. Druga próba powinna się udać.
static unsigned long alloc_fail_test(void) {
  const uint64_t q1 = 1;
  unsigned long visited = 0;
  moore_t *maf, *mas;

  errno = 0;
  if ((maf = ma_create_full(64, 64, 64, t_one, y_one, &q1)) != NULL)
    visited |= V(1, 0);
  else if (errno == ENOMEM &&
           (maf = ma_create_full(64, 64, 64, t_one, y_one, &q1)) != NULL)
    visited |= V(2, 0);
  else
    return visited |= V(4, 0); // To nie powinno się wykonać.

  errno = 0;
  if ((mas = ma_create_simple(1, 1, t_two)) != NULL)
    visited |= V(1, 1);
  else if (errno == ENOMEM && (mas = ma_create_simple(1, 1, t_two)) != NULL)
    visited |= V(2, 1);
  else
    return visited |= V(4, 1); // To nie powinno się wykonać.

  ma_delete(maf);
  ma_delete(mas);

  return visited;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static int memory_test(unsigned long (* test_function)(void)) {
  memory_test_data_t *mtd = get_memory_test_data();

  unsigned fail = 0, pass = 0;
  mtd->call_total = 0;
  mtd->fail_counter = 1;
  while (fail < 3 && pass < 3) {
    mtd->call_counter = 0;
    mtd->alloc_counter = 0;
    mtd->free_counter = 0;
    mtd->function_name = NULL;
    unsigned long visited_points = test_function();
    if (mtd->alloc_counter != mtd->free_counter ||
        (visited_points & 0444444444444444444444UL) != 0) {
      fprintf(stderr,
              "fail_counter %u, alloc_counter %u, free_counter %u, "
              "function_name %s, visited_point %lo\n",
              mtd->fail_counter, mtd->alloc_counter, mtd->free_counter,
              mtd->function_name, visited_points);
      ++fail;
    }
    if (mtd->function_name == NULL)
      ++pass;
    else
      pass = 0;
    mtd->fail_counter++;
  }

  return mtd->call_total > 0 && fail == 0 ? PASS : FAIL;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static int memory(void) {
  memory_tests_check();
  return memory_test(alloc_fail_test);
}

void t_add_timestamps(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
    next_state[0] = input[0]+input[1]; // sumujemy 2 liczby z wejścia
    next_state[1] = old_state[1] + 1; //zwiększamy licznik o 1
  }

  void t_shift3(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    next_state[0] = input[0] >> 3; //dzielimy przez 8
  }

  void t_3nplus1(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    next_state[0] = 3*input[0] + 1;
  }

  void t_plus1_mod8(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    next_state[0] = (input[0] + 1)%8;
  }


  const uint64_t M = 2137;
  void y_mulM(uint64_t *output, uint64_t const *state, size_t, size_t) {
    output[0] = (state[0] * state[1])%M;
  }

  //automat 0 dokonuje obliczeń na podsatwie wyjść automatów 0 i 1
  //zaś one dokonują obliczeń na podstawie automatu 0
  //
  //do tego na boku funkcjonuje automat disruptor, który łączy się na różne dziwne sposoby, jednak nie wpływa on na wynik
  //(podłącza się on jedynie do pierwszych 3 bitów automatu 1, które są nieistotne bo automat ten wykonuje dzielenie przez 8)
  //
  //UWAGA: zachęcam do spróbowania uruchomienia tego testu bez disruptora
  static int disruptor(void) {
    uint64_t zero = 2;
    const uint64_t q[2] = {17, 1};
    const uint64_t *y[3];
    moore_t *a[3];
    moore_t *disruptor = NULL;

    //automat 0 z każdym obrotem dodaje 2 liczby, które dostaje na wejściu oraz mnoży je przez licznik, który zwiększa co obrót
    //automat 1 dzieli wejście przez 8
    //automat 2 mnoży wejście 3 i dodaje 1
    a[0] = ma_create_full(100, 20, 80, t_add_timestamps, y_mulM, q);
    a[1] = ma_create_simple(20, 20, t_shift3);
    a[2] = ma_create_simple(20, 20, t_3nplus1);
    assert(a[0]);
    assert(a[1]);
    assert(a[2]);

    /*
      Początkowe stany automatów:
      ?, ? -> 17, 1 -> 17
      ? -> 0 -> 0
      ? -> 0 -> 0
    */


    y[0] = ma_get_output(a[0]);
    y[1] = ma_get_output(a[1]);
    y[2] = ma_get_output(a[2]);
    ASSERT(y[0] != NULL);
    ASSERT(y[0][0] == 17);
    ASSERT(y[1] != NULL);
    ASSERT(y[1][0] == 0);
    ASSERT(y[2] != NULL);
    ASSERT(y[2][0] == 0);


    ma_connect(a[1], 0, a[0], 0, 20);
    ma_connect(a[2], 0, a[0], 0, 20);
    ma_step(&(a[1]), 2);
    ma_connect(a[0], 0, a[1], 0, 20);
    ma_connect(a[0], 64, a[2], 0, 20);
    /*
      Obecne stany automatów
      2, 52 -> 17, 1 -> 17
      17 -> 2 -> 2
      17 -> 52 -> 52
    */
    ASSERT(y[0][0] == 17);
    ASSERT(y[1][0] == 2);
    ASSERT(y[2][0] == 52);

    uint64_t out0 = 17, out1 = 2, out2 = 52;
    for (uint64_t i = 2; i < 200; i++) {
      //printf("Loop for i = %ld \n", i);

      //Robimy krok na wszystkich automatach
      ASSERT(ma_step(a, 3) == 0);

      //Ręcznie liczymy nowe wyjścia automatów
      uint64_t out0_copy = out0;
      out0 = ((out1 + out2) * i)%M;
      out1 = out0_copy/8;
      out2 = 3*out0_copy + 1;

      //Sprawdzamy
      ASSERT(y[0][0] == out0);
      ASSERT(y[1][0] == out1);
      ASSERT(y[2][0] == out2);

      //Disruptor sieje ferment (bez wpływu na pozostałe wyniki)
      if (!disruptor) {
        //if (i%3 == 0) printf("i = %ld - TRIGGERS disruptor CREATION\n", i);
        if (i %3 == 0) disruptor = ma_create_simple(3, 3, t_plus1_mod8);
      } else {
        if (i%4 == 0) {
          //printf("i = %ld - TRIGGERS DELETE\n", i);
          ma_delete(disruptor);
          disruptor = NULL;
        } else {
          if (i%7 == 2 || i%7 == 3) {
            //podłączamy disruptor do 3 pierwszych bitów wejścia
            //printf("i = %ld - TRIGGERS CONNECT\n", i);
            ASSERT(ma_connect(a[1], 0, disruptor, 0, 3) == 0);
          } else {
            if (i%11 < 5) {
              //podłączamy wejście disruptora do fragmentu losowego wyjścia a0, a1, a2
              //printf("i = %ld - TRIGGERS WEIRD LINK UP\n", i);
              size_t rand_mod3 = (i%5+ 2)%3;
              size_t another = (rand_mod3 + 2 + i%8)%3;
              ASSERT(ma_connect(disruptor, 0, a[rand_mod3], 5, another + 1) == 0);
            } else {
              //printf("i = %ld - TRIGGERS DISCONNECTS\n", i);
              ASSERT(ma_disconnect(disruptor, 0, 3) == 0);
              ASSERT(ma_disconnect(a[1], 0, 3) == 0);
              ASSERT(ma_set_input(disruptor, &zero) == 0);
            }
          }
        }
      }
    }

    ma_delete(a[0]);
    ma_delete(a[1]);
    ma_delete(a[2]);
    if (disruptor) {
      ma_delete(disruptor);
    }
    printf("PASS\n");
    return PASS;
  }

void t_times3(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = 3 * input[0];
}

void t_plus(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] + input[0];
}

void t_shift(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
  next_state[0] = input[0] >> 60;
  if (s > 64 && n > 64) next_state[1] = input[1];
}

void y_plus1(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] + 1;
}

void y_times10(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] *10;
}

static int custom(void) {
  uint64_t x = 2;
  uint64_t x2 = 10;
  const uint64_t q1 = 0;
  const uint64_t *y[3];
  moore_t *a[3];
  const uint64_t zero[2] = {0, 0};

  a[0] = ma_create_full(10, 10, 10, t_times3, y_plus1, &q1);
  a[1] = ma_create_full(10, 10, 10, t_plus, y_times10, &q1);
  a[2] = ma_create_simple(100, 100, t_shift);
  assert(a[0]);
  assert(a[1]);
  assert(a[2]);

  /*
  Stan automatów:
  0 -> 0 -> 1
  0 -> 0 -> 0
  0, 0 -> 0, 0 -> 0, 0
  */
  y[0] = ma_get_output(a[0]);
  y[1] = ma_get_output(a[1]);
  y[2] = ma_get_output(a[2]);
  ASSERT(y[0] != NULL);
  ASSERT(y[0][0] == 1);
  ASSERT(y[1] != NULL);
  ASSERT(y[1][0] == 0);
  ASSERT(y[2] != NULL);
  ASSERT(y[2][0] == 0 && y[2][1] == 0);

  //Przepisujemy bity z wyjscia a[0] na wyjście a[1]
  ASSERT(ma_connect(a[1], 0, a[0], 0, 10) == 0);
  //Przepisujemy bity z wyjścia a[1] na bity 60-69 wejścia a[1]
  //tym samym rodzielamy wyjście a[1] na 2 części 4-bitową i 6-bitową
  ASSERT(ma_connect(a[2], 60, a[1], 0, 10) == 0);

  ma_set_input(a[0], &x);
  ma_set_input(a[1], zero);
  ma_set_input(a[2], zero);
  //printf("1.\n%lu\n%lu\n%lu %lu\n", y[0][0], y[1][0], y[2][0], y[2][1]);
  ma_step(a, 1);
  //printf("2.\n%lu\n%lu\n%lu %lu\n", y[0][0], y[1][0], y[2][0], y[2][1]);
  ma_step(&(a[1]), 1);
  //printf("3.\n%lu\n%lu\n%lu %lu\n", y[0][0], y[1][0], y[2][0], y[2][1]);
  ma_step(&(a[2]), 1);

  /*
  Stan automatów:
  2 -> 6 -> 7
  7 -> 7 -> 70
  2^61 + 2^62, 4 -> 6, 4 -> 6, 4
  */

  ASSERT(y[0][0] == 7);
  ASSERT(y[1][0] == 70);
  //printf("4.\n%lu\n%lu\n%lu %lu\n", y[0][0], y[1][0], y[2][0], y[2][1]);
  ASSERT(y[2][0] == 6);
  ASSERT(y[2][1] == 4);

  //odłączamy pierwszy bit wejścia automatu 1


  ma_disconnect(a[1], 0, 1);
  //zastępujemy go zeraem (podłaczone bity pozostają nietknięte)
  //teraz na wejściu a[1] powinno znajdowac się 6
  ma_set_input(a[1], &q1);
  ma_step(&(a[1]), 1);
  ASSERT(y[1][0] == 130);
  /*
  Stan automatów:
  2 -> 6 -> 7
  6 -> 13 -> 130
  2^61, 8 -> 6, 4 -> 6, 4
  */

  ASSERT(y[0][0] == 7);
  ASSERT(y[1][0] == 130);
  ASSERT(y[2][0] == 6);
  ASSERT(y[2][1] == 4);

  ma_set_input(a[0], &x2);
  ma_step(a, 3);

  ASSERT(y[0][0] == 31);
  ASSERT(y[1][0] == 190);
  ASSERT(y[2][0] == 2);
  ASSERT(y[2][1] == 8);
  /*
  Stan automatów:
  10 -> 30 -> 31
  (nieistotne (31)) -> 19 -> 190
  (niestotne) -> 2, 8 -> 2, 8
  */


  ma_delete(a[0]);
  ma_delete(a[1]);
  ma_delete(a[2]);
  return PASS;
}

void t_weird(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[0] + (input[1] << 14);
  next_state[1] = old_state[1] + (input[0] << 14) + (input[1] >> 50);
  next_state[2] = old_state[2] + (input[2] >> 50) + (input[0] >> 50) ^ input[1];
}

void t_weird2(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[1] + (input[1] << 13);
  next_state[1] = old_state[2] + (input[0] << 13) + (input[1] >> 50);
  next_state[2] = old_state[0] * 7 + (input[2] ^ 123879) + 14;
}

void t_weird3(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t, size_t) {
  next_state[0] = old_state[1] + (input[1] << 12);
  next_state[1] = old_state[0] + (input[0] << 12) + (input[1] >> 2) + 123897;
  next_state[2] = old_state[0] + (input[2] >> 2) + (input[0] >> 2) ^ input[1] * 4;
}

void y_xor(uint64_t *output, uint64_t const *state, size_t, size_t) {
  output[0] = state[0] & state[1];
  output[1] = state[2] ^ state[0];
  output[2] = state[1] ^ state[2];
}

#include <stdio.h>
#include <stdlib.h>

static int custom2(void) {
  const uint64_t q0[3] = {0, 0, 0};
  const uint64_t q_noise[3] = {120ll*315*213*1273+123, 91278381953132213, 1982357853123483726};
  const uint64_t q_noise2[3] = {
    q_noise[0] ^ q_noise[1] + q_noise[2],
    q_noise[1] ^ q_noise[2] + q_noise[0] * 12,
    q_noise[2] + q_noise[0] * 7 + 2 * q_noise[1]
  };
  const uint64_t q_noise3[3] = {
    q_noise[0] + q_noise2[1] * 2 + q_noise[2] * 3,
    q_noise2[1] + q_noise[2] * 4 + q_noise[0] * 5,
    q_noise[2] + q_noise[0] * 6 + q_noise2[1] * 7
  };

  const size_t N = 10; // Zapomniałem, że to istnieje, więc lepiej tego nie zmieniać

  // create the automatas

  moore_t *a[N];
  const uint64_t* o[N];
  a[0] = ma_create_full(192, 192, 192, t_weird, y_xor, q_noise);
  a[1] = ma_create_full(192, 192, 192, t_weird2, y_xor, q_noise2);
  a[2] = ma_create_full(192, 192, 192, t_weird3, y_xor, q_noise3);
  for (size_t i = 3; i < N; i++) {
    a[i] = ma_create_simple(192, 192, t_weird3);
  }

  for(size_t i = 0; i < N; i++) {
    ASSERT(a[i] != NULL);
  }

  for(size_t i = 0; i < N; i++) {
    o[i] = ma_get_output(a[i]);
    ASSERT(o[i] != NULL);
  }

  uint64_t operation = 1;
  scanf("%llu", &operation);
  for(size_t i = 0; operation != 0; ++i) {
    operation++;
    if(operation < 10){
      ma_delete(a[operation]);
      a[operation] = ma_create_simple(192, 192, t_weird);
      ASSERT(a[operation] != NULL);
      o[operation] = ma_get_output(a[operation]);
      ASSERT(o[operation] != NULL);
    }
    else if(operation < 20){
      operation -= 10;
      ma_step(&a[operation], 1);
    }
    else if(operation < 60){
      operation -= 20;
      if(operation % 4 == 0){
        ma_set_input(a[operation/4], q_noise);
      }
      else if(operation % 4 == 1){
        ma_set_input(a[operation/4], q_noise2);
      }
      else if(operation % 4 == 2){
        ma_set_input(a[operation/4], q_noise3);
      }
      else{
        ma_set_input(a[operation/4], q0);
      }
    }
    else{
      size_t automata_idx = operation % 10;
      operation /= 10;

      size_t num_bits = operation % 192 + 1;
      operation /= 192;

      size_t out_idx = operation % 192;
      operation /= 192;

      size_t in_idx = operation % 192;
      operation /= 192;

      if(in_idx + num_bits > 192){
        num_bits = 192 - in_idx;
      }
      if(out_idx + num_bits > 192){
        num_bits = 192 - out_idx;
      }

      operation %= 10;

      ma_connect(a[automata_idx], in_idx, a[operation], out_idx, num_bits);
    }

    ma_step(a, N);
    for(int j = 0; j < N; j++){
      printf("%llu %llu %llu\n", o[j][0], o[j][1], o[j][2]);
    }

    scanf("%llu", &operation);
  }

  for (int i = 0; i < N; i++) {
  	if (a[i]) {
    	ma_delete(a[i]);
    }
  }
  return PASS;
}

uint64_t popcount64(uint64_t x) {
  uint64_t count = 0;
  while (x) {
      count += x & 1;  // Add least significant bit
      x >>= 1;         // Shift right
  }
  return count;
}

void t_zero(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    for (size_t i = 0; i <= s/64; i++) next_state[i] = 0;
  }

  void t_bitcounter(uint64_t *next_state, uint64_t const *input, uint64_t const *old_state, size_t n, size_t s) {
    for (size_t i = 0; i <= n/64; i++) next_state[0] += popcount64(input[i]);
  }

  const uint64_t STATE_MOD = 6765878346298313;
  void refresh_state(uint64_t state[]) {
    state[0] = (state[0] + 2*state[1])%STATE_MOD;
    state[1] = (3*state[0] + state[1])%STATE_MOD;
  }

  /*
    Test tworzy N + 1 automatów simple
    Automaty w tablicy a - mają 0 wejścia i funkcję tranzycji zerująca stan
    Automat counter zlicza bity na wejściu - ma on duuuże wejście, do którego podpinają się automaty a (do niezależnych fragmentów)
    Następnie automaty z tablicy a po kolei zerują swoje wejście (wykonując krok)
  */
  static int counter_test(void) {
    const size_t N = 30;
    const size_t p = 131;
    const size_t c = 257;
    moore_t* a[N];
    for (size_t i = 0; i < N; i++) {
      a[i] = ma_create_simple(0, 124, t_zero);
      assert(a[i]);
    }
    moore_t* counter = ma_create_simple(5000, 30, t_bitcounter);
    const uint64_t *y = ma_get_output(counter);
    assert(counter);
    assert(y);
    ASSERT(y[0] == 0);

    uint64_t state[2];
    state[0] = 89327592387342423;
    state[1] = 90780953242323231;
    size_t perm[N];
    //w miarę losowa permutacja liczb 0, ..., N-1
    perm[0] = 7; for (size_t i = 1; i < N; i++) perm[i] = (perm[i-1] + 11)%N;

    uint64_t expected_ans = 0;
    for (size_t perm_id = 0; perm_id < N; perm_id++) {
      //generujemy losowy stan a[i] i podpinamy jego wyjście (121 bitów) do przedziału wejścia countera
      size_t i = perm[perm_id];
      refresh_state(state);
      ma_set_state(a[i], state);
      ma_connect(counter, (i*p) + c, a[i], 0, 121);
      ma_step(&counter, 1);
      expected_ans += popcount64(state[0]) + popcount64(state[1]);
      ASSERT(y[0] == expected_ans);
    }

    //printf("\n \nReshuffling \n \n");
    //w miarę losowa permutacja liczb 0, ..., N-1
    perm[0] = 13; for (size_t i = 1; i < N; i++) perm[i] = (perm[i-1] + 17)%N;

    for (size_t perm_id = 0; perm_id < N; perm_id++) {
      size_t i = perm[perm_id];

      //czyscimy (step dla a[i] oznacza wyzerowanie outputu)
      uint64_t* curr_y = ma_get_output(a[i]);
      assert(curr_y);
      expected_ans -= popcount64(curr_y[0]) + popcount64(curr_y[1]);
      ma_step(&(a[i]), 1);

      ma_step(&counter, 1);
      ASSERT(y[0] == expected_ans);
    }


    for (size_t i = 0; i < N; i++) ma_delete(a[i]);
    ma_delete(counter);
    printf("PASS\n");
  }


/** URUCHAMIANIE TESTÓW **/

typedef struct {
  char const *name;
  int (*function)(void);
} test_list_t;

#define TEST(t) {#t, t}

static const test_list_t test_list[] = {
  TEST(one),
  TEST(two),
  TEST(memory),
  TEST(disruptor),
  TEST(custom),
  TEST(custom2),
  TEST(counter_test);
};

static int do_test(int (*function)(void)) {
  int result = function();
  puts("quite long magic string");
  return result;
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    for (size_t i = 0; i < SIZE(test_list); ++i)
      if (strcmp(argv[1], test_list[i].name) == 0)
        return do_test(test_list[i].function);

  fprintf(stderr, "Użycie:\n%s nazwa_testu\n", argv[0]);
  return WRONG_TEST;
}
