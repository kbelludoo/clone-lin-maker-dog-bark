/*
 * Independent C11 oracle for Maker/Sky Dog.bark (uint64-scale).
 *
 * ink*spot, art*rate, room*WAD, dart*rate, due*chop two implementations:
 *   1. wrapping-detect unsigned mul (p/a == b and p <= INT64_MAX)
 *   2. __int128: (int128)a * (int128)b in [0, INT64_MAX]
 *
 * Consensus between (1) and (2) is required before any LIN comparison.
 * This file is not linked into the LIN compiler TCB.
 *
 * Upstream algorithm: makerdao/dss src/dog.sol bark / min / mul / Hole / Dirt
 * (AGPL-3.0-or-later). This oracle is original C11 and does not copy dog.sol.
 *
 * Distinct from clipper.take (auction buy) and vat.slip/flux/move.
 * bark is public (not wards-auth). chip/tip are clipper.kick/redo, not bark.
 * vat.grab / vow.fess / ClipperLike.kick are return values / next_id, not calls.
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define I64MAX ((uint64_t)INT64_MAX)
#define WAD 1000ULL

static int use_i128 = 0;

static int u_mul_ok_wrap(uint64_t a, uint64_t b) {
  uint64_t p;
  if (a > I64MAX || b > I64MAX) return 0;
  if (a == 0 || b == 0) return 1;
  p = a * b;
  if (p > I64MAX) return 0;
  return p / a == b;
}

static int u_mul_ok_i128(uint64_t a, uint64_t b) {
  __int128 p;
  if (a > I64MAX || b > I64MAX) return 0;
  p = (__int128)a * (__int128)b;
  return p >= 0 && p <= (__int128)I64MAX;
}

static int u_mul_ok(uint64_t a, uint64_t b) {
  return use_i128 ? u_mul_ok_i128(a, b) : u_mul_ok_wrap(a, b);
}

static uint64_t u_mul(uint64_t a, uint64_t b) {
  if (!u_mul_ok(a, b)) return 0;
  return a * b;
}

static int u_add_ok(uint64_t a, uint64_t b) {
  uint64_t s;
  if (a > I64MAX || b > I64MAX) return 0;
  s = a + b;
  return s >= a && s <= I64MAX;
}

typedef struct {
  uint64_t live, ink, art, rate, spot, dust;
  uint64_t g_hole, g_dirt, i_hole, i_dirt, chop, next_id;
} BarkIn;

static int parse_in(char **a, BarkIn *p) {
  p->live = strtoull(a[0], NULL, 10);
  p->ink = strtoull(a[1], NULL, 10);
  p->art = strtoull(a[2], NULL, 10);
  p->rate = strtoull(a[3], NULL, 10);
  p->spot = strtoull(a[4], NULL, 10);
  p->dust = strtoull(a[5], NULL, 10);
  p->g_hole = strtoull(a[6], NULL, 10);
  p->g_dirt = strtoull(a[7], NULL, 10);
  p->i_hole = strtoull(a[8], NULL, 10);
  p->chop = strtoull(a[10], NULL, 10);
  p->next_id = strtoull(a[11], NULL, 10);
  p->i_dirt = strtoull(a[9], NULL, 10);
  return 1;
}
