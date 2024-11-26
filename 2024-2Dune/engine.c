#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

/*
	20221761 천승은 Dune 1.5 중간 제출
1) 준비
	화면 배치 구현
	display() 하위 함수 작성
	초기 상태 - 표시, 초기 배치, 지형 구현
2) 커서 & 상태창
	선택(스페이스 바)
3) 중립 유닛
	샌드웜
		- 가장 가까운 유닛을 향해 천천히 움직임
		- 일반 유닛을 만나면 잡아 먹음
		- 가끔 배설(스파이스 매장지 생성)
4) 유닛 1기 생산
	명령어(단축키)와 Bonus(생산 시간, 'x'로 취소)를 구현했으나, 오류 발생으로 실행시 적용되지 않습니다.
*/ 

/*
1031 display.c에 [1) 준비]의 화면 배치 함수 구현
1101 display.c에 [1) 준비]의 초기 상태 배치 구현
	 + [2) 커서 & 상태창] 방향키 이동, 더블클릭, 선택, 취소 함수 구현
1102 display.c와 common.h 수정하여 [2) 커서 & 상태창] 오류 수정
1103 display.c, display.h, common.h 수정하여 [1) 준비]와 [2) 커서 & 상태창]
	 오류 수정하고, 실행시 콘솔 디테일 추가
1104 display.c [1) 준비] 위치 수정, [2) 커서 & 상태창] 오류 수정
1105 [3) 중립 유닛] 함수 구현 및 구조체 정의
1106 [3) 중립 유닛] 오류 수정
1107 전체적인 출력 오류 수정
1109 [2) 커서 & 상태창] 오류 수정
1110 [3) 중립 유닛] 기능 개선 수정
	 [4) 유닛 1기 생산] 및 [4) Bonus] 구현
1111 [4) 유닛 1기 생산] 및 [4) Bonus] 구현 구체화
	 전체적인 오류 개선
1115 [4) 유닛 1기 생산] 오류 수정
1116 전체적인 오류 수정1
1118 [5) 시스템 메시지] 구현
1123 [6) 건설] 구현
1126 [6) 건설] [7) 유닛 목록 구현] 구현
*/

void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);


/* ================= control =================== */
int sys_clock = 0;		// system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, {1, 1} };


/* ================= game data =================== */
char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

RESOURCE resource = {
	.spice = 0,
	.spice_max = 0,
	.population = 0,
	.population_max = 0
};

OBJECT_SAMPLE obj = {
	.pos = {1, 1},
	.dest = {MAP_HEIGHT - 2, MAP_WIDTH - 2},
	.repr = 'o',
	.speed = 300,
	.next_move_time = 300
};

/* ================= main() =================== */
int main(void) {
	srand((unsigned int)time(NULL));

	init();
	intro();
	KEY key = k_none;
	display(resource, map, cursor, objects, key);

	while (1) {
		// loop 돌 때마다(즉, TICK==10ms마다) 키 입력 확인
		KEY key = get_key();

		// 키 입력이 있으면 처리
		if (is_arrow_key(key)) {
			cursor_move(ktod(key));
		}
		else {
			// 방향키 외의 입력
			switch (key) {
			case k_quit: outro();
			case k_none:
			case k_undef:
			default: break;
			}
		}

		// 샘플 오브젝트 동작
		sample_obj_move();
		if (cursor.current.row < 0 || cursor.current.row >= MAP_HEIGHT ||
			cursor.current.column < 0 || cursor.current.column >= MAP_WIDTH) {
			printf("Cursor out of bounds!\n");
			fflush(stdout);
			exit(1);
		}

		// 화면 출력
		display(resource, map, cursor, objects, key);
		Sleep(TICK);
		sys_clock += 10;
	}
	system("pause");
	return 0;
}

/* ================= subfunctions =================== */
void intro(void) {
	printf("Power over SPICE is power over ALL.\n\n>> ... DUNE 1.5");
	Sleep(2000);
	system("cls");
}

void outro(void) {
	printf("exiting...\n");
	exit(0);
}

void init(void) {
	// layer 0(map[0])에 지형 생성
	for (int j = 0; j < MAP_WIDTH; j++) {
		map[0][0][j] = '#';
		map[0][MAP_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < MAP_HEIGHT - 1; i++) {
		map[0][i][0] = '#';
		map[0][i][MAP_WIDTH - 1] = '#';
		for (int j = 1; j < MAP_WIDTH - 1; j++) {
			map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])은 비워 두기(-1로 채움)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][i][j] = -1;
		}
	}

	// object sample
	map[1][obj.pos.row][obj.pos.column] = 'o';
}

// (가능하다면) 지정한 방향으로 커서 이동
void cursor_move(DIRECTION dir) {
	POSITION curr = cursor.current;
	POSITION new_pos = pmove(curr, dir);

	// validation check
	if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 && \
		1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {

		cursor.previous = cursor.current;
		cursor.current = new_pos;
	}
}

/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
	// 현재 위치와 목적지를 비교해서 이동 방향 결정	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// 목적지 도착. 지금은 단순히 원래 자리로 왕복
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright로 목적지 설정
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft로 목적지 설정
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// 가로축, 세로축 거리를 비교해서 더 먼 쪽 축으로 이동
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos가 맵을 벗어나지 않고, (지금은 없지만)장애물에 부딪히지 않으면 다음 위치로 이동
	// 지금은 충돌 시 아무것도 안 하는데, 나중에는 장애물을 피해가거나 적과 전투를 하거나... 등등
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 && \
		map[1][next_pos.row][next_pos.column] < 0) {

		return next_pos;
	}
	else {
		return obj.pos;  // 제자리
	}
}

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) {
		// 아직 시간이 안 됐음
		return;
	}

	// 오브젝트(건물, 유닛 등)은 layer1(map[1])에 저장
	//map[1][obj.pos.row][obj.pos.column] = -1;
	map[1][obj.pos.row][obj.pos.column] = ' ';	// 임시방편으로 공백으로 수정

	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}