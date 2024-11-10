/*
*  display.c:
* ȭ�鿡 ���� ������ ���
* ��, Ŀ��, �ý��� �޽���, ����â, �ڿ� ���� ���
* io.c�� �ִ� �Լ����� �����
*/

#include "display.h"
#include "io.h"
#include <time.h> // Ŀ�� �̵� �Լ���

// ����� ������� �»��(topleft) ��ǥ
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION object_pos = { 0, MAP_WIDTH + 2 };


char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], 
	int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);

// �ӽù������� ��ġ ���� ���� ����
POSITION pos = { MAP_HEIGHT + 2, 0 };

// ��Ʈ�����̽�, ���ڳ� �� ������ ������ �迭 ����
int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

// 4) ���� 1�� ���� - ����Ű ���
COMMAND_LIST command_list;
void init_commands() {
	command_list.command_cnt = 5;
	command_list.commands[0] = (BUILD_COMMAND){ 'B', 'H', 0, "Harvester", 0 };
	command_list.commands[1] = (BUILD_COMMAND){ 'B', 'S', 4, "Soldier", 1 };
	command_list.commands[2] = (BUILD_COMMAND){ 'S', 'F', 5, "Fremen", 1 };
	command_list.commands[3] = (BUILD_COMMAND){ 'A','F', 3, "Fighter", 2 };
	command_list.commands[4] = (BUILD_COMMAND){ 'F', 'T', 5, "Heavy Tank", 2 };
}

// 4) ���� 1�� ���� - Bonus ���� �ð�
int make_building_time = -1;

// ���� ����
void reset_color() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_DEFAULT);
}

void display_system_message(POSITION pos, const char* system_message);
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION object_pos);
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void init_set_col_map(int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);

void handle_input(KEY key, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void sandworm_action(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], OBJECT_SAMPLE objects[MAX_OBJECTS]);
void handle_command_input(KEY key, char building);

void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor,
	OBJECT_SAMPLE objects[MAX_OBJECTS], 
	KEY key)
{
	display_resource(resource);
	display_map(map, set_col_map);
	display_cursor(cursor);

	display_object_info(map, cursor, object_pos);
	display_commands(map);
	display_system_message(pos, system_message);

	init_set_col_map(set_col_map, map);

	handle_input(key, &cursor, map);

	sandworm_action(map, objects);

	int building_idx = find_building_pos(key);
	if (building_idx != -1) {  // ��ȿ�� ��ɾ ���� ���� ����
		handle_command_input(key, command_list.commands[building_idx]);
	}
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
	init_set_col_map(set_col_map, map);
	project(map, backbuf);

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

	/*
	for (int i = 0; i < MAX_OBJECTS; i++) {
		gotoxy((POSITION) { objects[i].pos.row + 1, objects[i].pos.column + 1 });
		set_color(set_col_map[1][objects[i].pos.row][objects[i].pos.column]);
		printf("%c", objects[i].repr);
	}
	*/

	/*
	// �� �׵θ� '#'
	for (int i = 0; i < MAP_HEIGHT; i++) {
		set_color(COLOR_DEFAULT);
		map[1][i][0] = '#';
		map[1][i][MAP_WIDTH - 1] = '#';
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][0][j] = '#';
			map[1][MAP_HEIGHT - 1][j] = '#';
		}
	}

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j]) {
				POSITION pos = { i, j };

				// ���� ����
				int set_col = set_col_map[0][i][j];
				for (int layer = 0; layer < N_LAYER; layer++) {
					if (set_col_map[layer][i][j] != COLOR_DEFAULT) {
						set_col = set_col_map[layer][i][j];
					}
				}
				set_color(set_col);

				printc(padd(map_pos, pos), backbuf[i][j], set_col);
			}
			frontbuf[i][j] = backbuf[i][j];
		}
	}
	reset_color();
	*/

	// �縷: �⺻ ����(�� ĭ)
	/*
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[0][i][j] = ' ';
		}
	}
	*/

	// �ǹ�, ����(B, P, S, R)�� layer 0
	// ����(H, W)�� layer 1
	/*
	// ��Ʈ���̵� ����, ����, �Ϻ�����
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

	// ���ڳ� ����, ����, �Ϻ�����
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

	// �����̽�
	map[0][MAP_HEIGHT - 4][3] = 'S';
	map[0][3][MAP_WIDTH - 4] = 'S';
	set_col_map[0][MAP_HEIGHT - 4][3] = COLOR_SPICE;
	set_col_map[0][3][MAP_WIDTH - 4] = COLOR_SPICE;

	// �����
	map[1][13][24] = 'W';
	map[1][7][56] = 'W';
	set_col_map[1][13][24] = COLOR_SANDWORM;
	set_col_map[1][7][56] = COLOR_SANDWORM;

	// ����
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
	*/
}

// frontbuf[][]���� Ŀ�� ��ġ�� ���ڸ� ���� �ٲ㼭 �״�� �ٽ� ���
void display_cursor(CURSOR cursor) {
	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	gotoxy(prev);
	int prev_color = set_col_map[0][prev.row][prev.column];
	set_color(prev_color);
	printf("%c", frontbuf[prev.row][prev.column]);

	gotoxy(curr);
	set_color(COLOR_CURSOR);
	printf(" ");

	reset_color();

	/*
	char ch = frontbuf[prev.row][prev.column];
	printc(padd(map_pos, prev), ch, set_col_map[0][prev.row][prev.column]);

	ch = frontbuf[curr.row][curr.column];
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);
	*/
}

// 1) �غ� - �ý��� �޽��� ǥ�� �Լ� + �ʱ� �޽���
char system_message[200] = "Waiting for the command... "; // �ʱ� �޽����� ���Ƿ� ����

void display_system_message(POSITION pos, const char* system_message) {
	static char last_message[200] = "";
	static int line = 0;

	if (strcmp(last_message, system_message) != 0) {
		strcpy_s(last_message, sizeof(last_message), system_message);
		gotoxy((POSITION) { pos.row + line, pos.column });
		printf("System: %s", system_message);
		
		line++;

		if (line >= 5) {
			line = 0;
		}
	}
}

// 1) �غ� - ����â �Լ�
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION object_pos) {
	char selected = map[1][cursor.current.row][cursor.current.column];
	gotoxy(object_pos);
	printf("Selected Object>> %c \n", selected);

	// 4) Bonus) ���� �ð� ǥ��
	if (make_building_time > 0) {
		printf("Production time: %d sec...\n", make_building_time);
		make_building_time--;
		if (make_building_time == 0) {
			display_system_message(pos, "Production is complete!");
		}
	}

	reset_color();
}

// 1) �غ� - ���â �Լ�
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 2 };
	gotoxy(command_pos);
	printf("Choose the commands number>> 1. Move 2. Attack 3. Harvest\n");
	// reset_color();
}

// 1) �غ� - �ʱ� ���� �ʱ� ��ġ �Լ�
/*
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	// �縷: �⺻ ����(�� ĭ)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[0][i][j] = ' ';
		}
	}

	// �ǹ�, ����(B, P, S, R)�� layer 0
	// ����(H, W)�� layer 1
	
	// ��Ʈ���̵� ����, ����, �Ϻ�����
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

	// ���ڳ� ����, ����, �Ϻ�����
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

	// �����̽�
	map[0][MAP_HEIGHT - 4][3] = 'S';
	map[0][3][MAP_WIDTH - 4] = 'S';
	set_col_map[0][MAP_HEIGHT - 4][3] = COLOR_SPICE;
	set_col_map[0][3][MAP_WIDTH - 4] = COLOR_SPICE;

	// �����
	map[1][13][24] = 'W';
	map[1][7][56] = 'W';
	set_col_map[1][13][24] = COLOR_SANDWORM;
	set_col_map[1][7][56] = COLOR_SANDWORM;

	// ����
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

// 1) �غ� - ���� ���� �Լ�
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

// 2) Ŀ�� & ����â - ����Ű �Է� �� �̵� �Լ�
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

// 2) Ŀ�� & ����â - ����Ű ����Ŭ�� �Լ�
const int double_click_time = 100;

void double_cursor(DIRECTION dir, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	static clock_t last_click = 0;
	clock_t cur_time = clock();

	// ����Ű ���� �̵�
	int step = 1;
	if ((cur_time - last_click) < double_click_time / 2) {
		step = 4;
	}
	last_click = cur_time;

	// Ŀ�� �̵�
	for (int i = 0; i < step; i++) {
		move_cursor(dir);
	}

	// ��ü ���� ������Ʈ
	display_object_info(map, *cursor, resource_pos);
}

// 2) Ŀ�� & ����â - ���� �Լ�
void select_object(CURSOR *cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION pos = cursor->current;
	char current_tile = map[1][pos.row][pos.column];
	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };

	switch (current_tile) {
		//case ' ' :
			//display_system_message(system_message_pos, "�縷 ������ �����Ͽ����ϴ�.");
			//break;
		case 'B' :
			display_system_message(system_message_pos, "������ �����Ͽ����ϴ�.");
			break;
		case 'H' :
			display_system_message(system_message_pos, "�Ϻ����͸� �����Ͽ����ϴ�.");
			break;
		case 'P':
			display_system_message(system_message_pos, "������ �����Ͽ����ϴ�.");
			break;
		case 'W':
			display_system_message(system_message_pos, "������� �����Ͽ����ϴ�.");
			break;
		case 'S':
			display_system_message(system_message_pos, "�����̽� �������� �����Ͽ����ϴ�.");
			break;
		case 'R':
			display_system_message(system_message_pos, "������ �����Ͽ����ϴ�.");
			break;
		default :
			display_system_message(system_message_pos, "�縷 ������ �����߽��ϴ�.");
			break;
	}
	
}

// 2) Ŀ�� & ����â - ��� �Լ�
void deselect_object() {
	display_object_info(NULL, (CURSOR) { 0, 0 }, resource_pos);
	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };
	display_system_message(system_message_pos, "");
}

// 2) Ŀ�� & ����â - Ŀ�� & ����â �Լ� ���������� ȣ��
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
		select_object(cursor, map); // �����̽��ٷ� ������Ʈ ����
		break;
	case k_esc:
		deselect_object(); // ESC Ű�� ���� ���
		break;
	case k_quit:
		outro(); // q Ű ���� ����
		exit(0);
		break;
	default:
		break;
	}
}

// 3) �߸� ���� - ����� ���� ���� �Լ�
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

// 3) �߸� ���� - ����� near_unit���� �̵� �Լ�
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

// 3) �߸� ���� - ���� ��ƸԱ� �Լ�
void eat_unit(OBJECT_SAMPLE* sandworm) {
	for (int i = 0; i < unit_count; i++) {
		if (sandworm->pos.row == objects[i].pos.row && sandworm->pos.column == objects[i].pos.column) {
			if (objects[i].repr != 'W') {
				for (int j = i; j < unit_count - 1;j++) {
					objects[j] = objects[j + 1];
				}
				unit_count--;

				display_system_message(sandworm->pos, "������� ������ ��ƸԾ����ϴ�!\n");
				break;
			}
		}
	}
}

// 3) �߸� ���� - ����� �輳 �Լ�
void make_spice(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION spice_pos = { rand() % MAP_HEIGHT, rand() % MAP_WIDTH };
	map[1][spice_pos.row][spice_pos.column] = 'S';

	display_system_message(pos, "������� �輳�Ͽ� �����̽� �������� �����Ǿ����ϴ�!\n");
}

// 3) �߸� ���� - ����� �ൿ ���������� ȣ�� �Լ�
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
			}
		}
	}
}

// 3) �߸� ���� - ����� �̵� ������Ʈ �Լ�
void update_sandworms(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < unit_count; i++) {
		if (objects[i].repr == 'W') { 
			sandworm_action(map, &objects[i]);
		}
	}
}

// 4) ���� 1�� ���� - ��ġ ã�� �Լ�
POSITION find_building_pos(char building) {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i].repr == building) {
			return objects[i].pos;
		}
	}
	POSITION invalid_pos = { -1, -1 };  // �ǵ������� ��ȿ���� ���� ��ġ�� ��ȯ�ϵ���
	return invalid_pos;
}

// 4) ���� 1�� ���� - �� ���� ã�� �Լ�
POSITION find_empty_pos(POSITION building_pos) {
	POSITION empty_pos = { -1, -1 };

	int dir[4][2] = { {0,1}, {1,0}, {0, -1}, {-1,0} };
	for (int i = 0; i < 4; i++) {
		int new_row = building_pos.row + dir[i][0];
		int new_col = building_pos.column + dir[i][1];

		if (new_row >= 0 && new_row < MAP_HEIGHT &&
			new_col >= 0 && new_col < MAP_WIDTH && map[1][new_row][new_col] == ' ') {
			empty_pos.row = new_row;
			empty_pos.column = new_col;
			return empty_pos;
		}
	}
	return empty_pos;
}

// 4) ���� 1�� ���� - ���� ����
void make_unit(char building, const char* unit_name) {
	POSITION building_pos = find_building_pos(building);
	if (building_pos.row == -1) {
		display_system_message(pos, "No space for placing the unit.\n");
		return;
	}

	POSITION unit_pos = find_empty_pos(building_pos);
	if (unit_pos.row == -1 && unit_pos.column == -1) {
		display_system_message(pos, "No space for placing the unit.\n");
		return;
	}

	map[0][unit_pos.row][unit_pos.column] = unit_name;
	display_system_message(pos, "A new production is ready.\n");
}

// 4) ���� 1�� ���� - Bonus ���� ���� ��� �Լ�
void building_cancel() {
	make_building_time = -1;
	display_system_message(pos, "The building has been canceled.\n");
}

// 4) ���� 1�� ���� - ����Ű �Է� ó�� �Լ�
extern COMMAND_LIST command_list;
int current_spice = 2;

void handle_command_input(KEY key, char building) {
	for (int i = 0; i < command_list.command_cnt; i++) {
		BUILD_COMMAND cmd = command_list.commands[i];

		if (cmd.building == building && cmd.command_k == key) {
			if (current_spice < cmd.cost) {
				display_system_message(pos, "Not euogh spice.\n");
				return;
			}
			current_spice -= cmd.cost;
			make_unit(building, cmd.unit_name);

			int make_building_time = 5; // Bonus) ���� �ð� ���� ����
			if (make_building_time == 0) {
				make_unit(cmd.building, cmd.unit_name);
			}
			
			display_system_message(pos, "Production is ready.\n");
			return;
		}
	}

	// Bonus) ���� ���
	if (key == 'x') {
		building_cancel();
	}
}