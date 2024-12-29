# 2024-2Dune

22학번 천승은

---------- 구현 목록 ----------

1) 준비 
	(화면 배치 구현, 
	display() 하위 함수 작성, 
	초기 상태 - 표시, 초기 배치, 지형 구현)
2) 커서 & 상태창 
	(선택(스페이스 바))
3) 중립 유닛 
	(샌드웜 
		- 가장 가까운 유닛을 향해 천천히 움직임, 
		- 일반 유닛을 만나면 잡아 먹음, 
		- 가끔 배설(스파이스 매장지 생성))
4) 유닛 1기 생산 
	(명령어(단축키)와 Bonus(생산 시간, 'x'로 취소)를 구현했으나, 오류 발생으로 실행시 적용되지 않습니다.)
5) 시스템 메시지 
	(시스템 메시지 로그 출력)
6) 건설 
	(커서 변환, 
	아무것도 선택하지 않은 상태에서는-)
7) 유닛 목록 구현 
	(common.h에 구현)


---------- 알고리즘 ----------

3) 중립 유닛 - 샌드웜 알고리즘
 [동족(샌드웜)을 제외한, 가장 가까운 유닛을 향해 천천히 움직임] 기능은 함수를 3가지로 나누어 구현했습니다. 기능 면에서 '가까운 유닛 감지하여 좌표를 반환하는 함수(find_near_unit())', '가까운 유닛으로 이동하는 함수(move_to_near())'로 나누었으며, 이 두 함수를 종합적으로 포괄하여 display() 함수에서 한 번에 호출할 수 있도록 '샌드웜 행동 종합적으로 호출 함수(sandworm_action())' 함수를 구현했습니다.

 find_near_unit() 함수입니다.
 함수 상단에 의도적으로 유효하지 않은 좌표 near_unit과, 임의의 큰 값을 저장한 min_distance 변수를 선언하였습니다.
 for 문을 통해 초기 상태의 유닛 개수 unit_count만큼 초기 상태의 유닛 배열 objects를 순회하도록 하고, 만약 샌드웜 레퍼선스를 가질 시 건너뛰도록 합니다.
 인수로 받는 샌드웜의 좌표와 object[i] 좌표 간의 거리를 하나씩 구해서 변수 distance에 저장하도록 하며, 만약 계산한 거리 값이 초기에 선언한 min_distance와 비교했을 때 값이 작다면 near_unit을 해당 유닛의 좌표로 지정합니다.

 move_to_near() 함수입니다.
 후술할 target 좌표의 row와 column 값과, 샌드웜의 좌표 row와 column 값을 각각 비교 후 증감하여 샌드웜이 이동할 목적지를 현재 위치 pos에 저장하도록 하여 샌드웜의 새 좌표를 설정합니다.

 sandworm_action() 함수입니다.
 find_near_unit()에서 반환한 좌표를 target에 저장한 후, move_to_near()와 호출합니다.
