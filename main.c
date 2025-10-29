#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define FILENAME "payrolls.dat"

//Global Variables
int editPath[2] = {0, 0}; bool editSw = false; int onEditMode = false; float recentEditTime = 0; bool afterEdit = false;
char logger[100] = "Made by John Rushell!";
//Search variables
char searchString[30] = ""; bool searchFocus = false; float snap2 = 0; float curBlink = 0.5;
int searchStringCount = 0; 

//Structures
typedef struct stack {
  int per_w; int per_h;
  char innerString[150];
  int editable;
} Stack;

typedef struct {
    int emp_no;
    char emp_name[50];
    float rate_per_day;
    int days_worked;
    float sss_contrib;
    float philhealth_contrib;
    float cash_advance;
    float gross_pay;
    float total_deduction;
    float net_pay;
} Payroll;

//Functions Declarations
void stackable(Rectangle stackable_rect, Stack stack_list[], int stack_count, float thickness, Font font_, int font_size, Color Bg, int stack_index);
void displayRecords(Stack modify_list[][10], int * payroll_count);
void addRecord(int *type_mode, int *step, char input_string[], char add_guide[], int *input_count, Payroll *modify, bool *isSuccess);
void updateRecord(int id, int row_index, char modify_value[], bool *success);
void deleteRecord(int employee_number);
int getNextEmpNo();
void computePayroll(Payroll *p);
//Gui Elements
void stackedPayrolls(Stack payroll_list[][10], int payroll_count, Rectangle rect, Font font, int * scroll_value, bool *refresh_stack);
void buttonText(char inner_text[], Font font_fam, float font_size, Vector2 position, Vector2 padding, Color bg_color);
void buttonTextRound(bool *clicked, char inner_text[], Font font_fam, float font_size, Vector2 position, Vector2 padding, float round_percentage, Color bg_color);
Rectangle textBoxRound(char inner_text[], Font font_fam, float font_size, Vector2 position, Vector2 padding, float round_percentage, Color bg_color);
void drawSearch(Vector2 position, Font font, float font_size, char place_holder[]);
void universalInput(int *char_pressed, int *type_count, char string_typed[]);
//Typing modes
void onlyNums(int * char_pressed, int * type_count, char string_typed[]);
void onlyLetters(int *char_pressed, int *type_count, char string_typed[]);


int main(){
  //Init window
  const int screenHeight = 600, screenWidth = 1000;
  //SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidth, screenHeight, "Payroll Management UI(Valmoria)");
  SetTargetFPS(30);

  //Variables
  Font raleway = LoadFontEx("ass/raleway.ttf", 200, 0, 0);
  Font montserrat = LoadFontEx("ass/montserrat.ttf", 280, 0, 0);
  Color inColor1 = {230, 230, 230, 255};

  //Elements
  float mtX = (GetScreenWidth() * 0.5) - ( (GetScreenWidth() * 0.85) / 2 ); float mtY = (GetScreenHeight() * 0.05);
  float mtW = (GetScreenWidth() * 0.85); float mtH = (GetScreenHeight() * 0.8);
  Rectangle tableBack = {mtX, mtY, mtW, mtH};
  Stack tableCategories[10] = {
    {7, 100, "Emp.\nNo."}, {20, 100, "Employee\nName"}, {8, 100, "Rate/\nDay"}, {6, 100, "Days"}, {15, 100, "Gross Pay"},
    {6, 100, "SSS"}, {7, 100, "Phil\nHealth"}, {6, 100, "C/A"}, {10, 100, "Total\nDeduction"}, {15, 100, "NetPay"}
  };

  //Payroll stuffs
  Stack payrollList[100][10];
  int payrollCount = 0;
  displayRecords(payrollList, &payrollCount);
  int scrollPayrollsValue = 0; bool refreshBool1 = false;

  //Buttons switch
  bool createBtnSw = false;
  bool cancelBtnSw = false;
  bool nextBtnSw = false;

  //Type mode when editing or creating
  char stringType[100] = "";
  int typeCount = 0; int stringTypeFontSize = 50;
  int typingMode = 0; //0 is non. 1 is only numbers. 2 only letters. 3 is normal input.  
  Vector2 typeSize = MeasureTextEx(montserrat, TextFormat("%s", stringType), stringTypeFontSize, 0);

  //In create window variables
  Payroll payrollFill; bool payrollAddSuccess = false; bool isCreate = false;
  int createStep = 0; char createGuide[50] = "";

  //Time shits
  float snap1 = GetTime(); float cursorDelay = 0.5;
  snap2 = GetTime();

  while(!WindowShouldClose()){
    //Handle input of stringType
    int keyGet = GetCharPressed();
    if(typingMode > 0){
      //Modes
      if(typingMode == 1) onlyNums(&keyGet, &typeCount, stringType);
      if(typingMode == 2) onlyLetters(&keyGet, &typeCount, stringType);
      //Backspace
      if(IsKeyPressed(KEY_BACKSPACE) && typeCount > 0){
        typeCount--;
        stringType[typeCount] = '\0';
      }
      searchFocus = false;
    } else if(typingMode == 0){
        strcpy(stringType, "");
        typeCount = 0;
    }
    if(searchFocus){
        universalInput(&keyGet, &searchStringCount, searchString);
    }
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) searchFocus = false;

    //States Handle (For edit mode)
    if(editSw){
      if(editPath[1] == 1) strcpy(createGuide, "Modify employee name:");
      if(editPath[1] == 2) strcpy(createGuide, "Modify rate per day:");
      if(editPath[1] == 3) strcpy(createGuide, "Modify days:");
      if(editPath[1] == 5) strcpy(createGuide, "Modify SSS:");
      if(editPath[1] == 6) strcpy(createGuide, "Modify PhilHealth:");
      if(editPath[1] == 7) strcpy(createGuide, "Modify Cash advance:");

      if(editPath[1] == 1) typingMode = 2;
      if(editPath[1] >= 2) typingMode = 1;
      createStep = 1;
      onEditMode = true;
      editSw = false;
    }

    //Buttons Handle
    if(createBtnSw && typingMode == 0){
      typingMode = 1; createStep = 1;
      addRecord(&typingMode, &createStep, stringType, createGuide, &typeCount, &payrollFill, &payrollAddSuccess);
      isCreate = true;
      createBtnSw = false;
    } else if(cancelBtnSw){
      typingMode = 0;
      createStep = 0;
      onEditMode = false;
      recentEditTime = GetTime(); afterEdit = true;
      strcpy(logger, "Canceled the creating or editing method.");
      cancelBtnSw = false;
    } else if(nextBtnSw){
      createStep++;
      if(isCreate && !onEditMode) addRecord(&typingMode, &createStep, stringType, createGuide, &typeCount, &payrollFill, &refreshBool1);
      if(onEditMode && createStep == 2){
        updateRecord(editPath[0], editPath[1], stringType, &refreshBool1);
        typingMode = 0;
        createStep = 0;
        recentEditTime = GetTime(); afterEdit = true;
        onEditMode = false;
      }
      nextBtnSw = false;
    }

    //Reload data of payrolls
    if(IsKeyPressed(KEY_R) && typingMode == 0){
      displayRecords(payrollList, &payrollCount);
    }
    if(IsKeyPressed(KEY_DOWN) && typingMode == 0){
      scrollPayrollsValue++;
      displayRecords(payrollList, &payrollCount);
    } else if(IsKeyPressed(KEY_UP) && typingMode == 0){
      scrollPayrollsValue--;
      displayRecords(payrollList, &payrollCount);
    }

    if(payrollAddSuccess){
      displayRecords(payrollList, &payrollCount);
      typingMode = 0;
      createStep = 0;
      isCreate = false;
      onEditMode = false;
      strcpy(logger, "Data created.");
      payrollAddSuccess = false;
    }
    if(refreshBool1){
      displayRecords(payrollList, &payrollCount);
      printf("Data refreshed.");
      refreshBool1 = false;
    }

    typeSize = MeasureTextEx(montserrat, TextFormat("%s", stringType), stringTypeFontSize, 0);
    //Draw
    BeginDrawing();

    ClearBackground((Color){191, 191, 191, 255});
    DrawRectangleRec(tableBack, inColor1);
    DrawRectangleLinesEx(tableBack, 5, BLACK);
    stackable((Rectangle){mtX, mtY, mtW, 50}, tableCategories, 10, 1.6, raleway, 16, (Color){199, 244, 255, 255}, -1);
    stackedPayrolls(payrollList, payrollCount, (Rectangle){mtX, mtY+50, mtW, 44}, montserrat, &scrollPayrollsValue, &refreshBool1);
    DrawTextEx(raleway, "To scroll press the 'Up' and 'Down' button in your keyboard", (Vector2){95, 482}, 15, 0, BLACK);
    drawSearch((Vector2){530, 525}, montserrat, 22, "To search by name type here!");
    buttonTextRound(&createBtnSw, "Create", raleway, 25, (Vector2){100, 530}, (Vector2){25, 25}, 0.6, (Color){33, 194, 76, 255});

    //Input box
    if(typingMode != 0){
      DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 215});
      //Blinking cursor cause I'm fancy like that
      if(GetTime() - snap1 >= cursorDelay){
        DrawRectangleRec((Rectangle){(screenWidth/2.0f)+(typeSize.x/2.0f)+5,
                         (screenHeight/2.0f)-(30), 5, 60}, WHITE);
        if(GetTime() - snap1 >= cursorDelay*2) snap1 = GetTime();
      }

      DrawTextEx(montserrat, TextFormat("%s", stringType), (Vector2){(screenWidth/2.0f) - (typeSize.x/2.0f),
                 (screenHeight/2.0f) - (typeSize.y/2.0f)}, stringTypeFontSize, 0, WHITE);
      DrawTextEx(montserrat, createGuide, (Vector2){15, 15}, 27, 0, GREEN);
      if(strlen(stringType) > 0) buttonTextRound(&nextBtnSw, "Next", raleway, 20, (Vector2){600, 530},
                                                 (Vector2){25, 25}, 0.6, (Color){31, 191, 65, 255});
      buttonTextRound(&cancelBtnSw, "Cancel", raleway, 20, (Vector2){700, 530}, (Vector2){25, 25}, 0.6, (Color){201, 6, 6, 255});
    }
    //Logger for shits
    DrawTextEx(montserrat, logger, (Vector2){(screenWidth - MeasureTextEx(montserrat, logger, 20, 0).x) - 10, 575}, 20, 0, RED);

    EndDrawing();
  }
  UnloadFont(raleway);
  CloseWindow();
  return 0;
}


//Function definitions
//Table functions
void stackable(Rectangle stackable_rect, Stack stack_list[], int stack_count, float thickness, Font font_, int font_size, Color Bg, int stack_index){
  Vector2 mousePos = GetMousePosition();
  float offsetX = 0;
  for(int i = 0; i < stack_count; i++){
    //Declare rectangle
    Rectangle stac_k = {stackable_rect.x + offsetX, stackable_rect.y,
                        (stack_list[i].per_w * 0.01) * stackable_rect.width,
                        (stack_list[i].per_h * 0.01) * stackable_rect.height };

    offsetX += (stack_list[i].per_w * 0.01) * stackable_rect.width;
    // Green when currently editing.
    if(editPath[0] == atoi(stack_list[0].innerString) && editPath[1] == i && onEditMode) DrawRectangleRec(stac_k, GREEN);
    // Yellow when editing and after editing.
    float delayAfterEdit = 2.3;
    bool editCond1 = (editPath[0] == atoi(stack_list[0].innerString) && editPath[1] == i && GetTime()-recentEditTime <= delayAfterEdit);
    if(editCond1 && afterEdit){
      DrawRectangleRec(stac_k, YELLOW);
    } else if(GetTime()-recentEditTime > delayAfterEdit){ afterEdit = false; }

    DrawRectangleRec(stac_k, Bg);
    DrawRectangleLinesEx(stac_k, thickness, BLACK);
    DrawTextEx(font_, stack_list[i].innerString, (Vector2){stac_k.x+10, stac_k.y+10}, font_size, 0, BLACK);

    bool cond1 = (i == 1 || i == 2 || i == 3 || i == 5 || i == 6 || i == 7);
    bool isRectClicked = (CheckCollisionPointRec(mousePos, stac_k) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
    if(isRectClicked && cond1 && stack_index >= 0){
      editSw = true;
      editPath[0] = atoi(stack_list[0].innerString); editPath[1] = i;
    }
  }
}

//Data managing
void displayRecords(Stack modify_list[][10], int * payroll_count){
    *payroll_count = 0;
    memset(modify_list, 0, sizeof(Stack) * 100 * 10);
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp) {
        return;
    }
    int column = 0;
    Payroll p;
    while (fread(&p, sizeof(Payroll), 1, fp)) {
        //Values
        Stack val0 = {7, 100}; strcpy(val0.innerString, TextFormat("%d", p.emp_no));
        Stack val1 = {20, 100}; strcpy(val1.innerString, p.emp_name);
        Stack val2 = {8, 100}; strcpy(val2.innerString, TextFormat("%.2f", p.rate_per_day));
        Stack val3 = {6, 100}; strcpy(val3.innerString, TextFormat("%d", p.days_worked));
        Stack val4 = {15, 100}; strcpy(val4.innerString, TextFormat("%.2f", p.gross_pay));
        Stack val5 = {6, 100}; strcpy(val5.innerString, TextFormat("%d", (int)p.sss_contrib));
        Stack val6 = {7, 100}; strcpy(val6.innerString, TextFormat("%d", (int)p.philhealth_contrib));
        Stack val7 = {6, 100}; strcpy(val7.innerString, TextFormat("%d", (int)p.cash_advance));
        Stack val8 = {10, 100}; strcpy(val8.innerString, TextFormat("%.2f", p.total_deduction));
        Stack val9 = {15, 100}; strcpy(val9.innerString, TextFormat("%.2f", p.net_pay));

        //Append values
        modify_list[column][0] = val0;
        modify_list[column][1] = val1;
        modify_list[column][2] = val2;
        modify_list[column][3] = val3;
        modify_list[column][4] = val4;
        modify_list[column][5] = val5;
        modify_list[column][6] = val6;
        modify_list[column][7] = val7;
        modify_list[column][8] = val8;
        modify_list[column][9] = val9;
        column += 1;
    }

    *payroll_count = column;
    printf("Payroll data loaded.\n");
    fclose(fp);
}
int getNextEmpNo() {
    FILE *fp = fopen(FILENAME, "rb");
    if (!fp) return 1;

    Payroll p;
    int last = 0;
    while (fread(&p, sizeof(Payroll), 1, fp))
        last = p.emp_no;

    fclose(fp);
    return last + 1;
}
void addRecord(int *type_mode, int *step, char input_string[], char add_guide[], int *input_count, Payroll *modify, bool *isSuccess){
    modify->emp_no = getNextEmpNo();
    if(*step == 1){ *type_mode = 2; strcpy(add_guide, "Enter Employee Name:"); }
    if(*step == 2){
      strcpy(modify->emp_name, input_string);
      modify->emp_name[strcspn(modify->emp_name, "\n")] = 0;
      strcpy(input_string, ""); *input_count = 0;
      *type_mode = 1; strcpy(add_guide, "Enter Rate per Day:");
    }
    if(*step == 3){ modify->rate_per_day = atof(input_string); strcpy(add_guide, "Enter Number of Days Worked:"); strcpy(input_string, ""); *input_count = 0; }
    if(*step == 4){ modify->days_worked = atoi(input_string); strcpy(add_guide, "Enter SSS Contribution:"); strcpy(input_string, ""); *input_count = 0; }
    if(*step == 5){ modify->sss_contrib = atof(input_string); strcpy(add_guide, "Enter PhilHealth Contribution:"); strcpy(input_string, ""); *input_count = 0; }
    if(*step == 6){ modify->philhealth_contrib = atof(input_string); strcpy(add_guide, "Enter Cash Advance:"); strcpy(input_string, ""); *input_count = 0; }
    if(*step == 7){
      modify->cash_advance = atof(input_string); strcpy(input_string, "");
      *input_count = 0; computePayroll(modify);
      //Open file
      FILE *fp = fopen(FILENAME, "ab");
      if (!fp) {
          printf("Error opening file!\n");
          strcpy(logger, "Error opening a file.");
          *type_mode = 0;
          *step = 0;
      }
      fwrite(modify, sizeof(Payroll), 1, fp);
      fclose(fp);
      strcpy(add_guide, "Record added successfully!");
      strcpy(logger, TextFormat("Added a new record of %s.", modify->emp_name));
      *isSuccess = true;
      *type_mode = 0; *step = 0;
    }
}
void deleteRecord(int employee_number){
    FILE *fp = fopen(FILENAME, "rb");
    FILE *temp = fopen("temp.dat", "wb");

    if (!fp || !temp) {
        printf("Error opening file!\n");
        return;
    }

    int found = false;
    Payroll p;
    while(fread(&p, sizeof(Payroll), 1, fp)){
        if(p.emp_no == employee_number){
            found = true;
            printf("Record for %s deleted.\n", p.emp_name);
            strcpy(logger, TextFormat("Employee '%s' data has been deleted", p.emp_name));
        } else {
            fwrite(&p, sizeof(Payroll), 1, temp);
        }
    }
    fclose(fp);fclose(temp);
    remove(FILENAME);
    rename("temp.dat", FILENAME);
    if (!found) printf("Employee not found!\n");
}
void updateRecord(int id, int row_index, char modify_value[], bool *success){
  FILE *fp = fopen(FILENAME, "rb+");
  if (!fp) {
      printf("No records found!\n");
      return;
  }
  bool found = false;
  Payroll p;

  while (fread(&p, sizeof(Payroll), 1, fp)) {
      if (p.emp_no == id) {
          found = true;
          if(row_index == 1) strcpy(p.emp_name, modify_value);
          if(row_index == 2) p.rate_per_day = atof(modify_value);
          if(row_index == 3) p.days_worked = atoi(modify_value);
          if(row_index == 5) p.sss_contrib = atof(modify_value);
          if(row_index == 6) p.philhealth_contrib = atof(modify_value);
          if(row_index == 7) p.cash_advance = atof(modify_value);

          computePayroll(&p);

          fseek(fp, -(long)sizeof(Payroll), SEEK_CUR);
          fwrite(&p, sizeof(Payroll), 1, fp);
          printf("Record updated successfully!\n");
          strcpy(logger, TextFormat("Edited the record of %s.", p.emp_name));
          printf("Modify Value: %s", modify_value);
          break;
      }
  }

  if (!found){ printf("Employee not found!\n"); *success = false; } else { *success = true; }
  fclose(fp);
}
void computePayroll(Payroll *p) {
    p->gross_pay = p->rate_per_day * p->days_worked;
    p->total_deduction = p->sss_contrib + p->philhealth_contrib + p->cash_advance;
    p->net_pay = p->gross_pay - p->total_deduction;
}


void stackedPayrolls(Stack payroll_list[][10], int payroll_count, Rectangle rect, Font font, int * scroll_value, bool *refresh_stack){
  Vector2 mousePos = GetMousePosition();
  //Stackables per column
  if(payroll_count <= 9){
    for(int i_col = 0; i_col < payroll_count; i_col++){
      float offsetY = rect.y + (rect.height * i_col);
      Rectangle columnRect = {rect.x, offsetY, rect.width, rect.height};
      //Draw stackable
      stackable(columnRect, payroll_list[i_col], 10, 1.6, font, 18, (Color){0, 0, 0, 0}, i_col);
      Rectangle deleteRect = textBoxRound("Del", font, 13, (Vector2){53, offsetY+15},
                                          (Vector2){10, 10}, 0.3, RED);
      if(CheckCollisionPointRec(mousePos, deleteRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        deleteRecord(atoi(payroll_list[i_col][0].innerString));
        *refresh_stack = true;
      }
    }
  } else {
    if(*scroll_value + 8 >= payroll_count) *scroll_value = (payroll_count-1) - 8;
    if(*scroll_value < 0) *scroll_value = 0;
    for(int i_col = 0; i_col < 9; i_col++){
      int curIndex = i_col + *scroll_value;
      float offsetY = rect.y + (rect.height * i_col);
      Rectangle columnRect = {rect.x, offsetY, rect.width, rect.height};
      //Draw stackable
      stackable(columnRect, payroll_list[curIndex], 10, 1.6, font, 18, (Color){0, 0, 0, 0}, curIndex);
      Rectangle deleteRect = textBoxRound("Del", font, 13, (Vector2){53, offsetY+15},
                                          (Vector2){10, 10}, 0.3, RED);
      if(CheckCollisionPointRec(mousePos, deleteRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        deleteRecord(atoi(payroll_list[curIndex][0].innerString));
        *refresh_stack = true;
      }
    }
  }
}

//Normal Button
void buttonText(char inner_text[], Font font_fam, float font_size, Vector2 position, Vector2 padding, Color bg_color){
  Vector2 textSize = MeasureTextEx(font_fam, inner_text, font_size, 0);
  Rectangle rectBack = {position.x - (padding.x / 2), position.y - (padding.x / 2), textSize.x + padding.x, textSize.y + padding.y};
  //Draw lol
  DrawRectangleRec(rectBack, bg_color);
  DrawTextEx(font_fam, inner_text, position, font_size, 0, WHITE);
}
//Rounded Button
void buttonTextRound(bool *clicked, char inner_text[], Font font_fam, float font_size, Vector2 position, Vector2 padding, float round_percentage, Color bg_color){
  Vector2 mousePos = GetMousePosition();
  Vector2 textSize = MeasureTextEx(font_fam, inner_text, font_size, 0);
  Rectangle rectBack = {position.x - (padding.x / 2), position.y - (padding.x / 2), textSize.x + padding.x, textSize.y + padding.y};
  //Check if clicked
  if(CheckCollisionPointRec(mousePos, rectBack) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){ *clicked = true; } else { *clicked = false; }
  DrawRectangleRounded(rectBack, round_percentage, 20, bg_color);
  DrawTextEx(font_fam, inner_text, position, font_size, 0, WHITE);
}
Rectangle textBoxRound(char inner_text[], Font font_fam, float font_size, Vector2 position, Vector2 padding, float round_percentage, Color bg_color){
  Vector2 textSize = MeasureTextEx(font_fam, inner_text, font_size, 0);
  Rectangle rectBack = {position.x - (padding.x / 2), position.y - (padding.x / 2), textSize.x + padding.x, textSize.y + padding.y};
  //Check if clicked
  DrawRectangleRounded(rectBack, round_percentage, 20, bg_color);
  DrawTextEx(font_fam, inner_text, position, font_size, 0, WHITE);
  return rectBack;
}
void drawSearch(Vector2 position, Font font, float font_size, char place_holder[]){
  //mark1
  Vector2 textSizePlaceholder = MeasureTextEx(font, place_holder, font_size, 0);
  Vector2 textSizeString = MeasureTextEx(font, searchString, font_size, 0);
  Rectangle searchBarRect = {position.x-5, position.y-5, 400, textSizePlaceholder.y+10};
  DrawRectangleLinesEx(searchBarRect, 2,BLACK);
  DrawTextEx(font, searchString, position, font_size, 0, BLACK);
  //Detect if search bar clicked
  bool isClicked = (CheckCollisionPointRec(GetMousePosition(), searchBarRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
  if(isClicked){
    searchFocus = true;
  }
  if(strlen(searchString) <= 0) DrawTextEx(font, place_holder, position, font_size, 0, (Color){0, 0, 0, 50});
  //Cursor blink again
  if(GetTime() - snap2 >= curBlink && searchFocus){
    DrawRectangleRec((Rectangle){position.x + textSizeString.x,
                                position.y, 3, font_size}, BLACK);
    if(GetTime() - snap2 >= curBlink*2) snap2 = GetTime();
  }
}


//Typing modes
void onlyNums(int *char_pressed, int *type_count, char string_typed[]){
  while(*char_pressed > 0){
    if((*char_pressed >= 48) && (*char_pressed <= 57) && (*type_count < 6)){
      string_typed[*type_count] = (char)*char_pressed;
      (*type_count)++;
      string_typed[*type_count] = '\0';
    }
    *char_pressed = GetCharPressed();
  }
}
void onlyLetters(int *char_pressed, int *type_count, char string_typed[]){
  while(*char_pressed > 0){
    //conditions
    bool con1 = ((*char_pressed >= 'A' && *char_pressed <= 'Z') || (*char_pressed >= 'a' && *char_pressed <= 'z'));
    bool con2 = (*char_pressed == ' ');

    if((con1 || con2) && (*type_count < 23)){
      string_typed[*type_count] = (char)*char_pressed;
      (*type_count)++;
      string_typed[*type_count] = '\0';
    }
    *char_pressed = GetCharPressed();
  }
}
void universalInput(int *char_pressed, int *type_count, char string_typed[]){
  while(*char_pressed > 0){
    //conditions
    bool con1 = ((*char_pressed >= 'A' && *char_pressed <= 'Z') || (*char_pressed >= 'a' && *char_pressed <= 'z'));
    bool con2 = (*char_pressed == ' ');
    bool con3 = ((*char_pressed >= 48) && (*char_pressed <= 57));

    if((con1 || con2 || con3) && (*type_count < 28)){
      string_typed[*type_count] = (char)*char_pressed;
      (*type_count)++;
      string_typed[*type_count] = '\0';
    }
    *char_pressed = GetCharPressed();
  }
  if(IsKeyPressed(KEY_BACKSPACE) && type_count > 0){
    (*type_count)--;
    string_typed[*type_count] = '\0';
  } else if(IsKeyDown(KEY_LEFT) && IsKeyDown(KEY_BACKSPACE)){
    *type_count = 0;
    strcpy(string_typed, "");
  }
}

//String methods
bool matchString(const char *text, const char *pattern) {
    // Case-insensitive, partial match
    if (!text || !pattern) return false;
    char lowerText[256], lowerPattern[256];
    int i;
    // Convert both strings to lowercase
    for (i = 0; text[i] && i < 255; i++)
        lowerText[i] = tolower((unsigned char)text[i]);
    lowerText[i] = '\0';
    for (i = 0; pattern[i] && i < 255; i++)
        lowerPattern[i] = tolower((unsigned char)pattern[i]);
    lowerPattern[i] = '\0';
    // Check if pattern is contained anywhere in text
    return strstr(lowerText, lowerPattern) != NULL;
}
