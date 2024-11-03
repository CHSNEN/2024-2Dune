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
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);

// �ӽù������� ��ġ ���� ���� ����
POSITION pos = { MAP_HEIGHT + 2, 0 };

void display_system_message(POSITION pos, const char* system_message);
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos);
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void init_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);


void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor)
{
	display_resource(resource);
	display_map(map);
	display_cursor(cursor);

	display_system_message(pos, system_message);
	display_object_info(map, cursor, resource_pos);
	display_commands(map);

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

	// �� �׵θ� '#'
	for (int i = 0; i < MAP_HEIGHT; i++) {
		map[1][i][0] = '#';
		map[1][i][MAP_WIDTH - 1] = '#';
		for (int j = 0; j < MAP_WIDTH; j++){
			map[1][0][j] = '#';
			map[1][MAP_HEIGHT - 1][j] = '#';
		}
	}
}

// frontbuf[][]���� Ŀ�� ��ġ�� ���ڸ� ���� �ٲ㼭 �״�� �ٽ� ���
void display_cursor(CURSOR cursor) {
	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	char ch = frontbuf[prev.row][prev.column];
	printc(padd(map_pos, prev), ch, COLOR_DEFAULT);

	ch = frontbuf[curr.row][curr.column];
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);
}

// 1) �غ� - �ý��� �޽��� ǥ�� �Լ� + �ʱ� �޽���
char system_message[200] = "Waiting for the command... "; // �ʱ� �޽����� ���Ƿ� ����

void display_system_message(POSITION pos, const char* system_message) {
	gotoxy(pos);
	printf("System: %s\n", system_message);
}

// 1) �غ� - ����â �Լ�
void display_object_info(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor, POSITION resource_pos) {
	char selected = map[1][cursor.current.row][cursor.current.column];
	gotoxy(resource_pos);
	printf("Selected Object>> %c\n", selected);
}

// 1) �غ� - ���â �Լ�
void display_commands(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 2 };
	gotoxy(command_pos);
	printf("Choose the commands number>> 1. Move 2. Attack 3. Harvest\n");
}

// 1) �غ� - �ʱ� ���� �ʱ� ��ġ �Լ�
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
	set_color(COLOR_ATREIDES);
	map[0][MAP_HEIGHT - 3][1] = 'B';
	map[0][MAP_HEIGHT - 3][2] = 'B';
	map[0][MAP_HEIGHT - 2][1] = 'B';
	map[0][MAP_HEIGHT - 2][2] = 'B';

	set_color(COLOR_PLATE);
	map[0][MAP_HEIGHT - 3][3] = 'P';
	map[0][MAP_HEIGHT - 3][4] = 'P';
	map[0][MAP_HEIGHT - 2][3] = 'P';
	map[0][MAP_HEIGHT - 2][4] = 'P';

	set_color(COLOR_ATREIDES);
	map[1][MAP_HEIGHT - 2][1] = 'H';

	// ���ڳ� ����, ����, �Ϻ�����
	set_color(COLOR_HARKONNEN);
	map[0][1][MAP_WIDTH - 3] = 'B';
	map[0][2][MAP_WIDTH - 3] = 'B';
	map[0][1][MAP_WIDTH - 2] = 'B';
	map[0][2][MAP_WIDTH - 2] = 'B';

	set_color(COLOR_PLATE);
	map[0][3][MAP_WIDTH - 3] = 'P';
	map[0][3][MAP_WIDTH - 3] = 'P';
	map[0][3][MAP_WIDTH - 2] = 'P';
	map[0][3][MAP_WIDTH - 2] = 'P';

	set_color(COLOR_HARKONNEN);
	map[1][2][MAP_WIDTH - 3] = 'H';

	// �����̽�
	set_color(COLOR_SPICE);
	map[0][MAP_HEIGHT - 4][3] = 'S';
	map[0][3][MAP_WIDTH - 4] = 'S';

	// �����
	set_color(COLOR_SANDWORM);
	map[1][13][24] = 'W';
	map[1][47][56] = 'W';

	// ����
	set_color(COLOR_ROCK);
	map[0][5][5] = 'R';
	map[0][6][6] = 'R';
	map[0][6][7] = 'R';
	map[0][7][6] = 'R';
	map[0][7][7] = 'R';
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
		step = 2;
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