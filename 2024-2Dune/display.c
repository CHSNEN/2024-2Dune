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
const POSITION object_pos = { 0, MAP_WIDTH + 2 };

char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0, 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0, 0 };

// 5) 시스템 메시지
#define MAX_MESSAGES	10 
#define MESSAGE_LENGTH	200 

char system_messages[MAX_MESSAGES][MESSAGE_LENGTH] = { 0 }; 
int current_message_count = 0;  


void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], 
	int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);

// 위치 지정 변수 정의
const POSITION message_pos = { MAP_HEIGHT + 2, 0 };

// 아트레디이스, 하코넨 등 색상을 구분할 배열 정의
int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

// 4) 유닛 1기 생산 - Bonus 생산 시간
int make_building_time = -1;

// 색상 리셋
void reset_color() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_DEFAULT);
}

void display_system_message(POSITION pos, const char* system_message);
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION object_pos);
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void init_set_col_map(int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);

void handle_input(KEY get_key, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void sandworm_action(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], OBJECT_SAMPLE objects[MAX_OBJECTS]);
void init_build_command_pos(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], BUILD_COMMAND* command[MAX_COMMANDS]);
POSITION find_building_pos(KEY get_key, BUILD_COMMAND* command);
void handle_command_input(KEY get_key, BUILD_COMMAND* command);

void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor,
	OBJECT_SAMPLE objects[MAX_OBJECTS], 
	KEY get_key,
	BUILD_COMMAND command[MAX_COMMANDS])
{
	display_resource(resource);
	display_map(map, set_col_map);
	display_cursor(cursor);

	display_object_info(map, cursor, object_pos);
	display_commands(map);
	display_system_message(message_pos, system_message);

	init_set_col_map(set_col_map, map);

	handle_input(get_key, &cursor, map);

	sandworm_action(map, objects);

	init_build_command_pos(map, command);

	//POSITION building_idx_pos = find_building_pos(get_key, command);
	//if (building_idx_pos.row >= 0 && building_idx_pos.row < MAX_COMMANDS) {  // 유효한 명령어가 있을 때만 실행
		//handle_command_input(get_key, &command[building_idx_pos.row]);
	//}
	
}

void display_resource(RESOURCE resource) {
	set_color(COLOR_RESOURCE);
	gotoxy(resource_pos);
	printf("spice = %d/%d, population=%d/%d\n",
		resource.spice, resource.spice_max,
		resource.population, resource.population_max
	);
	reset_color();
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
	gotoxy(map_pos);
	init_set_col_map(set_col_map, map);
	project(map, backbuf);
	set_color(COLOR_DEFAULT);

	for (int i = 0; i < MAX_OBJECTS; i++) {
		gotoxy(objects[i].pos);
		printf("%c", objects[i].repr);
	}

	for (int i = 0; i < MAP_HEIGHT; i++) {
		// gotoxy((POSITION) { i + 1, 0 });
		for (int j = 0; j < MAP_WIDTH; j++) {
			gotoxy((POSITION) { i + map_pos.row, j + map_pos.column });
			char element = backbuf[i][j];
			int color = set_col_map[0][i][j];

			switch (element) {
				case '#':
					set_color(COLOR_DEFAULT);
					printf("#");
					break;
				case 'B':
					set_color(color == COLOR_ATREIDES ? COLOR_ATREIDES : COLOR_HARKONNEN);
					printf("B");
					break;
				case 'H':
					set_color(color == COLOR_ATREIDES ? COLOR_ATREIDES : COLOR_HARKONNEN);
					printf("H");
					break;
				case 'P':
					set_color(COLOR_PLATE);
					printf("P");
					break;
				case 'S':
					set_color(COLOR_SPICE);
					printf("S");
					break;
				case 'R':
					set_color(COLOR_ROCK);
					printf("R");
					break;
				default:
					printf(" ");
					break;
			}
		}
		reset_color();
		printf("\n");
	}
	memset(backbuf, 0, sizeof(backbuf));
	memset(frontbuf, 0, sizeof(frontbuf));
}

// frontbuf[][]에서 커서 위치의 문자를 색만 바꿔서 그대로 다시 출력
void display_cursor(CURSOR cursor) {
	/*
	gotoxy(map_pos);
	set_color(COLOR_CURSOR);

	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;
	*/

	/*
	gotoxy(prev);
	int prev_color = set_col_map[0][prev.row][prev.column];
	set_color(prev_color);
	printf("%c", frontbuf[prev.row][prev.column]);

	gotoxy(curr);
	printf("%c", frontbuf[curr.row][curr.column]);
	*/

	/*
	gotoxy(prev);
	char ch = frontbuf[prev.row][prev.column];
	printc(padd(map_pos, prev), ch, COLOR_CURSOR);

	gotoxy(curr);
	ch = frontbuf[curr.row][curr.column];
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);
	*/

	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	// 이전 위치 복구
	gotoxy(padd(map_pos, prev));
	set_color(set_col_map[0][prev.row][prev.column]); // 이전 위치 색상 복원
	printf("%c", frontbuf[prev.row][prev.column]);

	// 현재 위치 출력
	gotoxy(padd(map_pos, curr));
	set_color(COLOR_CURSOR);  // 커서 색상 설정
	printf(" ");  // 커서 위치 표시

	reset_color();  // 색상 초기화

}

char system_message[200] = "Waiting for the command... "; // 초기 메시지를 임의로 설정

// 5) 시스템 메시지 - 메시지 추가 함수
void add_system_message(const char* new_message) {
	if (current_message_count >= MAX_MESSAGES) {
		for (int i = 1; i < MAX_MESSAGES; i++) {
			strcpy_s(system_messages[i - 1], sizeof(system_messages[i - 1]), system_messages[i]);
		}
		current_message_count = MAX_MESSAGES - 1;
	}

	strcpy_s(system_messages[current_message_count], sizeof(system_messages[current_message_count]), new_message);
	current_message_count++;
}


// 1) 준비 - 시스템 메시지 표시 함수 + 초기 메시지
void display_system_message(POSITION message_pos, const char* system_message) {
	gotoxy(message_pos);
	static char last_message[200] = "";
	static int line = 0;

	// 5) 시스템 메시지 - 형식: 메시지 로그
	if (strcmp(last_message, system_message) != 0) {
		strcpy_s(last_message, sizeof(last_message), system_message);
		gotoxy((POSITION) { message_pos.row + line, message_pos.column });
		printf("System: %s", system_message);
		
		line++;

		if (line >= 5) {
			line = 0;
		}
		set_color(COLOR_DEFAULT);
	}

	// 5) 시스템 메시지 - 메시지 출력 기능 추가
	/*
	for (int i = 0; i < MAX_MESSAGES; i++) {
		gotoxy((POSITION) { message_pos.row + i, message_pos.column });
		if (i < current_message_count) {
			printf("%s", system_messages[i]);
		}
		else {
			printf(" ");
		}
	}

	add_system_message(system_message);
	*/
	reset_color();
}

// 1) 준비 - 상태창 함수
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION object_pos) {
	gotoxy(object_pos);
	char selected = map[1][cursor.current.row][cursor.current.column];

	printf("Selected Object>> %c \n", selected);

	// 4) Bonus) 생산 시간 표시
	if (make_building_time > 0) {
		printf("Production time: %d sec...\n", make_building_time);
		make_building_time--;
		if (make_building_time == 0) {
			display_system_message(message_pos, "Production is complete!");
		}
	}

	set_color(COLOR_DEFAULT);
}

// 1) 준비 - 명령창 함수
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 2 };
	gotoxy(command_pos);
	printf("Choose the commands number>> 1. Move 2. Attack 3. Harvest\n");
	// reset_color();
}

// 1) 준비 - 초기 상태 초기 배치 함수
OBJECT_SAMPLE objects[MAX_OBJECTS] = {
	// ATREIDES B
	{{MAP_HEIGHT - 3, 1},{MAP_HEIGHT - 3, 1}, 'B', 0, 0, 0, COLOR_ATREIDES, 1},
	{{MAP_HEIGHT - 3, 2},{MAP_HEIGHT - 3, 2}, 'B', 0, 0, 0, COLOR_ATREIDES, 1},
	{{MAP_HEIGHT - 2, 1},{MAP_HEIGHT - 2, 1}, 'B', 0, 0, 0, COLOR_ATREIDES, 1},
	{{MAP_HEIGHT - 2, 2},{MAP_HEIGHT - 2, 2}, 'B', 0, 0, 0, COLOR_ATREIDES, 1},

	// ATREIDES P
	{{MAP_HEIGHT - 3, 3},{MAP_HEIGHT - 3, 3}, 'P', 0, 0, 0, COLOR_PLATE, 0},
	{{MAP_HEIGHT - 3, 4},{MAP_HEIGHT - 3, 4}, 'P', 0, 0, 0, COLOR_PLATE, 0},
	{{MAP_HEIGHT - 2, 3},{MAP_HEIGHT - 2, 3}, 'P', 0, 0, 0, COLOR_PLATE, 0},
	{{MAP_HEIGHT - 2, 4},{MAP_HEIGHT - 3, 4}, 'P', 0, 0, 0, COLOR_PLATE, 0},

	// ATREIDES H
	{{MAP_HEIGHT - 4, 1},{MAP_HEIGHT - 4, 1}, 'H', 0, 0, 0, COLOR_ATREIDES, 1},

	// HARKONNEN B
	{{1, MAP_WIDTH - 3}, {1, MAP_WIDTH - 3}, 'B', 0, 0, 0, COLOR_HARKONNEN, 2},
	{{2, MAP_WIDTH - 3}, {2, MAP_WIDTH - 3}, 'B', 0, 0, 0, COLOR_HARKONNEN, 2},
	{{1, MAP_WIDTH - 2}, {1, MAP_WIDTH - 2}, 'B', 0, 0, 0, COLOR_HARKONNEN, 2},
	{{2, MAP_WIDTH - 2}, {2, MAP_WIDTH - 2}, 'B', 0, 0, 0, COLOR_HARKONNEN, 2},

	// HARKONNEN P
	{{1, MAP_WIDTH - 4}, {1, MAP_WIDTH - 4}, 'P', 0, 0, 0, COLOR_PLATE, 0},
	{{2, MAP_WIDTH - 4}, {2, MAP_WIDTH - 4}, 'P', 0, 0, 0, COLOR_PLATE, 0},
	{{1, MAP_WIDTH - 5}, {1, MAP_WIDTH - 5}, 'P', 0, 0, 0, COLOR_PLATE, 0},
	{{2, MAP_WIDTH - 5}, {2, MAP_WIDTH - 5}, 'P', 0, 0, 0, COLOR_PLATE, 0},

	// HARKONNEN H
	{{3, MAP_WIDTH - 3}, {3, MAP_WIDTH - 3}, 'H', 0, 0, 0, COLOR_HARKONNEN, 2},

	// SPICE
	{{MAP_HEIGHT - 4, 3}, {MAP_HEIGHT - 4, 3}, 'S', 0, 0, 0, COLOR_SPICE, 0},
	{{3, MAP_WIDTH - 4}, {3, MAP_WIDTH - 4}, 'S', 0, 0, 0, COLOR_SPICE, 0},

	// SANDWORM
	{{13, 24}, {0, 0}, 'W', 1000, 0, 1, COLOR_SANDWORM, 0},
	{{7, 56}, {0, 0}, 'W', 1000, 0, 1, COLOR_SANDWORM, 0},

	// ROCK
	{{16, 42}, {16, 42}, 'R', 0, 0, 0, COLOR_ROCK, 0},
	{{16, 43}, {16, 43}, 'R', 0, 0, 0, COLOR_ROCK, 0},
	{{17, 42}, {17, 42}, 'R', 0, 0, 0, COLOR_ROCK, 0},
	{{17, 43}, {17, 43}, 'R', 0, 0, 0, COLOR_ROCK, 0},
	{{6, 6}, {6, 6}, 'R', 0, 0, 0, COLOR_ROCK, 0},
	{{6, 7}, {6, 7}, 'R', 0, 0, 0, COLOR_ROCK, 0},
	{{7, 6}, {7, 6}, 'R', 0, 0, 0, COLOR_ROCK, 0},
	{{7, 7}, {7, 7}, 'R', 0, 0, 0, COLOR_ROCK, 0}
};

int object_count = 30;
int unit_count = 4;
// 3), 4) 에서 사용될 현재 스파이스 매장지 업데이트용 변수
int current_spice = 2;


// 1) 준비 - 색상 적용 함수
void init_set_col_map(int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (i == 0 || i == MAP_HEIGHT - 1 || j == 0 || j == MAP_WIDTH - 1) {
				set_col_map[0][i][j] = COLOR_DEFAULT;
				map[0][i][j] = '#';
			}
		}
	}

	for (int i = 0; i < MAX_OBJECTS; i++) {
		int row = objects[i].pos.row;
		int col = objects[i].pos.column;

		if (objects[i].house == 1) {
			set_col_map[0][row][col] = COLOR_ATREIDES;
		}
		else if (objects[i].house == 2) {
			set_col_map[0][row][col] = COLOR_HARKONNEN;
		}
		else {
			set_col_map[0][row][col] = objects[i].color;
		}
	}
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
const int double_click_time = 100;

void double_cursor(DIRECTION dir, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	static clock_t last_click = 0;
	clock_t cur_time = clock();

	// 방향키 더블 이동
	int step = 1;
	if ((cur_time - last_click) < double_click_time / 2) {
		step = 4;
	}
	last_click = cur_time;

	// 커서 이동
	for (int i = 0; i < step; i++) {
		move_cursor(dir);
	}

	// 객체 정보 업데이트
	display_object_info(map, *cursor, resource_pos);
}

// 2) 커서 & 상태창 - 선택 함수
void select_object(CURSOR *cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION pos = cursor->current;
	char current_tile = map[1][pos.row][pos.column];
	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };

	switch (current_tile) {
		//case ' ' :
			//display_system_message(system_message_pos, "사막 지형을 선택하였습니다.");
			//break;
		case 'B' :
			display_system_message(system_message_pos, "본진을 선택하였습니다.");
			break;
		case 'H' :
			display_system_message(system_message_pos, "하베스터를 선택하였습니다.");
			break;
		case 'P':
			display_system_message(system_message_pos, "장판을 선택하였습니다.");
			break;
		case 'W':
			display_system_message(system_message_pos, "샌드웜을 선택하였습니다.");
			break;
		case 'S':
			display_system_message(system_message_pos, "스파이스 매장지를 선택하였습니다.");
			break;
		case 'R':
			display_system_message(system_message_pos, "바위를 선택하였습니다.");
			break;
		default :
			display_system_message(system_message_pos, "사막 지형을 선택했습니다.");
			break;
	}
	
}

// 2) 커서 & 상태창 - 취소 함수
void deselect_object() {
	display_object_info(NULL, (CURSOR) { 0, 0 }, resource_pos);
	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };
	display_system_message(system_message_pos, "");
}

// 2) 커서 & 상태창 - 커서 & 상태창 함수 종합적으로 호출
void handle_input(int key, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	DIRECTION dir;

	switch (key) {
	case k_up:
		dir = d_up;
		double_cursor(dir, cursor, map);
		break;
	case k_down:
		dir = d_down;
		double_cursor(dir, cursor, map);
		break;
	case k_left:
		dir = d_left;
		double_cursor(dir, cursor, map);
		break;
	case k_right:
		dir = d_right;
		double_cursor(dir, cursor, map); 
		break;
	case k_space:
		select_object(cursor, map); // 스페이스바로 오브젝트 선택
		break;
	case k_esc:
		deselect_object(); // ESC 키로 선택 취소
		break;
	case k_quit:
		outro(); // q 키 게임 종료
		exit(0);
		break;
	default:
		break;
	}
}

// 3) 중립 유닛 - 가까운 유닛 감지 함수
// OBJECT_SAMPLE sandworm = { {13, 24}, {0, 0}, 'W', 1000, 0, 1, COLOR_SANDWORM };

POSITION find_near_unit(POSITION sandworm_pos) {
	POSITION near_unit = { -1, -1 };
	int min_distance = MAP_HEIGHT + MAP_WIDTH;

	for (int i = 1; i < unit_count; i++) {
		if (objects[i].repr == 'W')
			continue;

		int distance = abs(sandworm_pos.row - objects[i].pos.row)
			+ abs(sandworm_pos.column - objects[i].pos.column);
		
		if (distance < min_distance) {
			min_distance = distance;
			near_unit = objects[i].pos;
		}
	}
	return near_unit;
}

// 3) 중립 유닛 - 샌드웜 near_unit으로 이동 함수
void move_to_near(OBJECT_SAMPLE* sandworm, POSITION target) {
	if (sandworm->pos.row < target.row) {
		sandworm->pos.row++;
	}
	else if (sandworm->pos.row > target.row) {
		sandworm->pos.row--;
	}
	if (sandworm->pos.column < target.column) {
		sandworm->pos.column++;
	}
	else if (sandworm->pos.column > target.column) {
		sandworm->pos.column--;
	}

	// printf("Sandworm moved to row: %d, column: %d\n", sandworm->pos.row, sandworm->pos.column);
}

// 3) 중립 유닛 - 유닛 잡아먹기 함수
void eat_unit(OBJECT_SAMPLE* sandworm) {
	for (int i = 0; i < unit_count; i++) {
		if (sandworm->pos.row == objects[i].pos.row && sandworm->pos.column == objects[i].pos.column) {
			if (objects[i].repr != 'W') {
				for (int j = i; j < unit_count - 1;j++) {
					objects[j] = objects[j + 1];
				}
				unit_count--;

				// 5) 시스템 메시지
				display_system_message(message_pos, "샌드웜이 유닛을 잡아먹었습니다!\n");
				break;
			}
		}
	}
}

// 3) 중립 유닛 - 샌드웜 배설 함수
void make_spice(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION spice_pos = { rand() % MAP_HEIGHT, rand() % MAP_WIDTH };
	map[1][spice_pos.row][spice_pos.column] = 'S';
	current_spice++;

	// 5) 시스템 메시지
	display_system_message(message_pos, "샌드웜이 배설하여 스파이스 매장지가 생성되었습니다!\n");
}

// 3) 중립 유닛 - 샌드웜 행동 종합적으로 호출 함수
void sandworm_action(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], OBJECT_SAMPLE* sandworm) {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i].repr == 'W') {
			POSITION target = find_near_unit(objects[i].pos);
			move_to_near(&objects[i], target);

			bool ate_unit = false;
			if (objects[i].pos.row == target.row && objects[i].pos.column == target.column) {
				ate_unit = true;
				eat_unit(&objects[i]);
			}

			if (ate_unit && rand() % 50 == 0) { 
				make_spice(map);
				current_spice++;
			}
		}
	}
}

// 3) 중립 유닛 - 샌드웜 이동 업데이트 함수
void update_sandworms(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < unit_count; i++) {
		if (objects[i].repr == 'W') { 
			sandworm_action(map, &objects[i]);
		}
	}
}

// 4) 유닛 1기 생산, 7) 유닛 목록 구현 - 단축키 목록 초기화
BUILD_COMMAND command[] = {
	{ 'B', 'H', 0, "Harvester", 0, {-1, -1} },
	{ 'B', 'S', 4, "Soldier", 1, {-1, -1} },
	{ 'S', 'F', 5, "Fremen", 1, {-1, -1} },
	{ 'A','F', 3, "Fighter", 2, {-1, -1} },
	{ 'F', 'T', 5, "Heavy Tank", 2, {-1, -1} }
};

// 4) 유닛 1기 생산 - 위치 찾기 함수
POSITION find_building_pos(KEY get_key, BUILD_COMMAND* command) {
	POSITION building_pos = { -1, -1 };
	for (int i = 0; i < MAX_COMMANDS; i++) {
		if (command[i].command_k == get_key) {
			building_pos.row = command[i].pos.row;
			building_pos.column = command[i].pos.column;
			return building_pos;
		}
	}
	return building_pos;
}

// 4) 유닛 1기 생산 - 빈 공간 찾기 함수
POSITION find_empty_pos(POSITION find_building_pos, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION empty_pos = { -1, -1 };

	int dir[4][2] = { {0,1}, {1,0}, {0, -1}, {-1,0} };
	for (int i = 0; i < 4; i++) {
		int new_row = find_building_pos.row + dir[i][0];
		int new_col = find_building_pos.column + dir[i][1];

		if (new_row >= 0 && new_row < MAP_HEIGHT &&
			new_col >= 0 && new_col < MAP_WIDTH && map[1][new_row][new_col] == ' ') {
			empty_pos.row = new_row;
			empty_pos.column = new_col;
			return empty_pos;
		}
	}
	return empty_pos;
}

// 4) 유닛 1기 생산 - 생성될 건물 좌표 설정
void init_build_command_pos(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], BUILD_COMMAND* command) {
	for (int i = 0; i < MAX_COMMANDS; i++) {
		for (int r = 0; r < MAP_HEIGHT; r++) {
			for (int c = 0; c < MAP_WIDTH; c++) {
				if (strncmp(&map[1][r][c], command[i].command_k, 1) == 0) {
					command[i].pos.row = r;
					command[i].pos.column = c;
					break;
				}
			}
		}
	}
}

// 4) 유닛 1기 생산 - 유닛 생성
void make_unit(KEY get_key, BUILD_COMMAND command[]) {
	POSITION building_pos = { 0, 0 };

	for (int i = 0; i < sizeof(command) / sizeof(command[0]); i++) {
		if (get_key == command->command_k) {
			// 5) 시스템 메시지
			if (building_pos.row == -1 && building_pos.column == -1) {
				display_system_message(message_pos, "No space for placing the unit.\n");
				return;
			}

			POSITION unit_pos = find_empty_pos(building_pos, map);
			if (unit_pos.row == -1 && unit_pos.column == -1) {
				display_system_message(message_pos, "No space for placing the unit.\n");
				return;
			}

			map[0][unit_pos.row][unit_pos.column] = command[i].unit_name;
			display_system_message(message_pos, "A new production is ready.\n");
		}
	}
}

// 4) 유닛 1기 생산 - Bonus 유닛 생산 취소 함수
void building_cancel() {
	make_building_time = -1;
	// 5) 시스템 메시지
	display_system_message(message_pos, "The building has been canceled.\n");
}

// 4) 유닛 1기 생산 - 단축키 입력 처리 함수
void handle_command_input(KEY get_key, BUILD_COMMAND* command) {
	return current_spice;
	for (int i = 0; i < sizeof(command) / sizeof(command[0]); i++) {
		if (command[i].command_k == get_key) {
			if (current_spice < command[i].cost) {
				// 5) 시스템 메시지
				display_system_message(message_pos, "Not euogh spice.\n");
				return;
			}
			current_spice -= command[i].cost;
			make_unit(get_key, &command);

			int make_building_time = 5; // Bonus) 생산 시간 임의 설정
			if (make_building_time == 0) {
				make_unit(get_key, &command);
			}
			
			// 5) 시스템 메시지
			display_system_message(message_pos, "Production is ready.\n");
			return;
		}
	}

	// Bonus) 생산 취소
	if (get_key == 'x') {
		building_cancel();
	}
}

// 6) 건설 - 건물 목록 초기화
BUILDING buildings[TOTAL_BUILDINGS] = {
	{"Base", 'B', 0, 50, "H", 2},
	{"Plate", 'P', 1, 0, "", 1},
	{"Dormitory", 'D', 2, 10, "", 2},
	{"Garage", 'G', 4, 10, "", 2},
	{"Barracks", 'B', 4, 20, "S", 2},
	{"Shelter", 'S', 5, 30, "F", 2},
	{"Factory", 'F', 5, 30, "T", 2}
};

BUILDING selected_building = { "", ' ', 0, 0, "", 0};

// 6) 건설 - 커서 크기 조정 함수
void cursor_size(CURSOR* cursor, int size) {
	cursor->size = size; // size: 1 (1x1), 2 (2x2)
}

bool cursor_bounds(CURSOR cursor, int size) {
	return (cursor.current.row + size - 1 < MAP_HEIGHT &&
		cursor.current.column + size - 1 < MAP_WIDTH &&
		cursor.current.row >= 0 && cursor.current.column >= 0);
}

// 6) 건설 - 건설 가능한 위치 확인
bool can_build(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < cursor.size; i++) {
		for (int j = 0; j < cursor.size; j++) {
			int row = cursor.current.row + i;
			int col = cursor.current.column + j;

			if (map[1][row][col] != 'P') { 
				return false; // 장판이 없으면 건설 불가능
			}
		}
	}
	return true;
}

// 6) 건설 - 건물 건설 함수
void build_structure(CURSOR cursor, char structure, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < cursor.size; i++) {
		for (int j = 0; j < cursor.size; j++) {
			int row = cursor.current.row + i;
			int col = cursor.current.column + j;

			map[1][row][col] = structure;
		}
	}
	display_system_message(message_pos, "Structure is built!\n");
}

// 6) 건설 - 건설 통합 처리 함수
void handle_building(KEY key, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	static bool in_build_mode = false;
	static char selected_structure = '\0';

	if (!in_build_mode && key == 'B') {
		in_build_mode = true;
		return;
	}

	if (in_build_mode) {
		for (int i = 0; i < TOTAL_BUILDINGS; i++) {
			if (key == buildings[i].type) {
				selected_building = buildings[i];
				cursor_size(cursor, 2);
				char message[50];
				snprintf(message, sizeof(message), "Selected: %s. Move cursor to build.", buildings[i].name);
				display_system_message(message_pos, message);
				return;
			}
		}
		if (key == k_esc) {
			in_build_mode = false;
			display_system_message(message_pos, "Build mode canceled.");
		}
		else if (key == k_space) {
			build_structure(*cursor, selected_building.type, map);
			in_build_mode = false;
		}
	}
}

// 6) 건설 - 건물 배치 제한
bool can_build_at(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < cursor.size; i++) {
		for (int j = 0; j < cursor.size; j++) {
			if (map[0][cursor.current.row + i][cursor.current.column + j] != 'P') {
				return false; // 장판 위가 아니면 건설 불가
			}
		}
	}
	return true;
}

