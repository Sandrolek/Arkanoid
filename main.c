// подключение библиотек
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include "glut.h"

// параметры окна
#define WIDTH 900
#define HEIGHT 700
#define SCALE 1

#define PI 3.14

// константы клавиш
#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_P 0x50
#define KEY_L 0x4C
#define KEY_F 0x46
#define KEY_V 0x56
#define KEY_SPACE 0x20
#define MASK  0x8000

// параметры игроков и мяча
#define PLAYER_WIDTH 10
#define PLAYER_HEIGHT 70
#define SPEED_PLAYER 10

#define WALL_HEIGHT 10

#define BALL_DIAMETER 20
#define BALL_TIMEOUT 10
#define SPEED_BALL 10

// статус игроков
enum state
{
	STAY = 0,
	UP = 1,
	DOWN = 2
};

// булевый тип
enum BOOL
{
	MY_TRUE = 1,
	MY_FALSE = 0
};

// с какой стеной столкновение
enum COLL
{
	UP_WALL = 0,
	RIGHT_WALL = 3,
	DOWN_WALL = 2,
	LEFT_WALL = 1
};

// кто забил
enum GOAL
{
	NONE = 0,
	RIGHT_PLAYER = 2,
	LEFT_PLAYER = 1
};

// сектор угла
enum SECTOR
{
	RIGHT_UP = 0,
	LEFT_UP = 1,
	LEFT_DOWN = 2,
	RIGHT_DOWN = 3
};

unsigned char image[HEIGHT][WIDTH][3];

int space = 0; // 0 - отпущена, 1 - нажата
int new_game = 1; // 0 - старая игра, 1 - новая(ждем пробела)

// переменные и статусы игроков

int stop_up;
int stop_down;

int player1_Y = 0;
int player1_X = 0;

int player1_X_last = 0;
int player1_Y_last = 0;

enum state player1_state = STAY;
enum state player1_last_state = STAY;

int player2_Y = 0;
int player2_X = 0;

int player2_X_last;
int player2_Y_last = 0;

enum state player2_state = STAY; // 0 - вверх, 1 - вниз                                   
enum state player2_last_state = STAY; // 0 - вверх, 1 - вниз

// переменные мяча
int ball_X_now;
int ball_Y_now;

int ball_X_last;
int ball_Y_last;

int ball_X_next = 0;
int ball_Y_next = 0;

long int ball_now_time = 0;
long int ball_next_time = 0;

int cur_angle;

enum BOOL ball_bounce = MY_FALSE; // 0 - только зашли, 1 - ждем 2 секи

enum COLL ball_coll;

enum SECTOR cur_sector;
enum SECTOR need_sector;

enum GOAL who_scored = NONE;
int goal_left = 0, goal_right = 0; // goal_left - пропуски мячей правым игроком, goal_right - левым игроком

// объект для хранения времени
SYSTEMTIME st;

// очищение заданного прямоугольника
void Clear_Rect(int X, int Y, int width, int height)
{
	int i, j, c;

	////printf("Cleaning rect on X: %i Y: %i\n\n", X, Y);

	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
			for (c = 0; c < 3; c++)
				image[Y + i][X + j][c] = 0;
}

// создание прямоугольника
void Make_rect(int width, int height, int X, int Y, enum BOOL clear, int X_last, int Y_last)
{
	int i, j;

	//printf("clear: %i\n", clear);
	if (clear == MY_TRUE)
		Clear_Rect(X_last, Y_last, width, height);

	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			image[Y + i][X + j][0] = 255;
			image[Y + i][X + j][1] = 128;
			image[Y + i][X + j][2] = 128;
		}
}

// слздание мяча
void Make_Ball(int diameter, int X, int Y, int X_last, int Y_last)
{
	int i, j;

	Clear_Rect(X_last, Y_last, diameter, diameter);

	for (i = 0; i < diameter; i++)
		for (j = 0; j < diameter; j++)
		{
			image[Y + i][X + j][0] = 255;
			image[Y + i][X + j][1] = 128;
			image[Y + i][X + j][2] = 255;
		}
}

/* Функция реакции на событие перерисовка окна */
void Draw(void)
{
	glClearColor(0.3, 0.5, 0.7, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glPixelZoom(SCALE, SCALE);
	glDrawPixels(WIDTH, HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, image);

	glFinish();
	glutSwapBuffers();
	glutPostRedisplay();
}

// своя обработка клавиш (WinAPI)
void MyKbd(void)
{
	////printf("X_1: %i, Y_1: %i\n", player1_X, player1_Y);
	player1_Y_last = player1_Y;

	////printf("X_2: %i, Y_2: %i\n", player2_X, player2_Y);
	player2_Y_last = player2_Y;

	if (GetKeyState(KEY_P) & MASK) // p
	{
		////printf("Pressed P\n");

		player2_Y += SPEED_PLAYER;

		if (player2_Y >= stop_up)
			player2_Y = stop_up;
	}

	if (GetKeyState(KEY_L) & MASK) // l
	{
		////printf("Pressed L\n");

		player2_Y -= SPEED_PLAYER;

		if (player2_Y <= stop_down)
			player2_Y = stop_down;
	}

	if (GetKeyState(KEY_W) & MASK) // w
	{
		////printf("Pressed W\n");

		player1_Y += SPEED_PLAYER;

		if (player1_Y >= stop_up)
			player1_Y = stop_up;
	}

	if (GetKeyState(KEY_S) & MASK) // s
	{
		////printf("Pressed S\n");

		player1_Y -= SPEED_PLAYER;

		if (player1_Y <= stop_down)
			player1_Y = stop_down;
	}

	if (GetKeyState(KEY_SPACE) & MASK)
		new_game = 0;

	player1_last_state = player1_state;
	player2_last_state = player2_state;

	Make_rect(PLAYER_WIDTH, PLAYER_HEIGHT, player1_X, player1_Y, MY_TRUE, player1_X_last, player1_Y_last);
	Make_rect(PLAYER_WIDTH, PLAYER_HEIGHT, player2_X, player2_Y, MY_TRUE, player2_X_last, player2_Y_last);
}

// обhаботка клавиши escape
void Kbd(unsigned char Key, int x, int y)
{
	if (Key == 27)
		exit(0);
}

// начало розыгрыша, после гола или в самом начале
void start_game()
{
	double angle_rad = 0;
	int angle_deg = 0;

	// обозначаем, что надо ждать пробела
	new_game = 1;

	// иициализируем положение мяча и игроков
	ball_X_next = WIDTH / 2 - BALL_DIAMETER / 2;
	ball_Y_next = HEIGHT / 2 - BALL_DIAMETER / 2;

	player1_Y = player2_Y = HEIGHT / 2 - PLAYER_HEIGHT / 2;

	printf("In start, who_scored: %i\n", who_scored);

	// в зависимости от того, кто забил, делаем уда мячом
	angle_rad = atan((HEIGHT / 2.0) / (WIDTH / 2.0));
	angle_deg = angle_rad * 180 / PI;
	printf("Angle_deg: %i, Angle_rad: %f, (HEIGHT / 2) / (WIDTH / 2): %f\n", angle_deg, angle_rad, (HEIGHT / 2.0) / (WIDTH / 2.0));
	switch (who_scored)
	{
	case NONE:
		cur_angle = rand() % 360;
		if (cur_angle >= 90 && cur_angle <= 270)
			cur_angle = 180 - angle_deg + rand() % (2 * angle_deg);
		else
			cur_angle = (360 - angle_deg + rand() % (2 * angle_deg)) % 360;
		break;
	case LEFT_PLAYER:
		cur_angle = 180 - angle_deg + rand() % (2 * angle_deg);
		break;
	case RIGHT_PLAYER:
		cur_angle = (360 - angle_deg + rand() % (2 * angle_deg)) % 360;
		break;
	}

	// ограничение, чтобы не был слишком прямой угол
	if (cur_angle <= 100 && cur_angle >= 90)
		cur_angle = 100;
	else if (cur_angle <= 90 && cur_angle >= 80)
		cur_angle = 80;
	else if (cur_angle >= 0 && cur_angle <= 10)
		cur_angle = 10;
	else if (cur_angle <= 360 && cur_angle >= 350)
		cur_angle = 350;
	else if (cur_angle <= 280 && cur_angle >= 270)
		cur_angle = 280;
	else if (cur_angle <= 270 && cur_angle >= 260)
		cur_angle = 260;
	else if (cur_angle <= 190 && cur_angle >= 180)
		cur_angle = 190;
	else if (cur_angle <= 180 && cur_angle >= 170)
		cur_angle = 170;

	// отрисовка
	Clear_Rect(0, 0, WIDTH, HEIGHT);

	Make_Ball(BALL_DIAMETER, ball_X_next, ball_Y_next, ball_X_now, ball_Y_now);

	Make_rect(PLAYER_WIDTH, PLAYER_HEIGHT, player1_X, player1_Y, MY_TRUE, player1_X_last, player1_Y_last);
	Make_rect(PLAYER_WIDTH, PLAYER_HEIGHT, player2_X, player2_Y, MY_TRUE, player2_X_last, player2_Y_last);

	Make_rect(WIDTH, WALL_HEIGHT, 0, 0, MY_FALSE, 0, 0);
	Make_rect(WIDTH, WALL_HEIGHT, 0, HEIGHT - WALL_HEIGHT, MY_FALSE, 0, 0);

	printf("Angle: %i\n", cur_angle);
}

// отскок мяча от стены
int bounce(enum COLL wall, int angle)
{
	int last_angle;

	angle -= 90 * cur_sector;
	angle = 90 - angle;

	//printf("Angle changed: %i ", angle);

	last_angle = 90 * need_sector + angle;

	//printf("Now angle: %i", last_angle);

	return last_angle;
}

// выбор сектора угла для bounce
void choose_angle(int angle)
{
	//printf("In CHOOSE: Angle: %i, ", angle);
	if (0 <= angle && angle <= 90)
	{
		//printf("0 - 90\n");
		cur_sector = RIGHT_UP;
		if (ball_coll == RIGHT_WALL)
			need_sector = LEFT_UP;
		else if (ball_coll == UP_WALL)
			need_sector = RIGHT_DOWN;
	}
	else if (90 < angle && angle <= 180)
	{
		//printf("90 - 180\n");
		cur_sector = LEFT_UP;
		if (ball_coll == LEFT_WALL)
			need_sector = RIGHT_UP;
		else if (ball_coll == UP_WALL)
			need_sector = LEFT_DOWN;
	}
	else if (180 < angle && angle <= 270)
	{
		//printf("180 - 270\n");
		cur_sector = LEFT_DOWN;
		if (ball_coll == LEFT_WALL)
			need_sector = RIGHT_DOWN;
		else if (ball_coll == DOWN_WALL)
			need_sector = LEFT_UP;
	}
	else if (270 < angle && angle <= 360)
	{
		//printf("270 - 360\n");
		cur_sector = RIGHT_DOWN;
		if (ball_coll == DOWN_WALL)
			need_sector = RIGHT_UP;
		else if (ball_coll == RIGHT_WALL)
			need_sector = LEFT_DOWN;
	}
	//printf("Cur: %i, Need: %i\n", cur_sector, need_sector);
}

// движение мяча и обработка столкновений и голов
void Move_Ball_Angle(int angle)
{
	int dif_X = 0, dif_Y = 0;

	GetLocalTime(&st);
	ball_now_time = st.wMinute * 60000 + st.wSecond * 1000 + st.wMilliseconds;
	//printf("Time: %i\n", ball_now_time);
	if (ball_now_time >= ball_next_time)
	{
		ball_next_time = ball_now_time + BALL_TIMEOUT;

		if (new_game != 1)
		{
			if (ball_bounce == MY_TRUE)
			{
				//printf("Stop waiting\n");
				ball_bounce = MY_FALSE;

				choose_angle(angle);

				cur_angle = bounce(ball_coll, angle);

				Make_Ball(BALL_DIAMETER, ball_X_next, ball_Y_next, ball_X_now, ball_Y_now);
				Draw();

				// printf("Angle: %i\n", cur_angle);
			}
			else
			{
				dif_X = cos(angle * PI / 180) * SPEED_BALL;
				dif_Y = sin(angle * PI / 180) * SPEED_BALL;

				ball_X_next = ball_X_now + dif_X;
				ball_Y_next = ball_Y_now + dif_Y;

				if (ball_X_next >= WIDTH - 1 - BALL_DIAMETER)
				{
					goal_left++;
					who_scored = LEFT_PLAYER;
					printf("GOAL for left!!\n");

					start_game();
				}
				// касание правым игроком
				else if (ball_X_next >= player2_X - BALL_DIAMETER && (ball_Y_next >= player2_Y - BALL_DIAMETER && ball_Y_next <= player2_Y + PLAYER_HEIGHT))
				{
					ball_bounce = MY_TRUE;
					ball_coll = RIGHT_WALL;

					ball_X_next = player2_X - BALL_DIAMETER;

					Make_Ball(BALL_DIAMETER, ball_X_next, ball_Y_next, ball_X_now, ball_Y_now);
					Draw();
				}
				// касание левым игроком
				else if (ball_X_next <= player1_X + PLAYER_WIDTH && (ball_Y_next <= player1_Y + PLAYER_HEIGHT && ball_Y_next >= player1_Y - BALL_DIAMETER))
				{
					ball_bounce = MY_TRUE;
					ball_coll = LEFT_WALL;

					ball_X_next = player1_X + PLAYER_WIDTH;

					Make_Ball(BALL_DIAMETER, ball_X_next, ball_Y_next, ball_X_now, ball_Y_now);
					Draw();
				}
				else if (ball_X_next <= 0)
				{
					goal_right++;
					who_scored = RIGHT_PLAYER;
					printf("GOAL for right!!\n");

					start_game();
				}
				else if (ball_Y_next <= 0 + WALL_HEIGHT)
				{
					ball_bounce = MY_TRUE;
					ball_coll = DOWN_WALL;

					ball_Y_next = 0 + WALL_HEIGHT;
					Make_Ball(BALL_DIAMETER, ball_X_next, ball_Y_next, ball_X_now, ball_Y_now);
					Draw();
				}
				else if (ball_Y_next >= HEIGHT - 1 - BALL_DIAMETER - WALL_HEIGHT)
				{
					ball_bounce = MY_TRUE;
					ball_coll = UP_WALL;

					ball_Y_next = HEIGHT - 1 - BALL_DIAMETER - WALL_HEIGHT;
					Make_Ball(BALL_DIAMETER, ball_X_next, ball_Y_next, ball_X_now, ball_Y_now);
					Draw();
				}

				Make_Ball(BALL_DIAMETER, ball_X_next, ball_Y_next, ball_X_now, ball_Y_now);

				ball_X_now = ball_X_next;
				ball_Y_now = ball_Y_next;
			}
		}
	}
}

// callback glut-а, использую как будто вечный цикл
void Idle(void) // Передвижение мяча + Опрос клавиатуры + Отрисовка игроков
{

	Move_Ball_Angle(cur_angle);

	MyKbd();
}

// инициализация, вызывается один раз
void _init()
{
	GetLocalTime(&st);
	ball_now_time = st.wMinute * 60000 + st.wSecond * 1000 + st.wMilliseconds;
	ball_next_time = ball_now_time + BALL_TIMEOUT;

	stop_up = HEIGHT - PLAYER_HEIGHT - WALL_HEIGHT;
	stop_down = WALL_HEIGHT;

	player2_X = WIDTH - PLAYER_WIDTH;
	player2_X_last = WIDTH - PLAYER_WIDTH;

	srand(time(NULL));

	start_game();

	ball_X_now = ball_X_next;
	ball_Y_now = ball_Y_next;

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("ARKANOID");

	glutDisplayFunc(Draw);
	glutKeyboardFunc(Kbd);
	glutIdleFunc(Idle);
}

// main.
void main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	_init();

	printf("Started\n");

	glutMainLoop();
}