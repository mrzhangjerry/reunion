#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define DAYS 365
#define STEPS_PER_DAY 40
#define GRID_SIZE 200
#define MAP_W (GRID_SIZE / 2)
#define MAP_H (GRID_SIZE / 4)
#define PAIRS 10000
#define DAY_MS 200

#define BLUE "\033[38;5;33m"
#define PINK "\033[38;5;213m"
#define WHITE "\033[97m"
#define RESET "\033[0m"

typedef struct{
    int x;
    int y;
}person;

static const int DX[4] = {0, 0, 1, -1};
static const int DY[4] = {1, -1, 0, 0};

static int inside(int x, int y){
    return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
}

static void random_step(person *p){
    while(1){
        int d = rand() % 4;
        int nx = p->x + DX[d];
        int ny = p->y + DY[d];
        if(inside(nx, ny)){
            p->x = nx;
            p->y = ny;
            return;
        }
    }
}

static double next_meet_prob(person b, person g){
    int bx[4], by[4], gx[4], gy[4], nb = 0, ng = 0;
    for(int d = 0; d < 4; d++){
        if(inside(b.x + DX[d], b.y + DY[d])){ bx[nb] = b.x + DX[d]; by[nb] = b.y + DY[d]; nb++; }
        if(inside(g.x + DX[d], g.y + DY[d])){ gx[ng] = g.x + DX[d]; gy[ng] = g.y + DY[d]; ng++; }
    }
    int hits = 0;
    for(int i = 0; i < nb; i++)
        for(int j = 0; j < ng; j++)
            if(bx[i] == gx[j] && by[i] == gy[j]) hits++;
    return (double)hits / (nb * ng);
}

static void sleep_ms(int ms){
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec t = {ms / 1000, (ms % 1000) * 1000000L};
    nanosleep(&t, NULL);
#endif
}

static void draw(person boy, person girl, int day, const double next_p[], int meets_today,
                 int total_meets, double p_cum, double p_first_today, double p_same_now){
    int bc = boy.x / 2,  br = (GRID_SIZE - 1 - boy.y) / 4;
    int gc = girl.x / 2, gr = (GRID_SIZE - 1 - girl.y) / 4;

    printf("\033[H");
    printf("Day %3d / %d   " BLUE "boy" RESET "  " PINK "girl" RESET "  " WHITE "same area" RESET "\033[K\n", day, DAYS);
    printf("+");
    for(int c = 0; c < MAP_W; c++) putchar('-');
    printf("+\033[K\n");
    for(int r = 0; r < MAP_H; r++){
        putchar('|');
        for(int c = 0; c < MAP_W; c++){
            int isB = (r == br && c == bc), isG = (r == gr && c == gc);
            if(isB && isG)  printf(WHITE "\xE2\xA3\xBF" RESET);
            else if(isB)    printf(BLUE  "\xE2\xA3\xBF" RESET);
            else if(isG)    printf(PINK  "\xE2\xA3\xBF" RESET);
            else            putchar(' ');
        }
        printf("|\033[K\n");
    }
    printf("+");
    for(int c = 0; c < MAP_W; c++) putchar('-');
    printf("+\033[K\n");

    printf("This pair: boy=(%3d,%3d) girl=(%3d,%3d)  meetings today=%d  total=%d\033[K\n",
           boy.x, boy.y, girl.x, girl.y, meets_today, total_meets);
    printf("P(meet on NEXT step) after each step today:\033[K\n");
    for(int row = 0; row < 4; row++){
        for(int k = 0; k < 10; k++){
            int i = row * 10 + k;
            double p = next_p[i];
            if(p > 0) printf(WHITE "%2d:%.4f " RESET, i + 1, p);
            else      printf("%2d:%.4f ", i + 1, p);
        }
        printf("\033[K\n");
    }
    printf("Over %d simulated pairs:\033[K\n", PAIRS);
    printf("  P(met at least once by end of day %d) = %.4f\033[K\n", day, p_cum);
    printf("  P(FIRST meeting happens on day %d)     = %.4f\033[K\n", day, p_first_today);
    printf("  P(in same cell at end of day %d)       = %.4f\033[K\n", day, p_same_now);
    printf("\033[J");
    fflush(stdout);
}

static person boys[PAIRS], girls[PAIRS];
static unsigned char met[PAIRS];

int main(void){
    srand((unsigned)time(NULL));
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | 0x0004);
#endif

    for(int i = 0; i < PAIRS; i++){
        boys[i].x = GRID_SIZE / 2;  boys[i].y = GRID_SIZE / 2;
        girls[i] = boys[i];
        met[i] = 0;
    }

    int ever_met = 0, total_meets = 0;
    double next_p[STEPS_PER_DAY];

    printf("\033[2J\033[?25l");

    for(int day = 1; day <= DAYS; day++){
        int first_today = 0, meets_today = 0;
        for(int s = 0; s < STEPS_PER_DAY; s++) next_p[s] = 0;

        for(int s = 0; s < STEPS_PER_DAY; s++){
            int step = (day - 1) * STEPS_PER_DAY + s + 1;
            for(int i = 0; i < PAIRS; i++){
                if(step == 1){
                    boys[i].x++;
                    girls[i].x--;
                }else{
                    random_step(&boys[i]);
                    random_step(&girls[i]);
                }
                if(boys[i].x == girls[i].x && boys[i].y == girls[i].y){
                    if(i == 0){ meets_today++; total_meets++; }
                    if(!met[i]){ met[i] = 1; ever_met++; first_today++; }
                }
            }
            next_p[s] = next_meet_prob(boys[0], girls[0]);
            if(total_meets > 0) break;
        }

        int same_now = 0;
        for(int i = 0; i < PAIRS; i++)
            if(boys[i].x == girls[i].x && boys[i].y == girls[i].y) same_now++;

        draw(boys[0], girls[0], day, next_p, meets_today, total_meets,
             (double)ever_met / PAIRS, (double)first_today / PAIRS, (double)same_now / PAIRS);
        sleep_ms(DAY_MS);

        if(total_meets > 0){
            printf("\033[?25h\nThey met again on day %d!\n", day);
            return 0;
        }
    }

    printf("\033[?25h\nThey walked through the same city for years, but the day they said goodbye was the last time they ever saw each other.\n");
    return 0;
}