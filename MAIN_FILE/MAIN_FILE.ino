#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include "Picopixel.h"
#include "Font_5x7_practical8pt7b.h"

/*--------------------- MATRIX PANEL CONFIG -------------------------*/
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 3      // Total number of panels chained one to another

MatrixPanel_I2S_DMA *display = nullptr;

// Module configuration
HUB75_I2S_CFG mxconfig(
  PANEL_RES_X,   // module width
  PANEL_RES_Y,   // module height
  PANEL_CHAIN    // Chain length
);

uint16_t myDARK = display->color565(64, 64, 64);
uint16_t myWHITE = display->color565(192, 192, 192);
uint16_t myRED = display->color565(255, 0, 0);
uint16_t myGREEN = display->color565(0, 255, 0);
uint16_t myBLUE = display->color565(0, 0, 255);
uint16_t myYELLOW = display->color565(196, 180, 84);
uint16_t myVIOLET = display->color565(127, 0, 255);

uint16_t colours[5] = { myDARK, myWHITE, myRED, myGREEN, myBLUE };
int DELAY = 1500;
int DELAY1 = 20000;

int n = 0;
void setup() {
  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(115200);
  delay(200);

  Serial.println("...Starting Display");
  //mxconfig.double_buff = true; // Turn of double buffer
  mxconfig.clkphase = false;
  mxconfig.gpio.e = 32;
  // OK, now we can create our matrix object
  display = new MatrixPanel_I2S_DMA(mxconfig);
  display->begin();  // setup display with pins as pre-defined in the library
  display->setFont(&Font_5x7_practical8pt7b);

}

// volatile int R0 = 0, R1 = 0, R2 = 0;

void loop() {
  display->flipDMABuffer(); // not used if double buffering isn't enabled
  delay(25);
  display->clearScreen();

  drawLayout(); // Draw the blocks
  drawDefaultDataFlowPaths(); //Default White Signals
  drawDefaultControlSignals(); //Defaukt Yellow Control Signals

  printVariableNames();

  lightUpInstructions();

  display->setCursor(130, 2);
  display->setTextColor(display->color565(15,15,15));
  display->println("RGB");
}

void printVariableNames() {

  // PC
  display->setCursor(130, 35);
  display->setTextColor(display->color565(0,255,0));
  display->print("PC:0x");

  display->setCursor(130, 45);
  display->print("R0:");

  display->setCursor(160, 45);
  display->print("R1:");

  display->setCursor(145, 55);
  display->print("R2:");
}

// DRAW LAYOUT
void drawLayout() {
  display->drawRect(11,27,4,6,myRED); // PC
  display->drawRect(17,24,14,17,myRED); // IM
  display->drawRect(37,3,8,17,myBLUE); // CU
  display->drawRect(50,24,21,25,myRED); // RF
  display->drawRect(97,24,15,25,myRED); // DM
  drawMux(5,27,5,33,7,31,7,29); // PCSrc
  drawMux(37,26,37,32,39,30,39,28); // RegSrc[0]
  drawMux(39,33,39,39,41,37,41,35); // RegSrc[1]
  drawMux(74,34,74,41,76,39,76,36); // ALUSrc
  drawMux(117,34,117,41,119,39,119,36); // MemToReg
  drawMux(52,56,52,59,57,59,57,51); // ExtImm
  drawALU(17,45,17,50,18,51,18,52,17,53,17,57,21,53,21,49); // PC + 4
  drawALU(36,42,36,45,37,46,37,48,36,49,36,52,39,49,39,45); // PC + 8
  drawALU(85,27,85,31,86,32,86,33,85,34,85,38,89,34,89,31); // ALUControl
  display->drawLine(32,5,32,60,myRED); // Instruction Bus
  drawMux(92,32,92,38,94,36,94,34); //SrcB
}

void drawMux(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
  display->drawLine(x1, y1, x2, y2, myRED);
  display->drawLine(x2, y2, x3, y3, myRED);
  display->drawLine(x3, y3, x4, y4, myRED);
  display->drawLine(x4, y4, x1, y1, myRED);
}

void drawALU(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5, int x6, int y6, int x7, int y7, int x8, int y8) {
  display->drawLine(x1, y1, x2, y2, myRED);
  display->drawLine(x2, y2, x3, y3, myRED);
  display->drawLine(x3, y3, x4, y4, myRED);
  display->drawLine(x4, y4, x5, y5, myRED);
  display->drawLine(x5, y5, x6, y6, myRED);
  display->drawLine(x6, y6, x7, y7, myRED);
  display->drawLine(x7, y7, x8, y8, myRED);
  display->drawLine(x8, y8, x1, y1, myRED);
}

// DRAW DATAPATH
struct Point {
  int x, y;
};

struct Line {
  Point p1, p2;
};

class DataPath {
    //
  public:
    Line* lines = nullptr;
    int numPoints;
    uint16_t COLOR;
    
    DataPath(){

    };
    DataPath(int numPoints) { 
      this->numPoints = numPoints;
      this->lines = new Line[numPoints-1];
      this->COLOR = myWHITE;
    }
    ~DataPath() { delete [] lines; }
    
    // Copy Constructor
    DataPath(DataPath const& copy) {
        numPoints  = copy.numPoints;
        COLOR = copy.COLOR;
        lines = new Line[copy.numPoints-1];
        std::copy(&copy.lines[0],&copy.lines[copy.numPoints-1],lines);
    }

    // "=" Operator overloading
    DataPath& operator=(DataPath rhs) {
        rhs.swap(*this);
        return *this;
    }

    void swap(DataPath& s) noexcept {
        using std::swap;
        swap(this->lines,s.lines);
        swap(this->numPoints,s.numPoints);
        swap(this->COLOR,s.COLOR);
    }

    void makeDataPath(Point* pointsForDataPath){
      for(int i = 0; i < this->numPoints-1; i++) {
        if(pointsForDataPath[i].x == -1) break;
        this->lines[i].p1 = pointsForDataPath[i];
        this->lines[i].p2 = pointsForDataPath[i+1];
        display->drawLine(this->lines[i].p1.x, this->lines[i].p1.y, this->lines[i].p2.x, this->lines[i].p2.y, this->COLOR);
      }
    }

    void setColor(uint16_t COLOR){
      this->COLOR = COLOR;
      for(int i = 0; i < numPoints-1; i++) {
        display->drawLine(this->lines[i].p1.x, this->lines[i].p1.y, this->lines[i].p2.x, this->lines[i].p2.y, COLOR);
      }
    }
};

const int MAX_POINTS = 7;
const int totalDataPaths = 35;
DataPath* dataPaths = new DataPath[totalDataPaths];

// Data for DataPaths
int numpoints[totalDataPaths] = {4,3,5,2,2,3,2,2,2,2,3,2,2,2,2,2,2,4,2,3,2,2,4,5,4,2,2,2,2,2,6,2,5,2,2};
Point pointsForDataPaths[totalDataPaths][MAX_POINTS] = {
  {{.x=5,.y=29}, {.x=2,.y=29}, {.x=2,.y=62}, {.x=43,.y=62}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 0
  {{.x=43,.y=62}, {.x=43,.y=44}, {.x=50,.y=44}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 1
  {{.x=5,.y=32}, {.x=4,.y=32}, {.x=4,.y=59}, {.x=26,.y=59}, {.x=26,.y=51}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 2
  {{.x=7,.y=30}, {.x=11,.y=30}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 3
  {{.x=14,.y=30}, {.x=17,.y=30}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 4
  {{.x=16,.y=30}, {.x=16,.y=48}, {.x=17,.y=48}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 5
  {{.x=15,.y=55}, {.x=17,.y=55}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 6
  {{.x=21,.y=51}, {.x=36,.y=51}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 7
  {{.x=34,.y=44}, {.x=36,.y=44}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 8
  {{.x=32,.y=41}, {.x=50,.y=41}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 9
  {{.x=36,.y=41}, {.x=36,.y=38}, {.x=39,.y=38}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 10
  {{.x=32,.y=34}, {.x=39,.y=34}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 11
  {{.x=35,.y=31}, {.x=37,.y=31}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 12
  {{.x=32,.y=27}, {.x=37,.y=27}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 13
  {{.x=39,.y=29}, {.x=50,.y=29}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 14
  {{.x=41,.y=36}, {.x=50,.y=36}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 15
  {{.x=32,.y=57}, {.x=52,.y=57}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 16
  {{.x=57,.y=56}, {.x=73,.y=56}, {.x=73,.y=39}, {.x=74,.y=39}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 17
  {{.x=70,.y=36}, {.x=74,.y=36}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 18
  {{.x=72,.y=36}, {.x=72,.y=45}, {.x=97,.y=45}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 19
  {{.x=70,.y=29}, {.x=85,.y=29}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 20
  {{.x=89,.y=33}, {.x=92,.y=33}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 21 **
  {{.x=111,.y=33}, {.x=116,.y=33}, {.x=116,.y=36}, {.x=117,.y=36}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 22
  {{.x=96,.y=35}, {.x=96,.y=51}, {.x=115,.y=51}, {.x=115,.y=39}, {.x=117,.y=39}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 23
  {{.x=119,.y=37}, {.x=124,.y=37}, {.x=124,.y=62}, {.x=43,.y=62}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 24
  {{.x=76,.y=37}, {.x=85,.y=37}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 25
  {{.x=32,.y=7}, {.x=37,.y=7}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 26
  {{.x=32,.y=10}, {.x=37,.y=10}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 27
  {{.x=32,.y=13}, {.x=37,.y=13}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 28
  {{.x=32,.y=16}, {.x=37,.y=16}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 29
  {{.x=89,.y=32}, {.x=91,.y=32}, {.x=91,.y=21}, {.x=34,.y=21}, {.x=34,.y=18}, {.x=37,.y=18}, {.x=-1,.y=-1}}, // 30
  {{.x=94,.y=35}, {.x=97,.y=35}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 31 ** MuxToMem
  {{.x=92,.y=37}, {.x=89,.y=37},  {.x=89,.y=41}, {.x=81,.y=41}, {.x=81,.y=37}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 32 ** SrcB
  {{.x=39,.y=47}, {.x=50,.y=47}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 33 ** R15
  {{.x=30,.y=30}, {.x=32,.y=30}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 34 ** InstMem2Bus
};

void drawDefaultDataFlowPaths() {
  for(int i = 0; i < totalDataPaths; i++) {
    DataPath currDataPath = DataPath(numpoints[i]);
    dataPaths[i] = currDataPath;
    dataPaths[i].makeDataPath(pointsForDataPaths[i]);
  }
}

// DRAW CONTROL SIGNALS 

const int totalControlSignals = 10;
DataPath* control_signals = new DataPath[totalControlSignals];

int numpoints2[totalControlSignals] = {5,3,3,3,3,5,3,2,3,3};
Point pointsForControlSignals[totalControlSignals][MAX_POINTS] = {
  {{.x=44,.y=4}, {.x=48,.y=4}, {.x=48,.y=2}, {.x=6,.y=2}, {.x=6,.y=28}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 0
  {{.x=44,.y=6}, {.x=118,.y=6}, {.x=118,.y=35}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 1
  {{.x=44,.y=8}, {.x=109,.y=8}, {.x=109,.y=24}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 2
  {{.x=44,.y=12}, {.x=87,.y=12}, {.x=87,.y=29}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 3
  {{.x=44,.y=14}, {.x=75,.y=14}, {.x=75,.y=35}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 4
  {{.x=44,.y=16}, {.x=71,.y=16}, {.x=71,.y=50}, {.x=55,.y=50}, {.x=55,.y=53}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 5
  {{.x=44,.y=18}, {.x=58,.y=18}, {.x=58,.y=24}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 6
  {{.x=40,.y=19}, {.x=40,.y=34}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 7
  {{.x=40,.y=23}, {.x=38,.y=23}, {.x=38,.y=27}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 8
  {{.x=44,.y=10}, {.x=87,.y=10}, {.x=87,.y=29}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}, {.x=-1,.y=-1}}, // 3
};

void drawDefaultControlSignals() {
  for(int i = 0; i < totalControlSignals; i++) {
    control_signals[i] = DataPath(numpoints2[i]);
    control_signals[i].makeDataPath(pointsForControlSignals[i]);
    control_signals[i].setColor(myYELLOW);
  }
}

// DRAW DATAPATH FOR EACH INSTRUCTIONS

const int MAX_DATAPATHS = totalDataPaths;
const int numIns = 6;
int dataPathForInstruction[numIns][MAX_DATAPATHS] = {
  {3,4,5,16,17,25,32,23,24,1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // MOV
  {3,4,5,16,17,25,32,23,24,1,-1,-1,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // MOV
  {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, //ADD
  {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // CMP 1
  {3,4,5,7,2,13,14,11,9,10,15,18,19,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // STR 
  {12,16,14,17,20,25,21,23,24,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},// B
  
  // {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // SUB
  // {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // SUB
  // {12,16,14,17,20,25,21,23,24,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},// B
  // {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // CMP 2
  // {12,16,14,17,20,25,21,23,24,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},// BEQ
  // {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, //ADD
  // {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // SUB
  // {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // SUB
  // {12,16,14,17,20,25,21,23,24,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},// B
  // {3,4,5,7,2,13,14,11,15,9,20,21,31,18,25,23,24,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}, // CMP 3
  // {12,16,14,17,20,25,21,23,24,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},// BEQ
  
};
const char* INS[numIns] = {"MOV  ", "MOV  ", "ADD  ", "CMP  ", "STRGT", "B    "};
const char* REG[numIns] = {"R0,#4      ", "R1,#7      ", "R2,#0      ", "R3,#0      ", "R2,[R1,#8]", "DONE       " };

const int MAX_CONTROL_SIGNALS = totalControlSignals;
int controlSignalsForInstruction[numIns][MAX_CONTROL_SIGNALS] = {
  {0,1,2,-1,-1,-1,-1,-1,-1},
  {3,4,5,-1,-1,-1,-1,-1,-1},
};

int R[3][numIns] = {
  {4,4,4,4,4,4},
  {0,7,7,7,7,7},
  {0,0,11,11,11,11}
};

void lightUpInstructions() {
  for(int i = 0; i < numIns; i++) {
    display->setCursor(160, 35);
    display->setTextColor(display->color565(255,0,0));
    display->print(4*i, HEX);

    display->setCursor(145, 45);
    display->print(R[0][i]);

    display->setCursor(175, 45);
    display->print(R[1][i]);

    display->setCursor(160, 55);
    display->print(R[2][i]);
    
    // display->setCursor(145, 30);

    for(int j = 0; j < MAX_CONTROL_SIGNALS; j++) {
      if(controlSignalsForInstruction[i][j] == -1) break;
      control_signals[controlSignalsForInstruction[i][j]].setColor(myVIOLET);
    }

    for(int j = 0; j < MAX_DATAPATHS; j++) {
      if(dataPathForInstruction[i][j] == -1) break;
      dataPaths[dataPathForInstruction[i][j]].setColor(myGREEN);

      display->setCursor(130, 10);
      display->setTextColor(display->color565(255,0,0));
      display->print(INS[i]);

      display->setCursor(130, 20);
      display->print(REG[i]);
      display->setCursor(130, 30);



      // display->println(R1);

      delay(DELAY);
    }

    // // Clearing
    delay(DELAY1);
    display->setCursor(160, 35);
    display->setTextColor(display->color565(0,0,0));
    display->print(4*i, HEX);
    display->setCursor(145, 45);
    display->print(R[0][i]);

    display->setCursor(175, 45);
    display->print(R[1][i]);

    display->setCursor(160, 55);
    display->print(R[2][i]);

    for(int j = 0; j < MAX_CONTROL_SIGNALS; j++) {
      if(controlSignalsForInstruction[i][j] == -1) break;
      control_signals[controlSignalsForInstruction[i][j]].setColor(myYELLOW);
    }

    for(int j = 0; j < MAX_DATAPATHS; j++) {
      if(dataPathForInstruction[i][j] == -1) break;
      dataPaths[dataPathForInstruction[i][j]].setColor(myWHITE);

      display->setCursor(130, 10);
      display->setTextColor(display->color565(0,0,0));
      display->print(INS[i]);

      display->setCursor(130, 20);
      display->print(REG[i]);
      display->setCursor(130, 30);

      // display->println(R1);

      // delay(200);
    }
    delay(DELAY); //inter-ins
    // n = n + 1;
    // if (n > 1)
    // DELAY = 300;
  }

}


/////////////////// CODE /////////////////////////////

/*

MOV R0, #4; {4, 0, 0}
MOV R1, #7; {4, 7, 0}
ADD R2, R1, R0; {4, 7, 11}
CMP R2, #0 {4, 7, 11}
STRGT R2, [R1, #8]; {4,7,11}
B DONE {4,7,11}

*/


