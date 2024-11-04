/*
*  display.c:
* 화면에 게임 정보를 출력
* 맵, 커서, 시스템 메시지, 정보창, 자원 상태 등등
* io.c에 있는 함수들을 사용함
*/

#include "display.h"
#include "io.h"
#include <time.h> // 커서 이동 함수용

// 출력할 내용들의 좌상단(topleft) 좌표
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };


char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], 
	int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);

// 임시방편으로 위치 지정 변수 정의
POSITION pos = { MAP_HEIGHT + 2, 0 };

// 아트레디이스, 하코넨 등 색상을 구분할 배열 정의
int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

void display_system_message(POSITION pos, const char* system_message);
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos);
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], 
	int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);


void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor)
{
	display_resource(resource);
	display_map(map, set_col_map);
	display_cursor(cursor);

	display_system_message(pos, system_message);
	display_object_info(map, cursor, resource_pos);
	display_commands(map);

	// init_map(map, set_col_map);
}

void display_resource(RESOURCE resource) {
	set_color(COLOR_RESOURCE);
	gotoxy(resource_pos);
	printf("spice = %d/%d, population=%d/%d\n",
		resource.spice, resource.spice_max,
		resource.population, resource.population_max
	);
}

// subfunction of draw_map()
void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			for (int k = 0; k < N_LAYER; k++) {
				if (src[k][i][j] >= 0) {
					dest[i][j] = src[k][i][j];
				}
			}
		}
	}
}

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	project(map, backbuf);

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j]) {
				POSITION pos = { i, j };

				// 색상 적용
				int set_col = set_col_map[0][i][j];
				if (set_col_map[1][i][j] == COLOR_SANDWORM) {
					set_col = set_col_map[1][i][j];
				}
				set_color(set_col);

				printc(padd(map_pos, pos), backbuf[i][j], set_col);
			}
			frontbuf[i][j] = backbuf[i][j];
		}
	}
	set_color(COLOR_DEFAULT);

	// 맵 테두리 '#'
	/*
	for (int i = 0; i < MAP_HEIGHT; i++) {
		set_color(COLOR_DEFAULT);
		map[1][i][0] = '#';
		map[1][i][MAP_WIDTH - 1] = '#';
		for (int j = 0; j < MAP_WIDTH; j++){
			map[1][0][j] = '#';
			map[1][MAP_HEIGHT - 1][j] = '#';
		}
	}
	*/

	// 사막: 기본 지형(빈 칸)
	/*
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[0][i][j] = ' ';
		}
	}
	*/

	// 건물, 지형(B, P, S, R)은 layer 0
	// 유닛(H, W)은 layer 1

	// 아트레이디스 본진, 장판, 하베스터
	map[0][MAP_HEIGHT - 3][1] = 'B';
	map[0][MAP_HEIGHT - 3][2] = 'B';
	map[0][MAP_HEIGHT - 2][1] = 'B';
	map[0][MAP_HEIGHT - 2][2] = 'B';
	set_col_map[0][MAP_HEIGHT - 3][1] = COLOR_ATREIDES;
	set_col_map[0][MAP_HEIGHT - 3][2] = COLOR_ATREIDES;
	set_col_map[0][MAP_HEIGHT - 2][1] = COLOR_ATREIDES;
	set_col_map[0][MAP_HEIGHT - 2][2] = COLOR_ATREIDES;

	map[0][MAP_HEIGHT - 3][3] = 'P';
	map[0][MAP_HEIGHT - 3][4] = 'P';
	map[0][MAP_HEIGHT - 2][3] = 'P';
	map[0][MAP_HEIGHT - 2][4] = 'P';
	set_col_map[0][MAP_HEIGHT - 3][3] = COLOR_PLATE;
	set_col_map[0][MAP_HEIGHT - 3][4] = COLOR_PLATE;
	set_col_map[0][MAP_HEIGHT - 2][3] = COLOR_PLATE;
	set_col_map[0][MAP_HEIGHT - 2][4] = COLOR_PLATE;

	map[1][MAP_HEIGHT - 4][1] = 'H';
	set_col_map[1][MAP_HEIGHT - 4][1] = COLOR_ATREIDES;

	// 하코넨 본진, 장판, 하베스터
	map[0][1][MAP_WIDTH - 3] = 'B';
	map[0][2][MAP_WIDTH - 3] = 'B';
	map[0][1][MAP_WIDTH - 2] = 'B';
	map[0][2][MAP_WIDTH - 2] = 'B';
	set_col_map[0][1][MAP_WIDTH - 3] = COLOR_HARKONNEN;
	set_col_map[0][2][MAP_WIDTH - 3] = COLOR_HARKONNEN;
	set_col_map[0][1][MAP_WIDTH - 2] = COLOR_HARKONNEN;
	set_col_map[0][2][MAP_WIDTH - 2] = COLOR_HARKONNEN;

	map[0][1][MAP_WIDTH - 4] = 'P';
	map[0][2][MAP_WIDTH - 4] = 'P';
	map[0][1][MAP_WIDTH - 5] = 'P';
	map[0][2][MAP_WIDTH - 5] = 'P';
	set_col_map[0][1][MAP_WIDTH - 4] = COLOR_PLATE;
	set_col_map[0][2][MAP_WIDTH - 4] = COLOR_PLATE;
	set_col_map[0][1][MAP_WIDTH - 5] = COLOR_PLATE;
	set_col_map[0][2][MAP_WIDTH - 5] = COLOR_PLATE;

	map[1][3][MAP_WIDTH - 2] = 'H';
	set_col_map[1][3][MAP_WIDTH - 2] = COLOR_HARKONNEN;

	// 스파이스
	map[0][MAP_HEIGHT - 4][3] = 'S';
	map[0][3][MAP_WIDTH - 4] = 'S';
	set_col_map[0][MAP_HEIGHT - 4][3] = COLOR_SPICE;
	set_col_map[0][3][MAP_WIDTH - 4] = COLOR_SPICE;

	// 샌드웜
	map[1][13][24] = 'W';
	map[1][7][56] = 'W';
	set_col_map[1][13][24] = COLOR_SANDWORM;
	set_col_map[1][7][56] = COLOR_SANDWORM;

	// 바위
	map[0][5][5] = 'R';
	map[0][6][6] = 'R';
	map[0][6][7] = 'R';
	map[0][7][6] = 'R';
	map[0][7][7] = 'R';
	set_col_map[0][5][5] = COLOR_ROCK;
	set_col_map[0][6][6] = COLOR_ROCK;
	set_col_map[0][6][7] = COLOR_ROCK;
	set_col_map[0][7][6] = COLOR_ROCK;
	set_col_map[0][7][7] = COLOR_ROCK;
}

// frontbuf[][]에서 커서 위치의 문자를 색만 바꿔서 그대로 다시 출력
void display_cursor(CURSOR cursor) {
	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	char ch = frontbuf[prev.row][prev.column];
	printc(padd(map_pos, prev), ch, COLOR_DEFAULT);

	ch = frontbuf[curr.row][curr.column];
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);
}

// 1) 준비 - 시스템 메시지 표시 함수 + 초기 메시지
char system_message[200] = "Waiting for the command... "; // 초기 메시지를 임의로 설정

void display_system_message(POSITION pos, const char* system_message) {
	gotoxy(pos);
	printf("System: %s\n", system_message);
}

// 1) 준비 - 상태창 함수
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos) {
	char selected = map[1][cursor.current.row][cursor.current.column];
	gotoxy(resource_pos);
	printf("Selected Object>> %c\n", selected);
}

// 1) 준비 - 명령창 함수
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 2 };
	gotoxy(command_pos);
	printf("Choose the commands number>> 1. Move 2. Attack 3. Harvest\n");
}

// 1) 준비 - 초기 상태 초기 배치 함수
/*
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	// 사막: 기본 지형(빈 칸)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[0][i][j] = ' ';
		}
	}

	// 건물, 지형(B, P, S, R)은 layer 0
	// 유닛(H, W)은 layer 1
	
	// 아트레이디스 본진, 장판, 하베스터
	map[0][MAP_HEIGHT - 3][1] = 'B';
	map[0][MAP_HEIGHT - 3][2] = 'B';
	map[0][MAP_HEIGHT - 2][1] = 'B';
	map[0][MAP_HEIGHT - 2][2] = 'B';
	set_col_map[0][MAP_HEIGHT - 3][1] = COLOR_ATREIDES;
	set_col_map[0][MAP_HEIGHT - 3][2] = COLOR_ATREIDES;
	set_col_map[0][MAP_HEIGHT - 2][1] = COLOR_ATREIDES;
	set_col_map[0][MAP_HEIGHT - 2][2] = COLOR_ATREIDES;

	map[0][MAP_HEIGHT - 3][3] = 'P';
	map[0][MAP_HEIGHT - 3][4] = 'P';
	map[0][MAP_HEIGHT - 2][3] = 'P';
	map[0][MAP_HEIGHT - 2][4] = 'P';
	set_col_map[0][MAP_HEIGHT - 3][3] = COLOR_PLATE;
	set_col_map[0][MAP_HEIGHT - 3][4] = COLOR_PLATE;
	set_col_map[0][MAP_HEIGHT - 2][3] = COLOR_PLATE;
	set_col_map[0][MAP_HEIGHT - 2][4] = COLOR_PLATE;

	map[1][MAP_HEIGHT - 4][1] = 'H';
	set_col_map[1][MAP_HEIGHT - 4][1] = COLOR_ATREIDES;

	// 하코넨 본진, 장판, 하베스터
	map[0][1][MAP_WIDTH - 3] = 'B';
	map[0][2][MAP_WIDTH - 3] = 'B';
	map[0][1][MAP_WIDTH - 2] = 'B';
	map[0][2][MAP_WIDTH - 2] = 'B';
	set_col_map[0][1][MAP_WIDTH - 3] = COLOR_HARKONNEN;
	set_col_map[0][2][MAP_WIDTH - 3] = COLOR_HARKONNEN;
	set_col_map[0][1][MAP_WIDTH - 2] = COLOR_HARKONNEN;
	set_col_map[0][2][MAP_WIDTH - 2] = COLOR_HARKONNEN;

	map[0][1][MAP_WIDTH - 4] = 'P';
	map[0][2][MAP_WIDTH - 4] = 'P';
	map[0][1][MAP_WIDTH - 5] = 'P';
	map[0][2][MAP_WIDTH - 5] = 'P';
	set_col_map[0][1][MAP_WIDTH - 4] = COLOR_PLATE;
	set_col_map[0][2][MAP_WIDTH - 4] = COLOR_PLATE;
	set_col_map[0][1][MAP_WIDTH - 5] = COLOR_PLATE;
	set_col_map[0][2][MAP_WIDTH - 5] = COLOR_PLATE;

	map[1][3][MAP_WIDTH - 2] = 'H';
	set_col_map[1][3][MAP_WIDTH - 2] = COLOR_HARKONNEN;

	// 스파이스
	map[0][MAP_HEIGHT - 4][3] = 'S';
	map[0][3][MAP_WIDTH - 4] = 'S';
	set_col_map[0][MAP_HEIGHT - 4][3] = COLOR_SPICE;
	set_col_map[0][3][MAP_WIDTH - 4] = COLOR_SPICE;

	// 샌드웜
	map[1][13][24] = 'W';
	map[1][7][56] = 'W';
	set_col_map[1][13][24] = COLOR_SANDWORM;
	set_col_map[1][7][56] = COLOR_SANDWORM;

	// 바위
	map[0][5][5] = 'R';
	map[0][6][6] = 'R';
	map[0][6][7] = 'R';
	map[0][7][6] = 'R';
	map[0][7][7] = 'R';
	set_col_map[0][5][5] = COLOR_ROCK;
	set_col_map[0][6][6] = COLOR_ROCK;
	set_col_map[0][6][7] = COLOR_ROCK;
	set_col_map[0][7][6] = COLOR_ROCK;
	set_col_map[0][7][7] = COLOR_ROCK;
}
*/

// 2) 커서 & 상태창 - 방향키 입력 및 이동 함수
void move_cursor(KEY key) {
	DIRECTION dir;

	switch (key) {
		case k_up:
			dir = d_up;
			break;
		case k_down:
			dir = d_down;
			break;
		case k_left:
			dir = d_left;
			break;
		case k_right:
			dir = d_right;
			break;
		default:
			dir = d_stay;
			break;
	}
}

// 2) 커서 & 상태창 - 방향키 더블클릭 함수
void double_cursor(DIRECTION dir, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	static clock_t last_click = 0;
	clock_t cur_time = clock();

	// 방향키 더블 이동
	int step = 1;
	if ((cur_time - last_click) < CLOCKS_PER_SEC / 2) {
		step = 4;
	}
	last_click = cur_time;

	// 커서 이동
	for (int i = 0; i < step; i++) {
		cursor_move(dir);
	}

	// 객체 정보 업데이트
	display_object_info(map, *cursor, resource_pos);
}

// 2) 커서 & 상태창 - 선택 함수
void select_object(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	char selected = map[0][cursor.current.row][cursor.current.column];
	display_object_info(map, cursor, resource_pos);
	
	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };
	display_system_message(system_message_pos, "오브젝트 선택");
}

// 2) 커서 & 상태창 - 취소 함수
void deselect_object() {
	display_object_info(NULL, (CURSOR) { 0, 0 }, resource_pos);

	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };
	display_system_message(system_message_pos, "선택을 취소합니다.");
}