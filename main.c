#include <stdio.h>
#include <string.h>

#define MM_IMPLEMENT
#include "mmlib/vector.h"
#include "mmlib/map.h"
#include "mmlib/heap.h"
#include "mmlib/graph.h"

#define MOVE_IDS \
	X(UP),       \
	X(DOWN),     \
	X(WEST),     \
	X(EAST),     \
	X(NORTH),    \
	X(SOUTH)

typedef enum {
#define X(x) x
MOVE_IDS
#undef X
} move_t;

char const *const move_str[] = {
#define X(x) #x
MOVE_IDS
#undef X
};


#define A_IDX 0
#define B_IDX 1
#define C_IDX 2
#define D_IDX 3
#define CASE_IDX 4
typedef struct {
	int base[4];
} board_state;

#define Z_SZ 11
#define Y_SZ 16
#define X_SZ 16
#define X_STRIDE 1
#define Y_STRIDE (X_SZ * X_STRIDE)
#define Z_STRIDE (Y_SZ * Y_STRIDE)
typedef char board_map[Z_SZ][Y_SZ][X_SZ]; //Z, Y, X

#define TIMES_2(x) x x
#define TIMES_4(x) x x x x
#define TIMES_8(x) x x x x x x x x
#define TIMES_12(x) TIMES_8(x) TIMES_4(x)
#define TIMES_16(x) TIMES_4(TIMES_4(x))

char const EMPTY_MAP[] = 
TIMES_4(
	TIMES_16("                ")
)
	TIMES_4("################")
	TIMES_8("####        ####")
	TIMES_4("################")

	TIMES_4("################")
	TIMES_2("####        ####")
	TIMES_4("####          ##")
	TIMES_2("####        ####")
	TIMES_4("################")

TIMES_2(
	 TIMES_2("################")
	TIMES_12("##            ##")
	 TIMES_2("################")
)
TIMES_2(
	TIMES_2("################")
	TIMES_8("##            ##")
	TIMES_2("##          ####")
	TIMES_2("##            ##")
	TIMES_2("################")
)
	TIMES_16("################")
;

int const A_offsets[] = {1092,1093,1094,1095,1108,1109,1110,1111,1124,1125,1126,1127,1140,1141,1142,1143,1348,1349,1350,1351,1352,1364,1365,1366,1367,1368,1380,1381,1382,1383,1384,1396,1397,1398,1399,1400,1604,1605,1620,1621,1860,1861,1876,1877,2082,2083,2084,2085,2086,2087,2098,2099,2100,2101,2102,2103,2114,2115,2116,2117,2118,2119,2130,2131,2132,2133,2134,2135,2148,2149,2150,2151,2164,2165,2166,2167,2338,2339,2340,2341,2342,2343,2354,2355,2356,2357,2358,2359,2370,2371,2372,2373,2374,2375,2386,2387,2388,2389,2390,2391,2404,2405,2406,2407,2420,2421,2422,2423};
int const B_offsets[] = {1096,1097,1098,1099,1112,1113,1114,1115,1128,1129,1130,1131,1144,1145,1146,1147,1353,1354,1355,1369,1370,1371,1385,1386,1387,1401,1402,1403,1610,1611,1626,1627,1866,1867,1882,1883,2090,2091,2106,2107,2120,2121,2122,2123,2124,2125,2136,2137,2138,2139,2140,2141,2154,2155,2156,2157,2170,2171,2172,2173,2346,2347,2362,2363,2376,2377,2378,2379,2380,2381,2392,2393,2394,2395,2396,2397,2410,2411,2412,2413,2426,2427,2428,2429};
int const C_offsets[] = {1156,1157,1158,1159,1172,1173,1174,1175,1188,1189,1190,1191,1204,1205,1206,1207,1412,1413,1414,1415,1428,1429,1430,1431,1444,1445,1446,1447,1460,1461,1462,1463,1700,1701,1716,1717,1956,1957,1972,1973,2178,2179,2180,2181,2182,2183,2194,2195,2196,2197,2198,2199,2210,2211,2212,2213,2214,2215,2226,2227,2228,2229,2230,2231,2244,2245,2260,2261,2434,2435,2436,2437,2438,2439,2450,2451,2452,2453,2454,2455,2466,2467,2468,2469,2470,2471,2482,2483,2484,2485,2486,2487,2500,2501,2516,2517};
int const D_offsets[] = {1160,1161,1162,1163,1176,1177,1178,1179,1192,1193,1194,1195,1208,1209,1210,1211,1416,1417,1418,1419,1432,1433,1434,1435,1448,1449,1450,1451,1464,1465,1466,1467,1706,1707,1722,1723,1963,1979,2184,2185,2186,2187,2200,2201,2202,2203,2216,2217,2218,2219,2232,2233,2234,2235,2246,2247,2248,2249,2250,2251,2252,2253,2262,2263,2264,2265,2266,2267,2268,2269,2440,2441,2442,2443,2456,2457,2458,2459,2472,2473,2474,2475,2488,2489,2490,2491,2502,2503,2504,2505,2506,2507,2508,2509,2518,2519,2520,2521,2522,2523,2524,2525};

#define A_offsets_len (sizeof(A_offsets)/sizeof(*A_offsets))
#define B_offsets_len (sizeof(B_offsets)/sizeof(*B_offsets))
#define C_offsets_len (sizeof(C_offsets)/sizeof(*C_offsets))
#define D_offsets_len (sizeof(D_offsets)/sizeof(*D_offsets))

int const *const offsets[] = {A_offsets, B_offsets, C_offsets, D_offsets};
int const lens[] = {A_offsets_len, B_offsets_len, C_offsets_len, D_offsets_len};

void make_map(board_map *dest, board_state const* state) {
	memcpy(dest, EMPTY_MAP, sizeof(board_map));
	int i;
	for (i = A_IDX; i <= D_IDX; i++) {
		if (state->base[i] <= -5*Z_STRIDE) continue;
		int j;
		for (j = 0; j < lens[i]; j++)
			if (offsets[i][j] + state->base[i] >= 0) break;
		for (; j < lens[i]; j++) {
			int idx = offsets[i][j] + state->base[i];
			(*dest)[0][0][idx] = 'A' + i;
		}
	}
}

void print_map(board_map const* m) {
	int i, j, k;
	for (i = 0; i < Z_SZ; i++) {
		printf("Z = %d\n", i);
		for (j = 0; j < Y_SZ; j++) {
			for (k = 0; k < X_SZ; k++) {
				printf("%c", (*m)[i][j][k]);
			}
			puts("");
		}
		puts("");
	}
}

typedef struct {
	move_t dir;
	char moveset[4]; //0 <=> A, 1 <=> B, etc.
} move;

typedef struct path_step {
	struct path_step *parent;
	move m;
	int cost;

	board_state state;
} path_step;

static int char_to_idx(char c) {
	switch(c) {
		case 'A': return A_IDX;
		case 'B': return B_IDX;
		case 'C': return C_IDX;
		case 'D': return D_IDX;
		case '#': return CASE_IDX;
		default: return -1;
	}
}

void get_legal_moves(board_state const* state, VECTOR_PTR_PARAM(move, dest)) {
	vector_clear(*dest);

	board_map m;
	make_map(&m, state);

	//Create three adjacency matrices, one for each axis. We don't
	//need one for each direction because X is_above Y implies 
	//Y is_below X 

	//Columns are each piece, ABCD. Rows are ABCD and the fifth is the casing.
	//A value of one in position is_above[i][j] implies piece j is above piece i
	char is_above[5][5];
	char is_west[5][5];
	char is_north[5][5];
	memset(is_above, 0, sizeof(is_above));
	memset(is_west, 0, sizeof(is_west));
	memset(is_north, 0, sizeof(is_north));

	int i, j, k;
	for (i = 0; i < Z_SZ - 1; i++) {
		for (j = 0; j < Y_SZ - 1; j++) {
			for (k = 0; k < X_SZ - 1; k++) {
				int cur = char_to_idx(m[i][j][k]);
				if (cur >= 0) {
					int below = char_to_idx(m[i+1][j][k]);
					if (below >= 0) is_above[cur][below] = 1;

					int south = char_to_idx(m[i][j+1][k]);
					if (south >= 0) is_north[cur][south] = 1;

					int east = char_to_idx(m[i][j][k+1]);
					if (east >= 0) is_west[cur][east] = 1;
				}
			}
		}
	}
	
	char is_above_closure[5][5];
	char is_west_closure[5][5];
	char is_north_closure[5][5];
	transitive_closure(is_above_closure[0], is_above[0], 5);
	transitive_closure(is_west_closure[0], is_west[0], 5);
	transitive_closure(is_north_closure[0], is_north[0], 5);
	

	//Now, for each direction, compute which pieces can go 
	//that way. The only thing that prevents movement is the 
	//casing. However, multiple pieces can (and sometimes must)
	//move at the same time. For example,
	//     (North)
	//  AAAAAAAAAAAAAA
	//  AAA BBBBBBBBBBB
	//  AAAAAAAAAAA BBB
	//   BBBBBBBBBBBBBB
	//
	//Neither A nor B can move North alone, but it is valid to 
	//move both.

	int piece;
	for (piece = 0; piece < 4; piece++) {
		if (state->base[piece] <= -5*Z_STRIDE) continue;

		//Up. Check columns of is_above. 
		if (!is_above_closure[4][piece]) {
			move *res = vector_lengthen(*dest);
			res->dir = UP;
			memset(&res->moveset, 0, sizeof(res->moveset));
			for (j = 0; j < 4; j++) {
				if (is_above_closure[j][piece]) res->moveset[j] = 1;
			}
		}
		//Down. Check rows of is_above.
		if (!is_above_closure[piece][4]) {
			move *res = vector_lengthen(*dest);
			res->dir = DOWN;
			memset(&res->moveset, 0, sizeof(res->moveset));
			for (j = 0; j < 4; j++) {
				if (is_above_closure[piece][j]) res->moveset[j] = 1;
			}
		}
		//North. Check columns of is_north. 
		if (!is_north_closure[4][piece]) {
			move *res = vector_lengthen(*dest);
			res->dir = NORTH;
			memset(&res->moveset, 0, sizeof(res->moveset));
			for (j = 0; j < 4; j++) {
				if (is_north_closure[j][piece]) res->moveset[j] = 1;
			}
		}
		//South. Check rows of is_north.
		if (!is_north_closure[piece][4]) {
			move *res = vector_lengthen(*dest);
			res->dir = SOUTH;
			memset(&res->moveset, 0, sizeof(res->moveset));
			for (j = 0; j < 4; j++) {
				if (is_north_closure[piece][j]) res->moveset[j] = 1;
			}
		}
		//West. Check columns of is_west. 
		if (!is_west_closure[4][piece]) {
			move *res = vector_lengthen(*dest);
			res->dir = WEST;
			memset(&res->moveset, 0, sizeof(res->moveset));
			for (j = 0; j < 4; j++) {
				if (is_west_closure[j][piece]) res->moveset[j] = 1;
			}
		}
		//East. Check rows of is_west.
		if (!is_west_closure[piece][4]) {
			move *res = vector_lengthen(*dest);
			res->dir = EAST;
			memset(&res->moveset, 0, sizeof(res->moveset));
			for (j = 0; j < 4; j++) {
				if (is_west_closure[piece][j]) res->moveset[j] = 1;
			}
		}
	}
}

void print_moves(VECTOR_PTR_PARAM(move, moves), int reverse) {
	int i;
	int start = reverse ? *moves_len - 1 : 0;
	int end = reverse ? -1 : *moves_len;
	int incr = reverse ? -1 : 1;
	for (i = start; i != end; i += incr) {
		printf("%03d. Move ", reverse ? ((int)*moves_len - i) : (i+1));
		char const *delim = "";
		int j;
		for (j = 0; j < 4; j++) {
			if ((*moves)[i].moveset[j]) {
				printf("%s%c", delim, j + 'A');
				delim = ", ";
			}
		}
		printf(" %s\n", move_str[(*moves)[i].dir]);
	}
}

//Really only useful in a single context, see main
static void undo_solution(VECTOR_PTR_PARAM(move, moves)) {
	int i;
	for (i = 0; i < *moves_len; i++) {
		printf("%03d. Move ", i+1);
		char const *delim = "";
		int j;
		for (j = 0; j < 4; j++) {
			if ((*moves)[i].moveset[j]) {
				printf("%s%c", delim, j + 'A');
				delim = ", ";
			}
		}

		switch((*moves)[i].dir) {
			case UP:
				puts(" DOWN"); break;
			case DOWN:
				puts(" UP"); break;
			case NORTH:
				puts(" SOUTH"); break;
			case SOUTH:
				puts(" NORTH"); break;
			case WEST:
				puts(" EAST"); break;
			case EAST:
				puts(" WEST"); break;
		}
	}
}

static int dir_to_offset(move_t dir) {
	switch(dir) {
		case UP: return -Z_STRIDE;
		case DOWN: return Z_STRIDE;
		case WEST: return -X_STRIDE;
		case EAST: return X_STRIDE;
		case NORTH: return -Y_STRIDE;
		case SOUTH: return Y_STRIDE;
	}
}

//Dest and src may alias
void apply_move(move const* m, board_state *dest, board_state const *src) {
	int i;
	for (i = 0; i < 4; i++) {
		if (m->moveset[i]) dest->base[i] = src->base[i] + dir_to_offset(m->dir);
		else dest->base[i] = src->base[i];
	}
}

int ptr_path_step_compar(void const *a, void const *b) {
	path_step const * const* pa = a;
	path_step const * const* pb = b;

	return (*pa)->cost - (*pb)->cost;
}

int main(void) {
	VECTOR_DECL(path_step *, search_nodes);
	vector_init(search_nodes);

	//Kind of by accident, a zero-initialized path step 
	//is exactly what I need for the initial one
	path_step *first = calloc(1, sizeof(path_step));
	/*first->state.base[0] = -1*Z_STRIDE;
	first->state.base[1] = -1*Z_STRIDE;
	first->state.base[2] =  0;
	first->state.base[3] =  0;*/
	vector_push(search_nodes, first);

	VECTOR_DECL(move, legal_moves);
	vector_init(legal_moves);

	VECTOR_DECL(path_step *, free_us);
	vector_init(free_us);

	map seen;
	map_init(&seen, board_state*, int, PTR2VAL);

#define MAX_ITERS 100000
	int iters = 0;
	while (iters++ < MAX_ITERS) {
		path_step *top;
		vector_heap_pop(search_nodes, &top, &ptr_path_step_compar);
		if (!top) {
			puts("ERROR: nothing left in moves");
			break;
		}
		vector_push(free_us, top);

		board_state *state = &top->state;
		int solved = 1;
		int i;
		for (i = 0; i < 4; i++) {
			if (state->base[i] > -5*Z_STRIDE) {
				solved = 0;
				break;
			}
		}
		if (solved) {
			VECTOR_DECL(move, solution);
			vector_init(solution);

			path_step *cur = top;
			while (cur) {
				vector_push(solution, cur->m);
				cur = cur->parent;
			}

			print_moves(VECTOR_ARG(solution), 1);

			puts("\nAnd now to undo it:");
			undo_solution(VECTOR_ARG(solution));

			vector_free(solution);
			break;
		}
		get_legal_moves(state, VECTOR_ARG(legal_moves));
		for (i = 0; i < legal_moves_len; i++) {
			path_step *to_insert = malloc(sizeof(path_step));
			to_insert->parent = top;
			to_insert->cost = top->cost + 1;
			to_insert->m = legal_moves[i];
			apply_move(&legal_moves[i], &to_insert->state, &top->state);

			int *prev_cost = map_search(&seen, &to_insert->state);
			if (!prev_cost || *prev_cost > to_insert->cost) {
				map_insert(&seen, &to_insert->state, 0, &to_insert->cost, 0);
				vector_heap_insert(search_nodes, &to_insert, &ptr_path_step_compar);
			} else {
				free(to_insert); //TODO: we could avoid this extra malloc/free pair
			}
		}
	}

	if (iters >= MAX_ITERS) {
		puts("Did not find a solution");
	}

	map_free(&seen);
	int i;
	for (i = 0; i < search_nodes_len; i++) free(search_nodes[i]);
	for (i = 0; i < free_us_len; i++) free(free_us[i]);
	vector_free(free_us);
	vector_free(search_nodes);
	vector_free(legal_moves);

	return 0;
}