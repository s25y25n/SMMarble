//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"


#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"



//board configuration parameters

static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;

typedef struct player {
        int energy;
        int position;
        char name[MAX_CHARNAME];
        int accumCredit;
        int exp;
        int flag_graduate;
} player_t;


static player_t *cur_player;
//static player_t cur_player[MAX_PLAYER];

#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif

//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
void generatePlayers(int n, int initEnergy); //generate a new player
void printGrades(int player); //print grade history of the player
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
#endif

static char* lectureHistory[MAX_CHARNAME] = {NULL};

// 수강한 강의 여부를 확인하는 함수를 정의
int isLectureTaken(int player, const char* lectureName) 
{
     int i;
    // 강의 목록을 확인하여 lectureName이 이미 수강한 강의인지 확인
    for (i=0;i<MAX_CHARNAME;++i) 
    {
        if (lectureHistory[i] != NULL && strcmp(lectureHistory[i], lectureName) == 0) 
        {
            return 1;  // 이미 수강한 강의
        }
    }
    return 0;  // 수강하지 않은 강의
}



float calcAverageGrade(int player) {
     int i; 
     float totalGrade = 0.0;
     int numGrades = smmdb_len(LISTNO_OFFSET_GRADE + player);

     for (i=0;i<numGrades;i++)
      {
        void *gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        int gradeIndex = smmObj_getNodeGrade(gradePtr);


        totalGrade += smmGradeValue[gradeIndex];
       }

    return (numGrades > 0) ? (totalGrade / numGrades) : 0.0;
}



int isGraduated(void) //플레이어가 졸업 조건을 충족했는지를 확인하는 함수
{

    int i;
    for (i = 0;i<player_nr;i++) 
    {    // 현재 플레이어의 누적 학점이 졸업 기준 이상인지 확인
        if (cur_player[i].accumCredit >= GRADUATE_CREDIT) 
        {
            return 1; // 졸업 조건 충족
        }
    }
    return 0; //아직 모든 플레이어가 졸업하지 않음
}


void printGrades(int player)
{    
    int i; 
    int gradeCount = smmdb_len(LISTNO_OFFSET_GRADE + player);

    if (gradeCount == 0)
    {
        printf("수강한 강의가 없습니다.\n");
        return;  // 강의가 없으면 더 이상 진행하지 않고 함수를 종료합니다.
    }

    for (i = 0; i < gradeCount; i++)
    {
        void *gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        printf("%s : %f\n",smmObj_getNodeName(gradePtr),smmGradeValue[smmObj_getNodeGrade(gradePtr)]);
    }
     printf(" average : %f\n",calcAverageGrade(player) );
}
 /*
smmObjGrade_e takeLecture(int player, char *lectureName, int credit) 
{
    smmObjLecture* Lecture = (smmObjLecture*)malloc(sizeof(smmObjLecture));
    strncpy(Lecture->name, lectureName, MAX_CHARNAME);
    Lecture->credit = credit;
    
    
    void *gradePtr = smmObj_genObject(lectureName, smmObjType_grade, 0, credit, 0, randomGrade);
    smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
}
*/




void printPlayerStatus(void)//플레이어의 이름, 위치, 총 학점, 에너지를 출력
{
     int i;
     
     for (i=0;i<player_nr;i++)
     {
         void *boardPtr = smmdb_getData(LISTNO_NODE, (cur_player[i].position) % board_nr);
         
         printf("==============================================\n%s at %i.%s, credit %i, energy %i\n==============================================\n", 
                      cur_player[i].name,
                      cur_player[i].position,
                      smmObj_getNodeName(boardPtr),
                      cur_player[i].accumCredit,
                      cur_player[i].energy
                      );
     }
}



void generatePlayers(int n, int initEnergy)//generate a new player
{
     //n time loop
     int i;
     for(i=0;i<n;i++) {
           
           // input name
         printf("Input player %i's name:", i+1); //¾E³≫ ¹?±¸ 
         scanf("%s",cur_player[i].name);
         fflush(stdin);
    
    //set position
    cur_player[i].position= 0;
    
    //set energy
    cur_player[i].energy = initEnergy;
    cur_player[i].accumCredit = 0;
    cur_player[i].flag_graduate = 0;
    cur_player[i].exp == 0;
    }
}


int rolldie(int player) //주사위 값을 출력하고, 'g'를 누르면 플레이어의 성적을 출력
{
    char c;
    printf("\n\n This is %s's turn! ::: Press any key to roll a die (press g to see grade): \n",cur_player[player].name);
    c = getchar();
    fflush(stdin);
    

    if (c == 'g' || c == 'G')
        printGrades(player);

    
    return (rand()%MAX_DIE + 1);
}


//action code when a player stays at a node
void actionNode(int player)
{   
    //현재 위치한 노드의 타입과 정보  
    void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    int type = smmObj_getNodeType(boardPtr);
    char *name = smmObj_getNodeName(boardPtr);
    void* gradePtr; 
    
    switch(type)
{
    case SMMNODE_TYPE_LECTURE: // 강의노드 
    {
        void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
        // 현재 위치의 강의 정보 가져오기
        int currentNodeType = smmObj_getNodeType(boardPtr);
        char* currentLectureName = smmObj_getNodeName(boardPtr);
        
        // 이전에 수강한 강의인지 확인
        int alreadyTaken = isLectureTaken(player, currentLectureName);
        
        if (smmObj_getNodeCredit(boardPtr) > 0 && cur_player[player].energy >= smmObj_getNodeEnergy(boardPtr))
        {
            //선택 여부 입력                                
            char choice;
            int grade = 0;
            int validInput = 0;

            while (!validInput)
            {
                if (alreadyTaken) //이미 수강한 경우  
               {
                 printf("You've already taken the lecture \"%s\".\n", currentLectureName);
                 validInput = 1; //  반복문 종료
                }
                printf("Do you want to take the lecture? (y/n): ");
                scanf(" %c", &choice);

                if (choice == 'y' || choice == 'Y')
                {
                  // 수강하는 경우
                  printf("You've taken the lecture \"%s\".\n", smmObj_getNodeName(boardPtr));
                  // 수강 및 성적 처리  
                  cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
                  cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr);
                  // 플레이어가 졸업 요건을 충족했는지 확인
                   if (cur_player[player].accumCredit >= GRADUATE_CREDIT)
                    {
                      cur_player[player].flag_graduate = 1;
                      printf("%s has accumulated enough credits to graduate!\n", cur_player[player].name);
                     }

                  grade = rand() % SMMGRADE_MAX;
                  gradePtr = smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit(boardPtr), 0, (smmObjGrade_e)grade);
                  smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
                  
                  /*성적 이력에 추가하기 
                  lectureObj = smmObj_genObject(name, 0, 0, smmObj_getNodeCredit(boardPtr), 0, (smmObjGrade_e)grade);
                  smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
                  */
                  printf("Grade for \"%s\": %s (%f)\n", smmObj_getNodeName(boardPtr),smmGradeName[grade],smmGradeValue[grade]);

                  validInput = 1; // 유효한 입력이므로 반복문 종료
                 }
                 else if (choice == 'n' || choice == 'N')
                 {
                     // 드랍하는 경우
                    printf("You've dropped the lecture \"%s\".\n", smmObj_getNodeName(boardPtr));
                    validInput = 1; // 유효한 입력이므로 반복문 종료
                  }
            else
                 {
                 // 다른 값 입력 시 처리
                printf("Invalid choice. Please enter 'y' or 'n'.\n");
                 }
              }
          }
    else
        { // 에너지 부족시 메시지 출력  
        printf("Not enough energy to take the lecture.\n");
        }
    break;
   }
   
   case SMMNODE_TYPE_RESTAURANT: // 식당노드  
   {   
        int remainer = 0 ;
        void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
        printf("You have reached a restaurant!\n");
        // 현재 플레이어의 에너지에 더하기
        remainer = cur_player[player].energy += smmObj_getNodeEnergy(boardPtr); 
        printf(" Let's eat in %s and charge %d energies. (remained energy : %i)\n",smmObj_getNodeName(boardPtr),smmObj_getNodeEnergy(boardPtr),remainer);
        
        break;   
   }

    case SMMNODE_TYPE_LABORATORY: // 실험실노드  
{
    // 실험 상태인지 확인
    if (cur_player[player].exp == 1)
    {
        int energyCost = 3; // 실험 시도마다 소요되는 에너지
        int diceResult = rolldie(player);

        if (cur_player[player].energy >= energyCost) // 충분한 에너지가 있는지 확인
        {
            int escapeValue = rolldie(player);

            if (diceResult > escapeValue)
            {   // 실험 성공
                printf("Experiment successful! Moving to home.\n");
                cur_player[player].position = SMMNODE_TYPE_HOME; // 실험 성공 시 홈 노드로 이동
                cur_player[player].exp = 0;  // 실험 상태 해제
            }
            else
            {
                printf("Experiment failed! Staying in the laboratory.\n");
            }
            cur_player[player].energy -= energyCost;
        }
        else
        {
            printf("Not enough energy to attempt the experiment.\n");
        }
        
        break;
    }
    else
    {
        printf("This is not experiment time. You can go through this lab.\n");
    }
    break;
}

    case SMMNODE_TYPE_HOME:
   { //지나가는 순간 에너지 보충 
      
       printf(" returned to HOME! energy charged by %d\n", INITENERGY);
       cur_player[player].energy += INITENERGY;
       break;
   }
     
    case SMMNODE_TYPE_GOTOLAB:
{
        int escapeValue = abs(rolldie(player) - 2);
        printf("OMG! This is experiment time!!\n");
        printf("Escape value for the experiment: %d\n", escapeValue);
        cur_player[player].position = SMMNODE_TYPE_LABORATORY; // 실험실 노드로 이동
        cur_player[player].exp = 1; // 실험 상태로 설정  
    break;
}
       
    case SMMNODE_TYPE_FOODCHANCE:
   {
        printf("You've got a food chance! Press any key to draw a food card: ");
        int userInput;
        scanf("%d", &userInput);
        //음식카드뽑기  
        FoodCard *foodCard = (FoodCard *)smmdb_getData(LISTNO_FOODCARD, rand() % food_nr);
        
        printf("You've drawn a food card: %s (get energy :%d)\n", smmObj_getFoodName(foodCard),smmObj_getFoodEnergy(foodCard));
        //에너지추가  
        cur_player[player].energy += smmObj_getFoodEnergy(foodCard);
        
       
        break;
   }
     
    case SMMNODE_TYPE_FESTIVAL:
   {
        printf("You've reached a festival! Press any key to draw a festival card: ");
        int userInput;
        scanf("%d", &userInput);
        //축제카드뽑기  
        FestivalCard *festCard = (FestivalCard *)smmdb_getData(LISTNO_FESTCARD, rand() % festival_nr);
        printf("%s\n", smmObj_getFestName(festCard));
      
        break; 
   }

    default:
        break;
    }
   
   
}




void goForward(int player, int step)
{
     int i;
     printf("result: %d\n",step);
     for (i = 0; i<=step; ++i)
    {
        // 주사위로 이동한 노드의 포인터를 구함
        void *boardPtr = smmdb_getData(LISTNO_NODE, (cur_player[player].position + i) % board_nr);
        printf("  => Jump to %s\n", smmObj_getNodeName(boardPtr));
    }
       // 플레이어의 위치 업데이트
     cur_player[player].position += step ;
     
     // 마지막 노드에 도달한 경우 보드를 한 바퀴 돈 것으로 처리
   if (cur_player[player].position >= board_nr)
    {
        void *firstBoardNode = smmdb_getData(LISTNO_NODE, 0);
        cur_player[player].energy += smmObj_getNodeEnergy(firstBoardNode);
        cur_player[player].position %= board_nr;
    }
     // 플레이어의 현재 위치에 해당하는 노드 정보 출력
     void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
     printf("%s go to node %i(name: %s)\n", cur_player[player].name, cur_player[player].position, 
     smmObj_getNodeName(boardPtr));
     
     // SMMNODE_TYPE_HOME 지날 때 조건 처리
    if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_HOME)
    {
    // 플레이어의 상태가 졸업 상태인지 확인하고, 졸업 상태라면 게임 종료
        if (cur_player[player].flag_graduate == 1)
        {
            printf("%s has graduated! Game over.\n", cur_player[player].name);
            exit(0);
        }
        if (cur_player[player].position != step) 
        {                     
             cur_player[player].energy += INITENERGY;                           
        }
     #if 0   
     //실험시간 노드일 때 
     if(smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_GOTOLAB )
      {
        int escapeValue = rolldie(player);
          printf("OMG! This is experiment time!!\n");
          printf("Escape value for the experiment: %d\n", escapeValue);
       }
    #endif
    }
     
}




int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i,j,k;
    int initEnergy;
    int turn = 0;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    FILE *logFile = fopen("logfile.txt", "a"); // "a"는 기존 파일에 이어서 쓰기를 의미

    if (logFile != NULL) {
        fprintf(logFile, "This is a log message.\n");

        // ... 프로그램 실행 ...

        fprintf(logFile, "Another log message.\n");

        fclose(logFile);
    } else {
        printf("Error opening the log file.\n");
    }
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
   printf("Reading board component......\n");
    while (fscanf(fp, "%s%i%i%i", name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        smmObjType_e objtype = smmObjType_board;
        smmObject_t *boardObj = (smmObject_t*)smmObj_genObject(name, objtype , type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj);
        
        if(type == SMMNODE_TYPE_HOME)
            initEnergy = energy;
            board_nr++;
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
   
     for (i = 0;i<board_nr;i++)
{
     void* boardObj = smmdb_getData(LISTNO_NODE, i);
        printf("node %i : %s, %i(%s), credit %i, energy %i\n", 
                     i, smmObj_getNodeName(boardObj), 
                     smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                     smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
}
   // printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_LECTURE));
   
    
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s%d", name, &energy) == 2) //read a food parameter set
    {
        //store the parameter set
        FoodCard *foodObj = (FoodCard *)malloc(sizeof(FoodCard));
          strcpy(foodObj->name, name);
          foodObj->energy = energy;
        smmdb_addTail(LISTNO_FOODCARD, foodObj);
        food_nr++;
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);
    
    for (j = 0; j<food_nr; j++)
{
    FoodCard *foodCard = (FoodCard *)smmdb_getData(LISTNO_FOODCARD, j);
    printf("%i. name: %s, energy: %i\n", j, smmObj_getFoodName(foodCard), smmObj_getFoodEnergy(foodCard));
}
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", name) == 1) //read a festival card string
    {
       //store the parameter set
        FestivalCard *festObj = (FestivalCard *)malloc(sizeof(FestivalCard));
          strcpy(festObj->name, name);
        smmdb_addTail(LISTNO_FESTCARD, festObj);
        festival_nr++;
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);
  
   for (k = 0; k<festival_nr; k++)
{
    FestivalCard *festCard = (FestivalCard *)smmdb_getData(LISTNO_FESTCARD, k);
    printf("%i. %s\n ", k, smmObj_getFestName(festCard));
}
   

    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
           printf("\n\n\n*********************************************************************\n=====================================================================\n                          Sookmyung Marble\n\n                          Let's Graduate!!!\n=====================================================================\n*********************************************************************\n\n\n");
           printf("\nHow many people are playing? :\n");
           scanf("%d", &player_nr);
           fflush(stdin);
    } while (player_nr < 0 || player_nr > MAX_PLAYER);
     
     cur_player = (player_t*)malloc(player_nr*sizeof(player_t));
     generatePlayers(player_nr, initEnergy);
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (!isGraduated()) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        //rolldie 함수 사용 
        
        if(cur_player[turn].exp == 1)
        {
           die_result = 0;
        }
        else
        {
            die_result = rolldie(turn);
        }
        
        //4-3. go forward
        goForward(turn, die_result);

		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        turn = (turn+1)%player_nr;
        
        if (cur_player[turn].flag_graduate == 1)
        {
           printf("%s has graduated!\n", cur_player[turn].name);
        
           /* Iterate through the grades of the player and print information
           void *gradePtr = smmdb_getHead(LISTNO_OFFSET_GRADE + i);
           while (gradePtr != NULL)
           {
            printf("   - Lecture: %s, Credit: %d, Grade: %s\n",
                   smmObj_getNodeName(gradePtr),
                   smmObj_getNodeCredit(gradePtr),
                   smmGradeName[smmObj_getNodeGrade(gradePtr)]);

            gradePtr = smmdb_getNext(gradePtr);
            }*/
         }
     
     }
    //5. Game End
    printf("Game has ended!\n");
    

   free(cur_player);
   
   return 0;
}

