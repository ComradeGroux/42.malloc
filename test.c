#include "malloc.h"
#include <stdio.h>
#include <string.h>

/* ─────────────────────────────────────────────
   Couleurs & compteurs
   ───────────────────────────────────────────── */
#define GREEN	"\033[38;5;46m"
#define RED		"\033[0;31m"
#define YELLOW	"\033[38;5;226m"
#define CYAN	"\033[38;5;51m"
#define GREY	"\033[38;5;240m"
#define RESET	"\033[0m"

static int	g_pass = 0;
static int	g_fail = 0;

/* ─────────────────────────────────────────────
   Helpers
   ───────────────────────────────────────────── */
static void	print_section(const char *name)
{
	printf("\n" CYAN "══════════════════════════════════════\n");
	printf("  %s\n", name);
	printf("══════════════════════════════════════" RESET "\n");
}

static void	check(const char *label, int condition)
{
	if (condition)
	{
		printf(GREEN "  [PASS]" RESET " %s\n", label);
		g_pass++;
	}
	else
	{
		printf(RED "  [FAIL]" RESET " %s\n", label);
		g_fail++;
	}
}

static void	dump(const char *ctx)
{
	printf(YELLOW "\n  -- show_alloc_mem : %s --\n" RESET GREY, ctx);
	show_alloc_mem();
	printf(RESET);
}

/* ─────────────────────────────────────────────
   1. Cas limites — malloc / free
   ───────────────────────────────────────────── */
static void	test_edge_cases(void)
{
	print_section("1. Cas limites — malloc / free");

	/* free(NULL) ne doit pas crasher */
	free(NULL);
	check("free(NULL) ne crashe pas", 1);

	/* malloc(0) ne doit pas crasher */
	void	*p0 = malloc(0);
	check("malloc(0) ne crashe pas", 1);
	free(p0);

	/* Taille 1 */
	void	*p1 = malloc(1);
	check("malloc(1) != NULL", p1 != NULL);
	if (p1) *(char *)p1 = 0x42;
	check("malloc(1) : ecriture sans crash", 1);
	free(p1);

	/* Frontieres TINY / SMALL / LARGE */
	void	*tiny_max = malloc(128);
	check("malloc(128) [TINY max] != NULL", tiny_max != NULL);
	dump("apres malloc(128)");
	free(tiny_max);
	dump("apres free(128)");

	void	*small_min = malloc(129);
	check("malloc(129) [SMALL min] != NULL", small_min != NULL);
	dump("apres malloc(129)");
	free(small_min);

	void	*small_max = malloc(1024);
	check("malloc(1024) [SMALL max] != NULL", small_max != NULL);
	dump("apres malloc(1024)");
	free(small_max);

	void	*large_min = malloc(1025);
	check("malloc(1025) [LARGE min] != NULL", large_min != NULL);
	dump("apres malloc(1025)");
	free(large_min);

	/* Tres grande allocation */
	void	*big = malloc(1024 * 1024 * 10);
	check("malloc(10 Mo) != NULL", big != NULL);
	if (big)
	{
		memset(big, 0xAB, 1024 * 1024 * 10);
		check("malloc(10 Mo) : memset complet sans crash", 1);
	}
	dump("apres malloc(10 Mo)");
	free(big);
	dump("apres free(10 Mo)");

	/* Alignement sur 16 octets pour plusieurs tailles */
	size_t	sizes[] = { 1, 3, 7, 9, 15, 17, 127, 129, 1023, 1025 };
	int		align_ok = 1;
	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
	{
		void	*pa = malloc(sizes[i]);
		if (!pa || ((unsigned long)pa % 16) != 0)
		{
			align_ok = 0;
			printf(RED "  alignement KO pour size=%zu (%p)\n" RESET, sizes[i], pa);
		}
		free(pa);
	}
	check("toutes tailles : retour aligne sur 16 octets", align_ok);
}

/* ─────────────────────────────────────────────
   2. Fragmentation et coalescence
   ───────────────────────────────────────────── */
static void	test_coalescence(void)
{
	print_section("2. Fragmentation et coalescence");

	/* ── 2a. Liberation sequentielle ── */
	printf(YELLOW "\n  [2a] Liberation sequentielle\n" RESET);

	void	*a1 = malloc(32);
	void	*a2 = malloc(32);
	void	*a3 = malloc(32);
	void	*a4 = malloc(32);
	printf("  a1=%p a2=%p a3=%p a4=%p\n", a1, a2, a3, a4);
	dump("apres 4x malloc(32)");

	free(a1); free(a2); free(a3); free(a4);
	dump("apres 4x free sequentiel");

	void	*reuse = malloc(100);
	check("[2a] apres coalescence : malloc(100) dans la zone reussit", reuse != NULL);
	dump("apres re-malloc(100)");
	free(reuse);

	/* ── 2b. Patron libre | occupe | libre ── */
	printf(YELLOW "\n  [2b] Patron libre | occupe | libre\n" RESET);

	void	*b1 = malloc(64);
	void	*b2 = malloc(64);
	void	*b3 = malloc(64);
	printf("  b1=%p b2=%p b3=%p\n", b1, b2, b3);
	dump("apres 3x malloc(64)");

	free(b1);
	free(b3);
	dump("apres free(b1) et free(b3) — b2 toujours occupe");

	free(b2);
	dump("apres free(b2) — coalescence triple attendue");

	void	*merged = malloc(160);
	check("[2b] coalescence triple : malloc(160) reussit", merged != NULL);
	dump("apres re-malloc(160)");
	free(merged);

	/* ── 2c. Fragmentation avancee (5 blocs) ── */
	printf(YELLOW "\n  [2c] Fragmentation avancee (5 blocs)\n" RESET);

	void	*c[5];
	for (int i = 0; i < 5; i++)
		c[i] = malloc(48);
	printf("  c[0]=%p c[1]=%p c[2]=%p c[3]=%p c[4]=%p\n",
		c[0], c[1], c[2], c[3], c[4]);
	dump("apres 5x malloc(48)");

	free(c[0]); free(c[2]); free(c[4]);
	dump("apres free c[0], c[2], c[4] — 3 trous");

	free(c[1]);
	dump("apres free c[1] — coalescence avec c[0] et c[2]");

	free(c[3]);
	dump("apres free c[3] — heap doit etre entierement libre");

	void	*full = malloc(200);
	check("[2c] heap recouvre totalement : malloc(200) reussit", full != NULL);
	free(full);
}

/* ─────────────────────────────────────────────
   3. realloc — cas limites
   ───────────────────────────────────────────── */
static void	test_realloc_edge(void)
{
	print_section("3. realloc — cas limites");

	/* realloc(NULL, n) == malloc(n) */
	void	*p = realloc(NULL, 64);
	check("realloc(NULL, 64) != NULL", p != NULL);
	dump("apres realloc(NULL, 64)");
	free(p);

	/* realloc(ptr, 0) == free(ptr) */
	p = malloc(64);
	void	*r = realloc(p, 0);
	check("realloc(ptr, 0) retourne NULL", r == NULL);
	dump("apres realloc(ptr, 0)");

	/* realloc meme taille : meme pointeur attendu */
	p = malloc(128);
	void	*same = realloc(p, 128);
	check("realloc meme taille : meme pointeur", same == p);
	dump("apres realloc(128, 128)");
	free(same);
}

/* ─────────────────────────────────────────────
   4. realloc — shrink
   ───────────────────────────────────────────── */
static void	test_realloc_shrink(void)
{
	print_section("4. realloc — shrink");

	/* SMALL 512 → 128 */
	char	*p = malloc(512);
	check("malloc(512) avant shrink != NULL", p != NULL);
	if (!p) return;
	for (int i = 0; i < 512; i++) p[i] = (char)i;
	dump("avant shrink (512)");

	char	*s = realloc(p, 128);
	check("realloc(512→128) != NULL", s != NULL);
	dump("apres shrink (→128)");
	if (s)
	{
		int ok = 1;
		for (int i = 0; i < 128; i++)
			if (s[i] != (char)i) { ok = 0; break; }
		check("shrink SMALL : 128 premiers octets preserves", ok);
	}
	free(s);

	/* LARGE 8192 → 2048 */
	char	*l = malloc(8192);
	check("malloc(8192) avant shrink LARGE != NULL", l != NULL);
	if (!l) return;
	memset(l, 0xCD, 8192);
	dump("avant shrink LARGE (8192)");

	char	*sl = realloc(l, 2048);
	check("realloc(8192→2048) != NULL", sl != NULL);
	dump("apres shrink LARGE (→2048)");
	if (sl)
	{
		int ok = 1;
		for (int i = 0; i < 2048; i++)
			if ((unsigned char)sl[i] != 0xCD) { ok = 0; break; }
		check("shrink LARGE : donnees preservees", ok);
		free(sl);
	}
}

/* ─────────────────────────────────────────────
   5. realloc — grow
   ───────────────────────────────────────────── */
static void	test_realloc_grow(void)
{
	print_section("5. realloc — grow");

	/* TINY 32 → SMALL 512 */
	char	*p = malloc(32);
	check("malloc(32) avant grow != NULL", p != NULL);
	if (!p) return;
	for (int i = 0; i < 32; i++) p[i] = (char)(i * 3);
	dump("avant grow TINY→SMALL (32)");

	char	*g = realloc(p, 512);
	check("realloc(32→512) TINY→SMALL != NULL", g != NULL);
	dump("apres grow (→512)");
	if (g)
	{
		int ok = 1;
		for (int i = 0; i < 32; i++)
			if (g[i] != (char)(i * 3)) { ok = 0; break; }
		check("grow TINY→SMALL : donnees originales preservees", ok);
		memset(g + 32, 0xFF, 512 - 32);
		check("grow TINY→SMALL : ecriture dans la zone etendue sans crash", 1);
	}
	free(g);

	/* TINY 32 → LARGE 4096 */
	char	*t = malloc(32);
	if (t) memset(t, 0x11, 32);
	dump("avant grow TINY→LARGE (32)");

	char	*tl = realloc(t, 4096);
	check("realloc(32→4096) TINY→LARGE != NULL", tl != NULL);
	dump("apres grow TINY→LARGE (→4096)");
	if (tl)
	{
		int ok = 1;
		for (int i = 0; i < 32; i++)
			if ((unsigned char)tl[i] != 0x11) { ok = 0; break; }
		check("grow TINY→LARGE : donnees preservees", ok);
		free(tl);
	}

	/* SMALL 256 → LARGE 8192 */
	char	*sm = malloc(256);
	if (sm) for (int i = 0; i < 256; i++) sm[i] = (char)i;
	dump("avant grow SMALL→LARGE (256)");

	char	*lg = realloc(sm, 8192);
	check("realloc(256→8192) SMALL→LARGE != NULL", lg != NULL);
	dump("apres grow SMALL→LARGE (→8192)");
	if (lg)
	{
		int ok = 1;
		for (int i = 0; i < 256; i++)
			if (lg[i] != (char)i) { ok = 0; break; }
		check("grow SMALL→LARGE : donnees preservees", ok);
		free(lg);
	}
}

/* ─────────────────────────────────────────────
   Main
   ───────────────────────────────────────────── */
int	main(void)
{
	setbuf(stdout, NULL);

	printf(CYAN "\n╔══════════════════════════════════════╗\n");
	printf(     "║        MALLOC TEST SUITE             ║\n");
	printf(     "╚══════════════════════════════════════╝" RESET "\n");

	test_edge_cases();
	test_coalescence();
	test_realloc_edge();
	test_realloc_shrink();
	test_realloc_grow();

	printf(CYAN "\n══════════════════════════════════════\n" RESET);
	printf("  " GREEN "PASS : %d" RESET "   " RED "FAIL : %d" RESET "\n", g_pass, g_fail);
	printf(CYAN "══════════════════════════════════════\n\n" RESET);

	return g_fail > 0 ? 1 : 0;
}
