#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// https://link.springer.com/referenceworkentry/10.1007%2F0-387-23483-7_145
static void extended_euclidean(mpz_t a, mpz_t b, mpz_t x, mpz_t y, mpz_t gcd) {
  mpz_t q, u, v, r, s, t, t2, t1, s2, s1;
  mpz_inits(q, u, v, r, s, t, t2, t1, s2, s1, NULL);

  mpz_set_ui(s1, 0);
  mpz_set_ui(s2, 1);
  mpz_set_ui(t1, 1);
  mpz_set_ui(t2, 0);

  mpz_set(u, a);
  mpz_set(v, b);

  while (mpz_sgn(v) > 0) {
    mpz_div(q, u, v);

    mpz_mul(r, q, v);
    mpz_sub(r, u, r);

    mpz_mul(s, q, s1);
    mpz_sub(s, s2, s);

    mpz_mul(t, q, t1);
    mpz_sub(t, t2, t);

    mpz_set(u, v);
    mpz_set(v, r);

    mpz_set(s2, s1);
    mpz_set(s1, s);
    mpz_set(t2, t1);
    mpz_set(t1, t);
  }

  mpz_set(gcd, u);
  mpz_set(x, s2);
  mpz_set(y, t2);

  mpz_clears(q, u, v, r, s, t, t2, t1, s2, s1, NULL);
}

static void gcd_euclid(mpz_t a, mpz_t b, mpz_t gcd) {
  mpz_t x, y;
  mpz_inits(x, y, NULL);
  extended_euclidean(a, b, x, y, gcd);
  mpz_clears(x, y, NULL);
}

// https://comeoncodeon.wordpress.com/2010/09/18/pollard-rho-brent-integer-factorization/
static void pollard_rho_brent(mpz_t n, mpz_t p) {
  if (mpz_even_p(n)) {
    mpz_set_ui(p, 2);
    return;
  }

  mpz_t nsub1, y, c, m, g, r, q, k, x, ys, i, j, min, tmp;
  mpz_inits(nsub1, y, c, m, g, r, q, k, x, ys, i, j, min, tmp, NULL);
  mpz_sub_ui(nsub1, n, 1);

  gmp_randstate_t state;
  gmp_randinit_default(state);
  gmp_randseed_ui(state, time(NULL));

  // generate y
  mpz_urandomm(y, state, nsub1);
  mpz_add_ui(y, y, 1);

  // generate c
  mpz_urandomm(c, state, nsub1);
  mpz_add_ui(c, c, 1);

  // generate m
  mpz_urandomm(m, state, nsub1);
  mpz_add_ui(m, m, 1);

  mpz_set_ui(g, 1);
  mpz_set_ui(r, 1);
  mpz_set_ui(q, 1);

  while (mpz_cmp_ui(g, 1) == 0) {
    mpz_set(x, y);

    // for (i = 0; i < r; i++)
    for (mpz_set_ui(i, 0); mpz_cmp(i, r) < 0; mpz_add_ui(i, i, 1)) {
      // y = (y*y mod n + c) mod n
      mpz_mul(y, y, y);
      mpz_mod(y, y, n);
      mpz_add(y, y, c);
      mpz_mod(y, y, n);
    }

    mpz_set_ui(k, 0);

    // while(k<r && g == 1)
    while (mpz_cmp(k, r) < 0 && mpz_cmp_ui(g, 1) == 0) {
      mpz_set(ys, y);

      // min = min(m, r-k)
      mpz_sub(tmp, r, k);
      mpz_set(min, mpz_cmp(m, tmp) < 0 ? m : tmp);

      // for(j=0; j < min; j++)
      for (mpz_set_ui(j, 0); mpz_cmp(j, min) < 0; mpz_add_ui(j, j, 1)) {
        // y = (y*y mod n + c) mod n
        mpz_mul(y, y, y);
        mpz_mod(y, y, n);
        mpz_add(y, y, c);
        mpz_mod(y, y, n);

        // q = abs(x-y)*q mod n
        mpz_sub(tmp, x, y);
        mpz_abs(tmp, tmp);
        mpz_mul(tmp, tmp, q);
        mpz_mod(q, tmp, n);
      }

      mpz_gcd(g, n, q);
      mpz_add(k, k, m);
    }

    // r = r * 2
    mpz_mul_ui(r, r, 2);
  }

  if (mpz_cmp(g, n) == 0) {
    do {
      // ys = (ys*ys mod n + c) mod n
      mpz_mul(ys, ys, ys);
      mpz_mod(ys, ys, n);
      mpz_add(ys, ys, c);
      mpz_mod(ys, ys, n);
      // g = |x-ys|
      mpz_sub(g, x, ys);
      mpz_abs(g, g);
      mpz_gcd(g, n, g);
    } while (!(mpz_cmp_ui(g, 1) > 0));
  }

  mpz_set(p, g);
  mpz_clears(nsub1, y, c, m, g, r, q, k, x, ys, i, j, min, tmp, NULL);
  gmp_randclear(state);
}

static void compute_e_phi(mpz_t p, mpz_t q, gmp_randstate_t state, mpz_t e,
                          mpz_t phi) {
  mpz_t psub1, qsub1, gcd;
  mpz_inits(psub1, qsub1, gcd, NULL);

  // phi = (p - 1) * (q - 1)
  mpz_sub_ui(psub1, p, 1);
  mpz_sub_ui(qsub1, q, 1);
  mpz_mul(phi, psub1, qsub1);
  // choose e in range (1, phi) and check if gcd(e, phi) == 1
  do {
    mpz_urandomm(e, state, phi);
    mpz_add_ui(e, e, 1);
    gcd_euclid(e, phi, gcd);
  } while (mpz_cmp_ui(gcd, 1) != 0);

  mpz_clears(psub1, qsub1, gcd, NULL);
}

// Miller Rabin primality test
static int miller_rabin_test(mpz_t d, mpz_t n, gmp_randstate_t state) {
  int is_prime = 0;
  mpz_t a, x, tmp;
  mpz_inits(a, x, tmp, NULL);

  // n - 4
  mpz_sub_ui(tmp, n, 4);
  mpz_urandomm(a, state, tmp);
  mpz_add_ui(a, a, 2);

  // a^d % n
  mpz_powm(x, a, d, n);
  // n - 1
  mpz_sub_ui(tmp, n, 1);
  if (mpz_cmp_ui(x, 1) == 0 || mpz_cmp(x, tmp) == 0) {
    is_prime = 1;
    goto cleanup;
  }

  while (mpz_cmp(d, tmp) != 0) {
    // (x * x) % n;
    mpz_mul(x, x, x);
    mpz_mod(x, x, n);
    // d = d * 2
    mpz_mul_ui(d, d, 2);

    if (mpz_cmp_ui(x, 1) == 0) {
      is_prime = 0;
      goto cleanup;
    }
    if (mpz_cmp(x, tmp) == 0) {
      is_prime = 1;
      goto cleanup;
    }
  }

cleanup:
  mpz_clears(a, x, tmp, NULL);
  return is_prime;
}

// https://helloacm.com/fermat-prime-test-algorithm/
static int fermat_test(const mpz_t n, gmp_randstate_t state) {
  int is_prime = 1;
  mpz_t tmp, a, nsub1;
  mpz_inits(tmp, a, nsub1, NULL);
  mpz_sub_ui(nsub1, n, 1);

  // pick random a
  do {
    mpz_urandomm(a, state, n);
  } while (mpz_cmp_ui(a, 2) < 0);

  // a^(n-1) mod n
  mpz_powm(tmp, a, nsub1, n);
  is_prime = mpz_cmp_ui(tmp, 1) == 0;

  mpz_clears(tmp, a, nsub1, NULL);
  return is_prime;
}

// https://comeoncodeon.wordpress.com/2010/09/18/miller-rabin-primality-test/
static int is_prime_numer(mpz_t number, gmp_randstate_t state,
                          size_t iterations) {
  if (mpz_cmp_ui(number, 1) <= 0 || mpz_cmp_ui(number, 4) == 0)
    return 0;
  if (mpz_cmp_ui(number, 3) <= 0)
    return 1;
  mpz_t d;
  mpz_init_set(d, number);
  // d = d - 1
  mpz_sub_ui(d, d, 1);
  // Divide even d by 2 till d is odd
  while (mpz_even_p(d))
    // d = d / 2
    mpz_tdiv_q_2exp(d, d, 1);

  int is_prime = fermat_test(number, state);
  if (is_prime) {
    // https://crypto.stackexchange.com/questions/71/how-can-i-generate-large-prime-numbers-for-rsa
    for (size_t i = 0; i < iterations; ++i) {
      if (!miller_rabin_test(d, number, state)) {
        is_prime = 0;
        break;
      }
    }
  }

  mpz_clears(d, NULL);
  return is_prime;
}

static void generate_prime(mp_bitcnt_t bitlength, gmp_randstate_t state,
                           mpz_t prime) {
  do {
    mpz_urandomb(prime, state, bitlength);
    // Set the highest bit to achieve the claimed key length
    mpz_setbit(prime, bitlength - 1);
    mpz_setbit(prime, bitlength - 2);
    mpz_setbit(prime, 0);
  } while (!is_prime_numer(prime, state, 64 /* libgcrypto */));
}

static void compute_p_q_n(mp_bitcnt_t bitlength, gmp_randstate_t state, mpz_t p,
                          mpz_t q, mpz_t n) {
  do {
    generate_prime((bitlength / 2) + (bitlength & 1), state, p);
    generate_prime(bitlength / 2, state, q);
  } while (mpz_cmp(p, q) == 0);
  // n = p * q
  mpz_mul(n, p, q);
}

// a^(-1) mod n
static void mult_inverse(mpz_t a, mpz_t n, mpz_t x) {
  mpz_t y, gcd;
  mpz_inits(y, gcd, NULL);
  extended_euclidean(a, n, x, y, gcd);
  if (mpz_sgn(x) < 0) {
    // make x positive
    mpz_add(x, n, x);
  }
  mpz_clears(y, gcd, NULL);
}

static void generate_keys(mp_bitcnt_t bitlength) {
  mpz_t p, q, n, e, d, phi;
  mpz_inits(p, q, n, e, d, phi, NULL);
  gmp_randstate_t state;
  gmp_randinit_default(state);
  gmp_randseed_ui(state, time(NULL));

  do {
    compute_p_q_n(bitlength, state, p, q, n);
    compute_e_phi(p, q, state, e, phi);
    mult_inverse(e, phi, d);
  } while (mpz_cmp(e, d) == 0);

  gmp_printf("%#Zx %#Zx %#Zx %#Zx %#Zx\n", p, q, n, e, d);
  mpz_clears(p, q, n, e, d, phi, NULL);
  gmp_randclear(state);
}

static void break_rsa(mpz_t e, mpz_t n, mpz_t c) {
  mpz_t p, q, d, phi, psub1, qsub1, plaintext;
  mpz_inits(p, q, d, phi, psub1, qsub1, plaintext, NULL);
  pollard_rho_brent(n, p);
  mpz_div(q, n, p);
  // phi = (p - 1) * (q - 1)
  mpz_sub_ui(psub1, p, 1);
  mpz_sub_ui(qsub1, q, 1);
  mpz_mul(phi, psub1, qsub1);
  mult_inverse(e, phi, d);
  mpz_powm(plaintext, c, d, n);
  gmp_printf("%#Zx %#Zx %#Zx\n", p, q, plaintext);
  mpz_clears(p, q, d, phi, psub1, qsub1, plaintext, NULL);
}

int main(int argc, char **argv) {
  switch (argc) {
  case 3:
    if (strcmp(argv[1], "-g") == 0) {
      char *err;
      long long bitlength = strtoll(argv[2], &err, 10);
      if (*err) {
        fprintf(stderr, "Invalid bitsize value!\n");
        return 1;
      } else {
        if (bitlength > 6) {
          generate_keys(bitlength);
          return 0;
        } else {
          fprintf(stderr, "Unable to generate keys for B = %lld!\n", bitlength);
          return 1;
        }
      }
    } else {
      fprintf(stderr, "Option \"-b\" expected!\n");
      return 1;
    }
  case 5:

    if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "-d") == 0 ||
        strcmp(argv[1], "-b") == 0) {
      for (int i = 2; i <= 4; ++i)
        if (argv[i][0] != '0' || argv[i][1] != 'x') {
          fprintf(stderr, "Missing '0x' before %s\n", argv[i]);
          return 1;
        }
    }

    if (strcmp(argv[1], "-e") == 0) {
      mpz_t e, n, m, c;
      mpz_init_set_str(e, argv[2] + 2, 16);
      mpz_init_set_str(n, argv[3] + 2, 16);
      mpz_init_set_str(m, argv[4] + 2, 16);
      mpz_init(c);
      int ok;
      if (mpz_cmp_ui(n, 0) == 0) {
        ok = 0;
        fprintf(stderr, "n must be > 0\n");
      } else {
        ok = !(mpz_cmp_ui(m, 0) < 0) && mpz_cmp(m, n) < 0;
        if (ok) {
          mpz_powm(c, m, e, n);
          gmp_printf("%#Zx\n", c);
        } else {
          fprintf(stderr, "m must be in range 0 <= m < n\n");
        }
      }
      mpz_clears(e, n, m, c, NULL);
      return ok ? 0 : 1;
    } else if (strcmp(argv[1], "-d") == 0) {
      mpz_t d, n, c, m;
      mpz_init_set_str(d, argv[2] + 2, 16);
      mpz_init_set_str(n, argv[3] + 2, 16);
      mpz_init_set_str(c, argv[4] + 2, 16);
      mpz_init(m);
      int ok = mpz_sgn(n) != 0;
      if (ok) {
        mpz_powm(m, c, d, n);
        gmp_printf("%#Zx\n", m);
      } else {
        fprintf(stderr, "n must be > 0\n");
      }
      mpz_clears(d, n, c, m, NULL);
      return ok ? 0 : 1;
    }
    if (strcmp(argv[1], "-b") == 0) {
      mpz_t e, n, c;
      mpz_init_set_str(e, argv[2] + 2, 16);
      mpz_init_set_str(n, argv[3] + 2, 16);
      mpz_init_set_str(c, argv[4] + 2, 16);
      int ok = mpz_sgn(n) != 0;
      if (ok)
        break_rsa(e, n, c);
      else
        fprintf(stderr, "n must be > 0\n");
      mpz_clears(e, n, c, NULL);
      return ok ? 0 : 1;
    } else {
      fprintf(stderr, "Option \"-e\" or \"-d\" expected!\n");
      return 1;
    }
  default:
    fprintf(stderr, "Invalid arguments!\n");
    return 1;
  }
}
