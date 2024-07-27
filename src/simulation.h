// This file contains the entire class for the conway's game of life simulation
#include <Adafruit_Protomatter.h>

// Define how big of a game arena we want
#define gameRows 32 // Total number of rows in the matrix
#define gameColumns 64 // Total number of colors in the matrix

// Class definition
class ConwaysGame {
    private:
        uint16_t simColor;
        Adafruit_Protomatter* matrix;
        byte genMap_1[gameRows][gameColumns]; // Used to store the current and next generations
        byte genMap_2[gameRows][gameColumns];
        byte (*currentGenMap)[gameRows][gameColumns];
        byte (*nextGenMap)[gameRows][gameColumns];

        // Checks all the neighbors of a specific cell
        // and returns true if its alive or false if its dead.
        bool checkNeighbors(uint8_t y, uint8_t x, 
                            uint8_t totalRows, uint8_t totalColumns);
    public:
        // Matrix only constructor
        ConwaysGame(Adafruit_Protomatter* disp);
        ConwaysGame(Adafruit_Protomatter* disp, uint16_t color);
        uint32_t calcNextGen();
        void initSeed(boolean rand); 
        void drawCurGen();
        void setColor(uint16_t color);
};

// Class implementation

// Matrix only constructor, by default the simulation
// is the color red
ConwaysGame::ConwaysGame(Adafruit_Protomatter* disp){
    matrix = disp;
    simColor = matrix->color565(255,0,0);
};

// Overloaded constructor, ability to change the color 
// of the simulation.
ConwaysGame::ConwaysGame(Adafruit_Protomatter* disp,uint16_t color){
    matrix = disp;
    simColor = color;
};

// Sets the seed of the simulation, aka the initial gane of life pattern
// on to the initial current generaton map.
// If the parameter is set to true, it will set a random pattern on every
// startup
void ConwaysGame::initSeed(boolean rand){

    if(rand){
        randomSeed(analogRead(26));
        for(int i=0;i<gameRows;i++){
            for(int j=0;j<gameColumns;j++){
                genMap_1[i][j]=(random(2)==1) ? true : false;
                genMap_2[i][j]=false;
            }
        }
    }else{
        // Clear both arrays and load hardcoded pattern
        for(int i=0;i<gameRows;i++){
            for(int j=0;j<gameColumns;j++){
                genMap_1[i][j]=(random(2)==1) ? true : false;
                genMap_2[i][j]=false;
            }
        }
        // Cordinates are in y,x for the arrays
        // due to the way two dimensional matrices work.
        uint8_t centerY = (gameRows/2)-1;
        uint8_t centerX = (gameColumns/2)-1;
        genMap_1[centerY-1][centerX-1] = 1;
        genMap_1[centerY-1][centerX] = 1;
        genMap_1[centerY-1][centerX+1] = 1;
        genMap_1[centerY][centerX-1] = 1;
        genMap_1[centerY][centerX+1] = 1;
        genMap_1[centerY+1][centerX-1] = 1;
        genMap_1[centerY+1][centerX] = 1;
        genMap_1[centerY+1][centerX+1] = 1;
        // above is the basic square pattern

        genMap_1[centerY][centerX+2] = 1;
        genMap_1[centerY][centerX-2] = 1;
        genMap_1[centerY+4][centerX] = 1;
        genMap_1[centerY-4][centerX] = 1;
    }

    currentGenMap = &genMap_1;
};

// Calculates the next generation of the game based on the current generation.
// Uses an internal map to keep track of the current and next generation.
// Returns the number of cells that are "updated" aka change state
// between the current game map and the next one
uint32_t ConwaysGame::calcNextGen(){

    uint32_t updates = 0; // Returns the number of updates done to the matrix
                    // Used as a way to reset the matrix if the pattern becomes
                    // "static"


    // Alternate between the maps to store the "next"
    // generation. One map will always contain the
    // "previous" which is the one shown in the LED matrix
    // while the other map will be calculated based on the one shown.
    if(currentGenMap == &genMap_1){
        // If the "current" map is genMap_1
        // set the "next" map to be genMap_2
        nextGenMap = &genMap_2;
    }else{
        nextGenMap = &genMap_1;
    }

    // Check if the cell meets the conditions to be
    // alive, if so light up the LED, if not, turn it off.
    for(int i=0;i<gameRows;i++){
        for(int j=0;j<gameColumns;j++){
            if(checkNeighbors(i,j,gameRows,gameColumns)){
                (*nextGenMap)[i][j] = true;
            }else{
                (*nextGenMap)[i][j] = false;
            }
            // See if the state of the current cell changed,
            // if so increase update count.
            if((*currentGenMap)[i][j] != (*nextGenMap)[i][j]){
                updates++;
            }
        }
    }
    // Point to the right current map
    currentGenMap = nextGenMap;

    return updates;
};

// Internal helper function to check all the living neighbors of a cell
// and return if the cell being checked is alive or dead. 
bool ConwaysGame::checkNeighbors(uint8_t y, uint8_t x, uint8_t totalRows, uint8_t totalColumns){
    // Checks all neighbors of a cell, if they are out of bounds
    // do not check them and just ignore them
    
    // calculate the number of live neighbors
    // and the top left corner to start
    uint8_t liveNeighbors = 0;
    int yTopLeft = y-1;
    int xTopLeft = x-1;

    // This algorithm works by first calculating the
    // top left neighbor (with the one being checked in the center)
    // and then iterating through the 9 cells that make up
    // the "square" that needs to be checked for each cell.
    // Remember that a neighbor is a cell that is right next
    // to any cell or directly diagonal to it.
    /*
    Algorithm starts here->  xxx
                             xXx
                             xxx

    with big X being the actual cell being checked.
    Then as the for loop iterates it adds offset to the
    Y and X cords relative to the top left neighbor of the
    cell being checked
    
    */
    for(int i=0;i<9;i++){

        // Calculate the y and x offsets 
        // relative to the top left neighbor
        int curYLoc =  yTopLeft+ i/3;
        int curXLoc = xTopLeft + i%3;

        // Negative values are out of bounds!
        if((curYLoc)<0 || (curXLoc)<0){
            continue;
        }
        // Out of the screen cords are out of bounds!
        if(curYLoc>=totalRows||curXLoc>=totalColumns){
            continue;
        }
        // Skip ourselves
        if(curXLoc==x && curYLoc==y){
            continue;
        }
        // Check if the neighbor is alive
        if( (*currentGenMap)[curYLoc][curXLoc] ){
            liveNeighbors++;
        }

    }


    // Evaluates the rules of the game based
    // on the state of the cell being checked
    bool currentCellState = (*currentGenMap)[y][x];
    if(currentCellState){
        if(liveNeighbors<2){
            return false;
        }
        if(liveNeighbors>3){
            return false;
        }
        return true;
    }else{
        if(liveNeighbors==3){
            return true;
        }
        return false;
    }

};

// Draws the current 'gameMap' to the LED matrix, must be called after
// calcNextGen function is called to properly draw the current generation on to the 
// LED matrix. This functionality could be combined with the calcNextGen function
// but for clarity I keep it seperate. 
void ConwaysGame::drawCurGen(){
    // Draws the current gameMap (also called the current generation)
    // on to the LED matrix.
    for(int i=0;i<gameRows;i++){
        for(int j=0;j<gameColumns;j++){
            if( (*currentGenMap)[i][j] ){
                // turn the LED to the simulations color
                matrix->drawPixel(j,i,simColor);
            }else{
                // turns the LED off
                matrix->drawPixel(j,i,matrix->color565(0,0,0));
            }

        }
    }
};

// Takes an unsigned 16 bit number and sets the internal
// color variable to it, used to change the color of
// the LED matrix animation.
void ConwaysGame::setColor(uint16_t color){
    simColor = color;
};