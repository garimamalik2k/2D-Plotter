#include <SD.h>
#include <SPI.h>
#include <Stepper.h>
#include <Servo.h>
#include <math.h>


  File myFile;

  const int stepsPerRevolution = 64; 
  const int ReturnSpeed=2;
  int Zup = 10;
  int Zdown = 5;
  int Z_pin = 16;
  int SDpin = 4;
  const int TransferConst; //the motion transfer constant linking the rpm of motor and cm/min of the X and Y axis
  float oldX=0; 
  float oldY=0; 
  int Speed = 2; //Speed
  String Mode;

  Stepper XStepper(stepsPerRevolution, 8, 9, 10, 11);
  Stepper YStepper(stepsPerRevolution, 12, 13, 14, 15);
  Servo ZaxisPen;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  ZaxisPen.attach(Z_pin);
  ZaxisPen.write(Zup);

  if (!SD.begin(SDpin)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  

}

void loop() {

  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt");
  
  // if the file opened okay, write to it:
  if (!myFile) {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
  if (myFile) {
    Serial.println("test.txt:");
    
    // read from the file until there's nothing else in it:
    ReadMultipleLinesofCode();
    
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

/*
 * Understanding the G-code
 * G20/G21: units selection
 * G17/G18/G19: plane selection (XY,XZ,YZ)
 * G28: return home
 * G90/G91: absolute mode/relative mode
 * F___: feed rate
 * G0: move to co-ordinates with the pen up
 * G01: move to the designated co-ordinates
 * G02/G03: Move to co-ordinates in a circular manner with a given center of rotation
 * 
 * 
 * Code:
 * Read in the file
 * Read next string UnitSelec (G20/G21), next string Plane (G17/G18/G19), next string Mode (G90/G91), next character FeedRate (If 'F', next int: use it to set the speed of the program) 
 * Plotting:
 * While (there is something to be read)
 * {
 * read in the first character. If its G00/G01: read in the next character (X), next int (X-pos), next char (Y), next int (Y-pos)
 * If its G01: put the pen down 
 * If G00/G01: Send the pen to the new position in linear way - function
 * If its G02/G03: read in the next character ('I'), next int (X-pos of radius), next char ('J'), next int (Y-pos of radius)
 * Put the pen down
 * If G02: send the pen to the new position in clockwise direction in circular way - function
 * If G03: send the pen to the new position in anticlockwise direction in circular way - function
 * Set the new coordinates
 * }
 * 
 */

//To convert from string to number: subtract '0'
//byteRead = byteRead-'0'
 
}


char readCharSD()
{
  //reading one character from SD card
  char character = Serial.write(myFile.read());
  return character;
}

int readintSD()
{
  int integer = readfloatSD();
  return integer;
}

float readfloatSD()
{
  //read next float in SD card
  /* while character is not ' ' or '.', read character, convert to integer, multiply with its place. 
   *  if its '.', count all the places before it, and calculate the integer value
   *  for the remaining floats, it is a decimal number
   */
  int i=0,j=0,k=0;
  char nextchar[20];
  int integerval=0;
  float floatvalue=0, decimalval=0;
  int val=0;
  int signconstant=1;
  
   do
   {
    i++;
    nextchar[i] = Serial.write(myFile.read()) ; 
    if (nextchar[i]=='-')
    {
      signconstant = -1;
      i=0;
      nextchar[20]=' ';
    }
    else
    nextchar[i] = (Serial.write(myFile.read()) - '0'); //converts char to number
   }while (isSpace(nextchar[i])||nextchar[i]=='.'||nextchar[i]=='\n');
   
   //calculating value of integer
   for (int j=1;j<i;j++)
   {
    val = nextchar[j]*10^(i-j-1);
    integerval+=val;
   }
    //calculating value of decimal (if exists)
    if (nextchar[i]=='.')
    {
      int k=-1;
      bool EON = false;

    while(!EON)
    {
        i++;
        nextchar[i] = (Serial.write(myFile.read()) - '0');
        if (isSpace(nextchar[i])||nextchar[i]=='\n')
        {
          EON=true;
          break;
        }
        decimalval+=(nextchar[i]*10^k);
        k--;
    }
    
    floatvalue = signconstant*(integerval+decimalval);
   }
  return floatvalue;
}

void ReadOneLineofGCode() 
{
  char buffer[20];
int idx = 0;


  bool EOL = false;
  while (! EOL)
  {
    float Xpos, Ypos, Zpos, Xrad, Yrad, Xdist, Ydist, Xdir, Ydir;
    char c = readCharSD();  // reads 1 char from SD
    if (c == '\n' || idx==19)  // prevent buffer overflow too..
    {
      buffer[idx] = 0;
      idx = 0;
      EOL = true;
    }
    else if (c=='(')
    {
      char b;
      do 
      {
        char b = readCharSD();
      } while (b!='\n');
      break;
    }
    else if (c=='%'||c=='\n')
    break;
    else
    {
      buffer[idx] = c;
      idx++;
      char G, M, F;

      //Decoding G01/G02/G03
     switch (c){
      case 'G':
        int G1var = readintSD();
        if (G1var==0)
        {
          int Gvar = readintSD();
        switch (Gvar)
        {
          case 0:
            for(int i=0;i<3;i++)
            {
              char zerovar = readCharSD();
              switch (zerovar)
              {
                case 'X':
                  float Xpos = readfloatSD();
                  break;
                case 'Y':
                  float Ypos = readfloatSD();
                  break;
                case 'Z':
                  float Zpos = readfloatSD();
                  break;

              }
              LinearMovement (oldX, oldY, Xpos, Ypos, Mode, Speed);
               oldX = Xpos;
               oldY = Ypos;
            }
            break;
            
          case 1:
            for(int i=0;i<3;i++)
              {
                char zerovar = readCharSD();
                switch (zerovar)
                {
                  case 'X':
                    float Xpos = readfloatSD();
                    break;
                  case 'Y':
                    float Ypos = readfloatSD();
                    break;
                 case 'Z':
                  float Zpos = readfloatSD();
                  break;
  
                }
                ZaxisPen.write(Zdown);
                LinearMovement (oldX, oldY, Xpos, Ypos, Mode, Speed);
                oldX = Xpos;
                oldY = Ypos;
              }
              break;
              
          case 2:
             for(int i=0;i<3;i++)
              {
                char zerovar = readCharSD();
                switch (zerovar)
                {
                  case 'X':
                    float Xpos = readfloatSD();
                    break;
                  case 'Y':
                    float Ypos = readfloatSD();
                    break;
                  case 'Z':
                  float Zpos = readfloatSD();
                  break;
                }
              }
              
                 for(int i=0;i<2;i++)
              {
                char onevar = readCharSD();
                switch (onevar)
                {
                  case 'I':
                    float Xrad = readfloatSD();
                    break;
                  case 'J':
                    float Yrad = readfloatSD();
                    break;
                }
                
                ZaxisPen.write(Zdown);
                CircularMovement (Gvar, oldX, oldY, Xpos, Ypos, Xrad, Yrad, Mode, Speed);
                oldX = Xpos;
                oldY = Ypos;
              }
              break;
              
          case 3:
            for(int i=0;i<3;i++)
              {
                char zerovar = readCharSD();
                switch (zerovar)
                {
                  case 'X':
                    float Xpos = readfloatSD();
                    break;
                  case 'Y':
                    float Ypos = readfloatSD();
                    break;
                  case 'Z':
                  float Zpos = readfloatSD();
                  break;
                }
              }
              
                 for(int i=0;i<2;i++)
              {
                char onevar = readCharSD();
                switch (onevar)
                {
                  case 'I':
                    float Xrad = readfloatSD();
                    break;
                  case 'J':
                    float Yrad = readfloatSD();
                    break;
                }
                
                ZaxisPen.write(Zdown);
                CircularMovement (Gvar, oldX, oldY, Xpos, Ypos, Xrad, Yrad, Mode, Speed);
                oldX = Xpos;
                oldY = Ypos;
              }
              break;

            case 28:
            ReturnHome(oldX, oldY);
            break;
        }
                
                ZaxisPen.write(Zdown);
                CircularMovement (Gvar, oldX, oldY, Xpos, Ypos, Xrad, Yrad, Mode, Speed);
                float oldX = Xpos;
                float oldY = Ypos;
              }
              break;
        
        
    
     
        
      case 'F':
        float Speed = readfloatSD();
        break;
        
      case 'M':
        int zerovar = readintSD();
          switch(zerovar)
          {
            case 0:
              PauseProgram();
              break;
            case 2:
              ReturnHome(oldX,oldY);
              break;
            case 30:
              ReturnHome(oldX, oldY);
              break;
          }
       
      }
    }
  }
}

void ReadMultipleLinesofCode()
{
  bool EOC = false;
  while (myFile.available())
  {
    ReadOneLineofGCode;
  }
  EOC=true;
}


 void LinearMovement (int oldX, int oldY, int newX, int newY, String Mode, int FeedRate)
 {

  int Xdir, Ydir, Xdist, Ydist;
  
  if (Mode == "G90") //absolute mode
  {
    int Xdist = abs(newX-oldX);
    int Ydist = abs(newY-oldY);
    int Xdir = (newX-oldX)/abs(newX-oldX);
    int Ydir = (newY-oldY)/abs(newY-oldY);
    

  }
  if (Mode == "G91") //relative mode
  { 
    int Xdist = abs(newX);
    int Ydist = abs(newY);
    int Xdir = abs(newX)/newX;
    int Ydir = abs(newY)/newY;
    
  }
    int slope = Ydist/Xdist;
    
    XStepper.setSpeed(FeedRate*TransferConst);
    YStepper.setSpeed(FeedRate*TransferConst);
    
    /*
    Method 1: Moving 1 step at a time in X direction if slope is more than 1, and vice versa
    */
    
    for (int i=1;i<=Xdist;i++)
    {
    
    
    if (slope>=1) 
    {
    XStepper.step(stepsPerRevolution*Xdir);
    YStepper.step(stepsPerRevolution*Ydir*slope);
    }
    else if (slope<1)
    {
      int inverseslope = Xdist/Ydist;
      YStepper.step(stepsPerRevolution*Ydir);
      XStepper.step(stepsPerRevolution*Xdir*inverseslope);
    }
    }
    
    //Move Z pen up
    ZaxisPen.write(Zup);

    
  }
    
void CircularMovement (int CWorCCW, float oldX, float oldY, float newX, float newY, float Xradius, float Yradius, String Mode, int FeedRate)
{
  
int QuadStart, QuadFinish;
float StartAngle, FinishAngle;

//Finding total arc length of travel 

//Step 1: Finding Angle of start and finish position (StartAngle, FinishAngle)
//Start
if ((oldX-Xradius)>0)
{
  if ((oldY-Yradius)>=0)
  {
    QuadStart = 1;
    StartAngle = atan(abs(oldY-Yradius)/abs(oldX-Xradius));
  }
  else 
  {
    QuadStart = 4;
    StartAngle = 360-atan(abs(oldY-Yradius)/abs(oldX-Xradius));
  }
}
else if ((oldX-Xradius)<0)
{
  if ((oldY-Yradius)>=0)
  {
    QuadStart = 2;
    StartAngle = 180-atan(abs(oldY-Yradius)/abs(oldX-Xradius));
  }
  else
  { 
    QuadStart = 3;
    StartAngle = 180+atan(abs(oldY-Yradius)/abs(oldX-Xradius));
  }
}
else if ((oldX-Xradius)==0)
{
  if ((oldY-Yradius)>0)
  {
    QuadStart = 2;
    StartAngle = 90;
  }
  else if ((oldY-Yradius)<0)
  { 
    QuadStart = 3;
    StartAngle = 270;
  }
  else if ((oldY-Yradius)==0)
  { 
    QuadStart = 1;
    StartAngle = 0;
  }  
}


//Finish
if ((newX-Xradius)>0)
{
  if ((newY-Yradius)>=0)
  {
    QuadFinish = 1;
    FinishAngle = atan(abs(newY-Yradius)/abs(newX-Xradius));
  }
  else 
  {
    QuadFinish = 4;
    FinishAngle = 360-atan(abs(newY-Yradius)/abs(newX-Xradius));
  }
}
else if ((newX-Xradius)<0)
{
  if ((newY-Yradius)>=0)
  {
    QuadFinish = 2;
    FinishAngle = 180-atan(abs(newY-Yradius)/abs(newX-Xradius));
  }
  else
  { 
    QuadFinish = 3;
    FinishAngle = 180+atan(abs(newY-Yradius)/abs(newX-Xradius));
  }
}
else if ((newX-Xradius)==0)
{
  if ((newY-Yradius)>0)
  {
    QuadFinish = 2;
    FinishAngle = 90;
  }
  else if ((newY-Yradius)<0)
  { 
    QuadFinish = 3;
    FinishAngle = 270;
  }
  else if ((newY-Yradius)==0)
  { 
    QuadFinish = 1;
    FinishAngle = 0;
  }  
}

//Step 2: Finding angle of travel (AngleTravel)

float AngleDiff = abs(StartAngle-FinishAngle);
float AngleTravel=0;
if (FinishAngle>StartAngle)
{
  if (CWorCCW==2)
  AngleTravel = 360-AngleDiff;
  else if (CWorCCW==3)
  AngleTravel = AngleDiff;
}
else if (FinishAngle<StartAngle)
{
  if (CWorCCW==2)
  AngleTravel = AngleDiff;
  else if (CWorCCW==3)
  AngleTravel = 360-AngleDiff;
}


//Step 3: Finding total arc length (ArcLength)

  float ArcLength = 0, Radius = 0;
  Radius = sqrt(sq(oldX-Xradius)+sq(oldY-Yradius));
  ArcLength = AngleTravel*Radius;

// Step 4: Doing the circular motion

  int finalX=0, finalY=0, Xdir=0, Ydir=0;
  float DistCovered=0, NormalSlope=0;
  
  do {
  
  //Step 4.1: Find slope of radius

  float SlopeCap = 10;
  float XTanDir = oldX - Xradius;
  float YTanDir = oldY - Yradius;
  float TangentSlopeAbs = abs((oldY - Yradius)/(oldX - Xradius));

  //Step 4.2: Find slope of tangent (normal to radius)

  float NormalSlopeAbs = abs(1/TangentSlopeAbs);

  Xdir = abs(YTanDir)/YTanDir;
  Ydir = -1*abs(XTanDir)/XTanDir;
  
  if (CWorCCW==2)
  {
    // nothing changes
  }
  if (CWorCCW==3)
  {
    Xdir = -1*Xdir;
    Ydir = -1*Ydir;
  }

  if (NormalSlope<1)
  {
    int NormalSlopeInverted = 1/NormalSlopeAbs;
    if (NormalSlopeInverted>SlopeCap)
    {
      NormalSlopeInverted=SlopeCap;             //capping the number at SlopeCap
      Ydir=0;
    }
    YStepper.step(stepsPerRevolution*Ydir);
    XStepper.step(stepsPerRevolution*Xdir*NormalSlopeInverted);
    oldX+=NormalSlopeInverted;
    oldY+=1;
    DistCovered+=sqrt(sq(NormalSlopeInverted)+1);
  }

  else if (NormalSlope>1)
  {
    int NormalSlope = NormalSlopeAbs;
    if (NormalSlope>SlopeCap)
    {
      NormalSlope=SlopeCap;                   //capping the number at SlopeCap
      Xdir=0;
    }
    XStepper.step(stepsPerRevolution*Xdir);
    YStepper.step(stepsPerRevolution*Ydir*NormalSlope);
    oldX+=1;
    oldY+=NormalSlope;
    DistCovered+=sqrt(sq(NormalSlope)+1);
  }


  //Step 4: Repeat Steps 1-3 until 0.5 close to final x and y value and 0.5 close to arc length

  } while (abs(oldX-newX)>0.5 || abs(oldY-newY)>0.5 || abs(DistCovered-ArcLength)>1); //total distance travelled ~ arc length

    //Step 4.3: Move Z pen up
    ZaxisPen.write(Zup);
  
  
}

void PauseProgram()
{
  ZaxisPen.write(Zup);
  XStepper.setSpeed(0);
  YStepper.setSpeed(0);
}

void ReturnHome(float endX, float endY)
{
  ZaxisPen.write(Zup);
  XStepper.setSpeed(ReturnSpeed);
  YStepper.setSpeed(ReturnSpeed);
  XStepper.step(-1*endX*stepsPerRevolution);
  YStepper.step(-1*endY*stepsPerRevolution);
  XStepper.setSpeed(0);
  YStepper.setSpeed(0);
  
}
