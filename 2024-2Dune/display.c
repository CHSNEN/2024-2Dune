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

// 2) 커서 & 상태창 - 방향키 이동 처리
POSITION pmove(POSITION cur, DIRECTION dir) {
	POSITION move_vector = dtop(dir);
	return padd(cur, move_vector);

	POSITION new_pos = cur;
	switch (dir) {
		case d_up:
			new_pos.row -= 1;
			break;
		case d_down:
			new_pos.row += 1;
			break;
		case d_left:
			new_pos.column -= 1;
			break;
		case d_right:
			new_pos.column += 1;
			break;
		default:
			break;
	}
	return new_pos;
}

char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);

void display_system_message(POSITION map_pos, const char* system_message);
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos);
void display_commands(POSITION map_pos);
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);


void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor)
{
	display_resource(resource);
	display_map(map);
	display_cursor(cursor);

	display_system_message(map_pos, system_message);
	display_object_info(map, cursor, resource_pos);
	display_commands(map_pos);

	init_map(map);
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

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	project(map, backbuf);

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j]) {
				POSITION pos = { i, j };
				printc(padd(map_pos, pos), backbuf[i][j], COLOR_DEFAULT);
			}
			frontbuf[i][j] = backbuf[i][j];
		}
	}
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

void display_system_message(POSITION map_pos, const char* system_message) {
	gotoxy(map_pos);
	printf("System: %s\n", system_message);
}

// 1) 준비 - 상태창 함수
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos) {
	char selected = map[1][cursor.current.row][cursor.current.column];
	gotoxy(resource_pos);
	printf("Selected Object>> %c\n", selected);
}

// 1) 준비 - 명령창 함수
void display_commands(POSITION map_pos) {
	gotoxy(map_pos);
	printf("Choose the commands number>> 1. Move  2. Attack  3. Harvest\n");
}

// 1) 준비 - 초기 상태 초기 배치 함수
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
	for (int i = MAP_HEIGHT - 3; i < MAP_HEIGHT - 1; i++) {
		for (int j = 1; j <= 2; j++) {
			if (i == MAP_HEIGHT - 3 && j == 1) {
				map[0][i][j] = 'B';
			}
			else {
				map[0][i][j] = 'P';
			}
		}
	}
	map[1][MAP_HEIGHT - 2][1] = 'H';

	// 하코넨 본진, 장판, 하베스터
	for (int i = 1; i <= 2; i++) {
		for (int j = MAP_WIDTH - 3; j < MAP_WIDTH - 1; j++) {
			if (i == 1 && j == MAP_WIDTH - 3) {
				map[0][i][j] = 'B';
			}
			else {
				map[0][i][j] = 'P';
			}
		}
	}
	map[1][MAP_HEIGHT][MAP_WIDTH - 2] = 'H';

	// 스파이스
	map[0][MAP_HEIGHT - 4][MAP_WIDTH - 2] = '5';

	// 샌드웜
	map[1][3][4] = 'W';
	map[1][7][6] = 'W';

	// 바위
	map[0][5][5] = 'R';
	map[0][6][6] = 'R';
	map[0][3][8] = 'R';
}

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
void double_cursor(DIRECTION dir, CURSOR* cursor) {
	static clock_t last_click = 0;
	clock_t cur_time = clock();

	// 방향키 더블 이동
	int step = 1;
	if ((cur_time - last_click) < CLOCKS_PER_SEC / 2) {
		step = 2;
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

	const char* system_message = "오브젝트 선택";
	display_system_message(system_message);
}

// 2) 커서 & 상태창 - 취소 함수
void deselect_object() {
	display_object_info(NULL, (CURSOR) { 0, 0 }, resource_pos);

	const char* system_mesage = "선택을 취소합니다.";
	display_system_message(system_message);
}