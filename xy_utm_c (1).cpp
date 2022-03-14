// xy_utm_c.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>

/*
1경기장 원점- 우측하단
(경도-128.579011515, UTM_Easting-3880818.15)
(위도-35.06938242 UTM_Northing-461616.44)                    
int og_x = 0; // UTM_Northing-461616.44
int og_y = 0; // UTM_Easting-3880818.15

int x = 1;  // UTM_Northing-461636.44
int y = 0; // UTM_Easting-3880818.15

*/
struct utm
{
	double u_n;
	double u_e;
} utm_arr[2][2];

double og_u_n;
double og_u_e;

void CalcUtm() {
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (i == 0 && j == 0)
				continue;

		}

	}
}



int main()
{
	printf("utm 원점(초기위치)입력\n");
	scanf_s("%lf %lf",&og_u_n, &og_u_e);

	utm_arr[0][0].u_n = og_u_n;
	utm_arr[0][0].u_e = og_u_e;
    


	printf("\n 입력한 원점: %lf, %lf \n",og_u_n, og_u_e);
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
