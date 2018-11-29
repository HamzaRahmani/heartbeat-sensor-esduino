// filename ******** Main.C ************** 

//***********************************************************************
// Hamza Rahmani 
//  FINAL PROJECT
//***********************************************************************

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"  /* derivative information */
#include "SCI.h"

char string[20];
unsigned short val;  
unsigned int on=0;
unsigned int thold;
unsigned int abvThold;
unsigned int bpm;
unsigned int time;
int lowdig;   //least sig dig
int middig;   //intermediate sig dig
int highdig;  //most sig fig

void msDelay(unsigned int);
interrupt VectorNumber_Vtimch0 void ISR_Vtimch0(void);

void Out(void){
 PTJ ^=0x01; //Toggle PJ0
}

void OutCRLF(void){
  SCI_OutChar(CR);
  SCI_OutChar(LF);
 // PTJ ^= 0x00;          // toggle LED D2
}
 void SetClk16(void){
  
  CPMUREFDIV = 0x00;        //Using ref clock 1mhz, not oscill 
  CPMUSYNR = 0x0F;          //Set SYNDIV >> 15+1 dec>> 32=2*1mhz*16=32mhz
  CPMUPOSTDIV = 0x00;       //no post div
  CPMUCLKS = 0x80;          //Select PLL clock 32mhz/2=16mhz
  CPMUOSC = 0x00;           //turn off oscill frequency
  
  while(!(CPMUFLG & 0x08));   //wait for LOCK Flag    
  
  }
  
void Display_BPM(int bp){
  int tempvar=0;
  
  
  lowdig=bp%10;   //bpm/10 remainder
  tempvar= bp/10;     //b/10 to temp
  middig=tempvar%10;  //tempvar/10 remainder
  highdig=tempvar/10; //tempvar/10 to most sig fig
  
  PT1AD= lowdig &0x04;         //AD0-3
  PTP=  middig  &0x04;              //P0-3
  PTM= highdig   &0x01;              //M0
}
  
void main(void) {
SetClk16();
 //******Set Ports******//
  DDRJ = 0x01;      //onboard LED
  ATDDIEN =0x0F;
  DDR1AD=0x0F;      //AD
  DDRM=0x01;        //M
  DDRP=0x0F;        //PP
  
 //DRJ |=0x01;
  

 //******* TIME CAPTURE******//                                               
          
  TSCR1 = 0x90;    //Timer System Control Register 1
                    // TSCR1[7] = TEN:  Timer Enable (0-disable, 1-enable)
                    // TSCR1[6] = TSWAI:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[5] = TSFRZ:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[4] = TFFCA:  Timer Fast Flag Clear All (0-normal 1-read/write clears interrupt flags)
                    // TSCR1[3] = PRT:  Precision Timer (0-legacy, 1-precision)
                    // TSCR1[2:0] not used

  TSCR2 = 0x00;    //Timer System Control Register 2
                    // TSCR2[7] = TOI: Timer Overflow Interrupt Enable (0-inhibited, 1-hardware irq when TOF=1)
                    // TSCR2[6:3] not used
                    // TSCR2[2:0] = Timer Prescaler Select: See Table22-12 of MC9S12G Family Reference Manual r1.25 (set for bus/1)                  
  TIOS = 0xFE;     //Timer Input Capture or Output capture
                    //set TIC[0] and input (similar to DDR)
  PERT = 0x01;     //Enable Pull-Up resistor on TIC[0]

  TCTL3 = 0x00;    //TCTL3 & TCTL4 configure which edge(s) to capture
  TCTL4 = 0x02;    //Configured for falling edge on TIC[0]

 // The next one assignment statement configures the Timer Interrupt Enable                                                   
          
   
  TIE = 0x01;      //Timer Interrupt Enable

/*
 * The next one assignment statement configures the ESDX to catch Interrupt Requests                                                   
 */           
  
	EnableInterrupts; //CodeWarrior's method of enabling interrupts


//************ATD Channel**************//
	ATDCTL1 = 0x4F;		// set for 12-bit resolution
	ATDCTL3 = 0x88;		// right justified, one sample per sequence  ???
	//assuming atd clock =1mhz
	ATDCTL4 = 0x07;		// prescaler = 7 ATD clock = 16mhz /(2 * (7 + 1)) == 1.04MHz
	ATDCTL5 = 0x26;		// continuous conversion on channel 6
	                    
// Setup LED and SCI
 //DRJ |= 0x01;     // PortJ bit 0 is output to LED D2 on DIG13
  SCI_Init(9600);
  
  //SCI_OutString("Technological Arts - EsduinoXtreme ADC demo"); OutCRLF();
 // for(;;) {
  // PTJ ^= 0x01;          // toggle LED
  /* val=ATDDR0;
   if(on==1){
    
  // SCI_OutString("Analog0 reading in decimal = ");
   SCI_OutUDec(val);
   // SCI_OutString("    Analog0 reading in hex = ");
   // SCI_OutUHex(val);
  // OutCRLF();     
  SCI_OutChar(CR);
   }
    msDelay(33);*/    // 1 sec delay, you can validate using method outlined in Lecture W8-2_W8-3
     //val=ATDDR0;
    //if(on==0){
    
    //SCI_OutUDec(val);
    //OutCRLF();       */
    thold=190;     //200
    abvThold=0;
      for (;;){
      if (on==0){
      val=ATDDR0;   
      SCI_OutUDec(val);
      OutCRLF();
      
      if (val>=thold&&abvThold==0){
      abvThold=1;
      bpm=60000/time;
      //bpm=120;;
      OutCRLF();
      time=0;
      
      lowdig=bpm%10;
      bpm/=10;
      middig=bpm%10;
      bpm/=10;
      highdig=bpm%10;
  
      PT1AD=(middig & 0x07)*16; //Port AD from a0 to a3 for the ones
      PT1AD+=lowdig;
      PTM=highdig & 0x01;     //PTM0 is for the hundred bit
      PTP=(middig & 0x08)*16;      //Port P from p0 to p3 for the tens
      PTP+=middig;
  
     
      
    }
    
    if (abvThold==1&&val<thold){
      abvThold=0;
    }
    
    time+=40;    //finds time between each pulse
    if (time>5000){
      time=0;
      OutCRLF();
      PT1AD=0x00;
      }
     
     
     msDelay(40);

  }
}
} 

//***************Interrupt***************//
interrupt VectorNumber_Vtimch0 void ISR_Vtimch0(void){
 unsigned int temp; //DON'T EDIT THIS
// PTJ ^= 0x00; //off state
 if(on==0){   
   on=1;
   Out(); //toggle
   SCI_OutChar(CR);

 } else{
   on=0;   
   Out();  //toggle
 }     
 msDelay(500);
 temp=TC0;
 
}

void msDelay(unsigned int k)
{
 /*nt i;
  
  TSCR1 = 0x90;
  TSCR2 = 0x00;
  TIOS |= 0x01;
  TC0 = TCNT + 1600;
  for(i = 0; i < k; i++){
    while(!(TFLG1_C0F));
    TC0 += 1600;
  }                  
  TIOS &= ~0x01;
}  */                      //x/68=eclock/6.25
  unsigned int i;
  unsigned int j;
  
  for(j=0; j<k;j++){
    for(i=0; i<176;i++){
      //Delay
      PTJ=PTJ;
      PTJ=PTJ;
      PTJ=PTJ;
      PTJ=PTJ;
      PTJ=PTJ;
    }
  } 
}