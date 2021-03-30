#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define dot_dev "/dev/dot"
#define fnd_dev   "/dev/fnd"

#define KEY_NUM1    1
#define KEY_NUM2    2
#define KEY_NUM3    3
#define KEY_NUM4    4
#define KEY_NUM5    5
#define KEY_NUM6    6
#define KEY_NUM7    7
#define KEY_NUM8    8
#define KEY_NUM9    9
#define KEY_NUM10   10
#define KEY_NUM11   11
#define KEY_NUM12   12
#define     MAXCHR  32

char lcdDev[] = "/dev/clcd";
char tactswDev[] = "/dev/tactsw";

int  tactswFd = (-1);
int dot_fd = (-1);
int  lcdFd = (-1);
int fnd_fd = 0;

int tactValue;
int tryValue;
int count = 1000;
int num0 = 0;
int num1 = 0;
int num2 = 0;
int num3 = 0;

time_t start;

unsigned char fnd_num[4];
unsigned char bit_sum[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char state[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char _dotValue[8] = { 8, 8, 8, 8, 8, 8, 8, 8};
unsigned char _dotValue2[16] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};

unsigned char row[8][8] = {
    {128, 128, 128, 128, 128, 128, 128, 128}, //1열
    {64, 64, 64, 64, 64, 64, 64, 64},         //2열
    {32, 32, 32, 32, 32, 32, 32, 32},         //3열
    {16, 16, 16, 16, 16, 16, 16, 16},         //4열
    {8, 8, 8, 8, 8, 8, 8, 8},                 //5열
    {4, 4, 4, 4, 4, 4, 4, 4},                 //6열
    {2, 2, 2, 2, 2, 2, 2, 2},                 //7열
    {1, 1, 1, 1, 1, 1, 1, 1},                 //8열
};

unsigned char square[16][8] = {
    {192, 192, 0, 0, 0, 0, 0, 0}, //왼쪽 위부터 오른쪽으로 2x2사각형
    {48, 48, 0, 0, 0, 0, 0, 0},
    {12, 12, 0, 0, 0, 0, 0, 0},
    {3, 3, 0, 0, 0, 0, 0, 0},
    {0, 0, 192, 192, 0, 0, 0, 0},
    {0, 0, 48, 48, 0, 0, 0, 0},
    {0, 0, 12, 12, 0, 0, 0, 0},
    {0, 0, 3, 3, 0, 0, 0, 0},
    {0, 0, 0, 0, 192, 192, 0, 0},
    {0, 0, 0, 0, 48, 48, 0, 0},
    {0, 0, 0, 0, 12, 12, 0, 0},
    {0, 0, 0, 0, 3, 3, 0, 0},
    {0, 0, 0, 0, 0, 0, 192, 192},
    {0, 0, 0, 0, 0, 0, 48, 48},
    {0, 0, 0, 0, 0, 0, 12, 12},
    {0, 0, 0, 0, 0, 0, 3, 3}
};

unsigned char FND_DATA_TBL[11]={
   0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xD8, 0x80, 0x98, 0xFF
   // 0,1,2,3,4,5,6,7,8,9,default
};

// dot operator
void dot_clear(){
    int i;
    for(i = 0; i < 8; i++){
        state[i]=0;
    }
}


void displayDot_mode1(int num1, int num2, int num3)
{    
    int i = 0;
    
    for (i = 0; i < 8; i++)
    {
        //인자로 받은 값들의 행별로 비교하고 비교한 배열을 or연산을 통해 비트열 합치기
        bit_sum[i] = row[num1][i] | row[num2][i] | row[num3][i];
    }

    for (i = 0; i < 8; i++)
    {
        //합친 비트열을 가진 배열과 현재 상태인 배열을 행별로 xor연산
        state[i] = state[i] ^ bit_sum[i];
    }

};


void displayDot_mode2(int num1, int num2, int num3)
{
    int i = 0;

    for (i = 0; i < 8; i++)
    {
        //인자로 받은 값들의 행별로 비교하고 비교한 배열을 or연산을 통해 비트열 합치기
        bit_sum[i] = square[num1][i] | square[num2][i] | square[num3][i];
    }

    for (i = 0; i < 8; i++)
    {
        //합친 비트열을 가진 배열과 현재 상태인 배열을 행별로 xor연산
        state[i] = state[i] ^ bit_sum[i];
    }
};

// i/o
void close_io()
{
    if (dot_fd >= 0)
    {
        close(dot_fd);
        dot_fd = -1;
    }
    if (tactswFd >= 0)
    {
        close(tactswFd);
        tactswFd = -1;
    }
    if (lcdFd >= 0)
    {
        close(lcdFd);
        lcdFd = -1;
    }
    if (fnd_fd >= 0)
    {
        close(fnd_fd);
        fnd_fd = -1;
    }
};

unsigned char tactsw_get(int tmo)
{   
    unsigned char b;
    
   if (tmo) { 
           if (tmo < 0)
                  tmo = ~tmo * 1000;
           else
                  tmo *= 1000000;

           while (tmo > 0) {
                  usleep(10000);
                  read(tactswFd, &b, sizeof(b));
                if (b) return(b);
                     tmo -= 10000;
           }
           return(-1); 
       }
       else {

            read(tactswFd, &b, sizeof(b));
           return(b);
   }
};

int inputTactSw() 
{    
    close_io();
    tactswFd = open(tactswDev, O_RDONLY);

    while(1)
    {
       unsigned char c;
    
       c = tactsw_get(10);
    
       switch (c) {
          case KEY_NUM1:  return 1; break;
      case KEY_NUM2:  return 2; break;
      case KEY_NUM3:  return 3; break;
      case KEY_NUM4:  return 4; break;
      case KEY_NUM5:  return 5;  break;
      case KEY_NUM6:  return 6; break;
      case KEY_NUM7:  return 7; break;
      case KEY_NUM8:  return 8; break;
      case KEY_NUM9:  return 9; break;
      case KEY_NUM10:  return 10; break;
      case KEY_NUM11:  return 11; break;
      case KEY_NUM12:  return 12; break;
      default: ;

       }
    }
    
    close_io();
};

void tact_read(){

    unsigned char buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char b;
    int i;

    for(i = 0; i < 8; i++){
        buf[i] = _dotValue[i];
        printf("%d,  ", buf[i]);
    }

    close_io();

    tactswFd = open(tactswDev, O_RDONLY);
    printf("tact read\n");  
    read(tactswFd, &b, sizeof(b));
    
    switch (b)
    {
    case 1:
        printf("1!!\n");  
        count--;
        displayDot_mode1(buf[0], buf[1], buf[2]);
        break;
        
    case 2:
        printf("2!!\n");  
        count--;
        displayDot_mode1(buf[3], buf[4], buf[5]);
        break;
        
    case 3:
        printf("3!!\n");  
        count--;
        displayDot_mode1(buf[6], buf[7], buf[0]);
        break;
        
    case 4:
        printf("4!!\n");  
        count--;
        displayDot_mode1(buf[0], buf[0], buf[0]);
        break;      
    case 11:
        dot_clear();
        count--;
        break;         
    case KEY_NUM12:
        close(dot_fd);
        close(tactswFd);
        exit(1);
        break;
    default:
        break;
    }

    close_io();
};

void tact_read2()
{

    unsigned char buf[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char b;
    int i;

    for (i = 0; i < 16; i++)
    {
        buf[i] = _dotValue2[i];
        printf("%d,  ", buf[i]);
    }

    close_io();

    tactswFd = open(tactswDev, O_RDONLY);
    printf("tact read\n");
    read(tactswFd, &b, sizeof(b));

    switch (b)
    {
    case 1:
        printf("1!!\n");
        count--;
        displayDot_mode2(buf[0], buf[1], buf[2]);
        break;

    case 2:
        printf("2!!\n");
        count--;
        displayDot_mode2(buf[3], buf[4], buf[5]);
        break;

    case 3:
        printf("3!!\n");
        count--;
        displayDot_mode2(buf[6], buf[7], buf[8]);
        break;

    case 4:
        printf("4!!\n");
        count--;
        displayDot_mode2(buf[9], buf[10], buf[11]);
        break;

    case 5:
        printf("5!!\n");
        count--;
        displayDot_mode2(buf[12], buf[13], buf[14]);
        break;

    case 6:
        printf("6!!\n");
        count--;
        displayDot_mode2(buf[15], buf[0], buf[0]);
        break;

    case 7:
        printf("7!!\n");
        count--;
        displayDot_mode2(buf[0], buf[0], buf[0]);
        break;

    case 8:
        printf("8!!\n");
        count--;
        displayDot_mode2(buf[0], buf[0], buf[0]);
        break;
    case 11:
        dot_clear();
        count--;
        break;

    case KEY_NUM12:
        close(dot_fd);
        close(tactswFd);
        exit(1);
        break;
    default:
        break;
    }

    close_io();
};

void dot_write(){
    unsigned char dot_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int i;

    for(i = 0; i<8; i++){
        dot_data[i] = state[i];
    }

    close_io();

    dot_fd = open(dot_dev, O_RDWR);
    memcpy(dot_data, dot_data, 8);
    printf("dot write\n");
    write(dot_fd, &dot_data, sizeof(dot_data)); //현재 상태 쓰기
    usleep(300000);
    close_io();
};

void clcd_write(char *s)
{
    close_io();

    int n;
    char buf[MAXCHR];

    n = strlen(s);

    lcdFd = open( lcdDev, O_RDWR);

    memset(buf, 0, sizeof(buf));
    memcpy(buf, s, n);
    write(lcdFd, buf, MAXCHR);

    close_io();

}

void clcd_count_write()
{
    close_io();

    char s[] = "score : ";
    char s1[10];
    int n;
    char buf[MAXCHR];
    
    sprintf(s1, "%d", count);

    strcat(s, s1);

    n = strlen(s);

    lcdFd = open( lcdDev, O_RDWR);

    memset(buf, 0, sizeof(buf));
    memcpy(buf, s, n);
    write(lcdFd, buf, MAXCHR);

    close_io();
}

void fnd_write(){

    close_io();

    fnd_fd = open(fnd_dev, O_RDWR);
    time_t end = time(NULL);

    int re = (int) (end - start);

    int i;
    printf("                      %d                 \n", re);
    for(i=0; i<10; i++){
        num0 = re/1000;           // 천의 자리
        num1 = (re/100)%10;       // 백의 자리
        num2 = (re/10)%10;        // 십의 자리
        num3 = re%10;
    }
   
    fnd_num[0] = FND_DATA_TBL[num0];
    fnd_num[1] = FND_DATA_TBL[num1];
    fnd_num[2] = FND_DATA_TBL[num2];
    fnd_num[3] = FND_DATA_TBL[num3];

    write(fnd_fd, &fnd_num, sizeof(fnd_num));
    usleep(300000);

    close_io();
}


// paly game

int isDotOn()
{
    int dotOnValue = 0;
    int i;
    for (i = 0; i < 8; i++)
    {
        if (state[i] == 255)
        {
            dotOnValue++;
        }
    }

    if (dotOnValue == 8)
    {
        printf("dotOn\n");
        return 0;
    }else{
        return 1;
    }
};

void randDot()
{
   int index = 0;
    int sameValue = 0;
    int i = 0;
    
    srand(time(NULL));
    
    while(index < 8)
    {
       int value = rand() % 8;
       for (i = 0; i < 8; i++)
       {
            if (_dotValue[i] == value)
            {
               sameValue++;
            }
       }
       
       if (sameValue == 0)
       {
            _dotValue[index] = value;
            index++;
       }
       
       sameValue = 0;
    }
}

void randDot2()
{
   int index = 0;
    int sameValue = 0;
    int i = 0;
    
    srand(time(NULL));
    
    while(index < 16 )
    {
       int value = rand() % 16;
       for (i = 0; i < 16; i++)
       {
            if (_dotValue2[i] == value)
            {
               sameValue++;
            }
       }
       
       if (sameValue == 0)
       {
            _dotValue2[index] = value;
            index++;
       }
       
       sameValue = 0;
    }
}

void playMode1()
{
    int keep = 1;
    randDot();
    sleep(1);
    start = time(NULL);
    while (keep)
    {
        keep = isDotOn();
        tact_read();
        dot_write();
        clcd_count_write();
        fnd_write();
    }
};

void playMode2()
{
    int keep = 1;
    randDot2();
    sleep(1);
    start = time(NULL);
    while (keep)
    {
        keep = isDotOn();
        tact_read2();
        dot_write();
        clcd_count_write();
        fnd_write();        
    }
};

// print start
void startMenuDisplay() {
    clcd_write("Please enjoy our BULB GAME");
    sleep(2);
    clcd_write("made by team    DISCORD");
    sleep(2);
    clcd_write("Press TactSW to select mode");
    sleep(2);
    clcd_write("(1) easy_mode   (2) hard_mode");

};

int main()
{
    int mode = -1;
    startMenuDisplay();
    while(1){
        mode=inputTactSw();
        if (mode == 1 || mode == 2){
            break;
        }
    }

    if(mode == 1)
    {
        playMode1();
    }else if(mode == 2)
    {
        playMode2();
    }

    sleep(3);
    

    
    return 0;
};



