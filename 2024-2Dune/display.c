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

// ���� ����
void reset_color() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COLOR_DEFAULT);
}

void display_system_message(POSITION pos, const char* system_message);
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos);
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
// void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], 
	// int set_col_map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void sandworm_action(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], OBJECT_SAMPLE objects[MAX_OBJECTS]);


void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor,
	OBJECT_SAMPLE objects[MAX_OBJECTS])
{
	display_resource(resource);
	display_map(map, set_col_map);
	display_cursor(cursor);

	display_object_info(map, cursor, resource_pos);
	display_commands(map);
	display_system_message(pos, system_message);

	// init_map(map, set_col_map);
	sandworm_action(map, objects);
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
		gotoxy((POSITION) { i + 1, 0 });
		for (int j = 0; j < MAP_WIDTH; j++) {
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

	for (int i = 0; i < MAX_OBJECTS; i++) {
		gotoxy((POSITION) { objects[i].pos.row + 1, objects[i].pos.column + 1 });
		set_color(set_col_map[1][objects[i].pos.row][objects[i].pos.column]);
		printf("%c", objects[i].repr);
	}

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
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos) {
	char selected = map[1][cursor.current.row][cursor.current.column];
	gotoxy(resource_pos);
	printf("Selected Object>> %c \n", selected);
	// reset_color();
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
	{{MAP_HEIGHT - 3, 1},{MAP_HEIGHT - 3, 1}, 'B', 0, 0, 0, COLOR_ATREIDES},
	{{MAP_HEIGHT - 3, 2},{MAP_HEIGHT - 3, 2}, 'B', 0, 0, 0, COLOR_ATREIDES},
	{{MAP_HEIGHT - 2, 1},{MAP_HEIGHT - 2, 1}, 'B', 0, 0, 0, COLOR_ATREIDES},
	{{MAP_HEIGHT - 2, 2},{MAP_HEIGHT - 2, 2}, 'B', 0, 0, 0, COLOR_ATREIDES},

	// ATREIDES P
	{{MAP_HEIGHT - 3, 3},{MAP_HEIGHT - 3, 3}, 'P', 0, 0, 0, COLOR_PLATE},
	{{MAP_HEIGHT - 3, 4},{MAP_HEIGHT - 3, 4}, 'P', 0, 0, 0, COLOR_PLATE},
	{{MAP_HEIGHT - 2, 3},{MAP_HEIGHT - 2, 3}, 'P', 0, 0, 0, COLOR_PLATE},
	{{MAP_HEIGHT - 2, 4},{MAP_HEIGHT - 3, 4}, 'P', 0, 0, 0, COLOR_PLATE},

	// ATREIDES H
	{{MAP_HEIGHT - 4, 1},{MAP_HEIGHT - 4, 1}, 'H', 0, 0, 0, COLOR_ATREIDES},

	// HARKONNEN B
	{{1, MAP_WIDTH - 3}, {1, MAP_WIDTH - 3}, 'B', 0, 0, 0, COLOR_HARKONNEN},
	{{2, MAP_WIDTH - 3}, {2, MAP_WIDTH - 3}, 'B', 0, 0, 0, COLOR_HARKONNEN},
	{{1, MAP_WIDTH - 2}, {1, MAP_WIDTH - 2}, 'B', 0, 0, 0, COLOR_HARKONNEN},
	{{2, MAP_WIDTH - 2}, {2, MAP_WIDTH - 2}, 'B', 0, 0, 0, COLOR_HARKONNEN},

	// HARKONNEN P
	{{1, MAP_WIDTH - 4}, {1, MAP_WIDTH - 4}, 'P', 0, 0, 0, COLOR_PLATE},
	{{2, MAP_WIDTH - 4}, {2, MAP_WIDTH - 4}, 'P', 0, 0, 0, COLOR_PLATE},
	{{1, MAP_WIDTH - 5}, {1, MAP_WIDTH - 5}, 'P', 0, 0, 0, COLOR_PLATE},
	{{2, MAP_WIDTH - 5}, {2, MAP_WIDTH - 5}, 'P', 0, 0, 0, COLOR_PLATE},

	// HARKONNEN H
	{{3, MAP_WIDTH - 3}, {3, MAP_WIDTH - 3}, 'H', 0, 0, 0, COLOR_HARKONNEN},

	// SPICE
	{{MAP_HEIGHT - 4, 3}, {MAP_HEIGHT - 4, 3}, 'S', 0, 0, 0, COLOR_SPICE},
	{{3, MAP_WIDTH - 4}, {3, MAP_WIDTH - 4}, 'S', 0, 0, 0, COLOR_SPICE},

	// SANDWORM
	{{13, 24}, {0, 0}, 'W', 100000, 0, 1, COLOR_SANDWORM},
	{{7, 56}, {0, 0}, 'W', 100000, 0, 1, COLOR_SANDWORM},

	// ROCK
	{{5, 5}, {5, 5}, 'R', 0, 0, 0, COLOR_ROCK},
	{{6, 6}, {6, 6}, 'R', 0, 0, 0, COLOR_ROCK},
	{{6, 7}, {6, 7}, 'R', 0, 0, 0, COLOR_ROCK},
	{{7, 6}, {7, 6}, 'R', 0, 0, 0, COLOR_ROCK},
	{{7, 7}, {7, 7}, 'R', 0, 0, 0, COLOR_ROCK}
};

int object_count = 26;
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

		if (objects[i].repr == 'B' || objects[i].repr == 'P' || objects[i].repr == 'H') {
			set_col_map[0][row][col] = COLOR_ATREIDES;
		}
		else if (objects[i].repr == 'W' || objects[i].repr == 'T' || objects[i].repr == 'C') {
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
void double_cursor(DIRECTION dir, CURSOR* cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	static clock_t last_click = 0;
	clock_t cur_time = clock();

	// ����Ű ���� �̵�
	int step = 1;
	if ((cur_time - last_click) < CLOCKS_PER_SEC / 2) {
		step = 4;
	}
	last_click = cur_time;

	// Ŀ�� �̵�
	for (int i = 0; i < step; i++) {
		cursor_move(dir);
	}

	// ��ü ���� ������Ʈ
	display_object_info(map, *cursor, resource_pos);
}

// 2) Ŀ�� & ����â - ���� �Լ�
void select_object(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	char selected = map[0][cursor.current.row][cursor.current.column];
	display_object_info(map, cursor, resource_pos);
	
	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };
	display_system_message(system_message_pos, "������Ʈ ����");
}

// 2) Ŀ�� & ����â - ��� �Լ�
void deselect_object() {
	display_object_info(NULL, (CURSOR) { 0, 0 }, resource_pos);

	POSITION system_message_pos = { MAP_HEIGHT + 2, 0 };
	display_system_message(system_message_pos, "������ ����մϴ�.");
}

// 3) �߸� ���� - ����� ���� ���� �Լ�
OBJECT_SAMPLE sandworm = { {13, 24}, {0, 0}, 'W', 1000, 0, 1, COLOR_SANDWORM };

POSITION find_near_unit(POSITION sandworm_pos) {
	POSITION near_unit = objects[0].pos;
	int min_distance = abs(sandworm_pos.row - objects[0].pos.row)
		+ abs(sandworm_pos.column - objects[0].pos.column);

	for (int i = 1; i < unit_count; i++) {
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
	else if (sandworm->pos.row < target.row) {
		sandworm->pos.row--;
	}
	if (sandworm->pos.column < target.column) {
		sandworm->pos.column++;
	}
	else if (sandworm->pos.column < target.column) {
		sandworm->pos.column--;
	}
}

// 3) �߸� ���� - ���� ��ƸԱ� �Լ�
void eat_unit(OBJECT_SAMPLE* sandworm) {
	for (int i = 0; i < unit_count; i++) {
		if (sandworm->pos.row == objects[i].pos.row && sandworm->pos.column == objects[i].pos.column) {
			for (int j = i; j < unit_count - 1;j++) {
				objects[j] = objects[j + 1];
			}
			unit_count--;

			display_system_message(pos, "������� ������ ��ƸԾ����ϴ�!\n");
			break;
		}
	}
}

// 3) �߸� ���� - ����� �輳 �Լ�
void make_spice(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION spice_pos = { rand() % MAP_HEIGHT, rand() % MAP_WIDTH };
	map[1][spice_pos.row][spice_pos.column] = 'S';

	display_system_message(pos, "������� �輳�Ͽ� �����̽� �������� �����Ǿ����ϴ�!\n");
}

// 3) �߸� ���� - ����� �ൿ ���������� ȣ��
void sandworm_action(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], OBJECT_SAMPLE* sandworm) {
	POSITION near_unit_pos = find_near_unit(sandworm->pos);
	sandworm->dest = near_unit_pos;

	move_to_near(&sandworm, sandworm->dest);
	eat_unit(sandworm);
	if (rand() % 1000 == 0) {
		make_spice(map);
	}
}