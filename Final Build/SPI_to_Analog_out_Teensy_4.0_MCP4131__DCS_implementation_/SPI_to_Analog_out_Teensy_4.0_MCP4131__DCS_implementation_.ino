#include <SPI.h>
int16_t decode_MAIN(uint32_t data); //prototype func so they can be at end of file
void send_analog_out(int16_t value);
int16_t decode_DIAG(uint32_t data);
uint16_t sign_extend(uint16_t num);



//___________________________________________________________________________________________________
//|                                                                                                                                                                                                 
//|                              INTRO                                                                                       
//|________________________________________________________________________________________________   
    //this code implemnts a spi sniffer to convert Spi measurments from out current sensor (DS_ACS37820)
    //to an analog output. The sniffer uses Teensy 4.0 ARM M-7 processor's Low power spi module (LSPI)
    //the analog out is from a MCP4131 Digital to analog converter (DAC). The DAC is an external device 
    //teensy controls via spi. Refer to documentation in folder for datasheets and more information. 




void setup() {
//___________________________________________________________________________________________________
//|                                                                                              
//|                                                                                                  
//|                                                                                                    
//|                              DAC SPI setup  (DAC SPI)                                                                                       
//|                                                                                                  
//|                                                                                                  
//|__________________________________________________________________________________________________                                                                                                  

        //this section covers the configuration of the LSPI module.
        //use send_analog_out function to send analog data out. Refer to end of document
        Serial.begin(115200);
        
        pinMode(36, OUTPUT); // set the SS pin as an output of DAC
        SPI2.begin();

        

  
  




//___________________________________________________________________________________________________
//|                                                                                              
//|                                                                                                  
//|                                                                                                    
//|                              LSPI module setup  (current sensor)                                                                                       
//|                                                                                                  
//|                                                                                                  
//|__________________________________________________________________________________________________                                                                                                  


        //this section covers the configuration of the LSPI module.
        
        
        pinMode (10, INPUT); //CS for Current sensor spi
        SPI.begin();   //this sets up the clock and all that good jazz
         
        LPSPI4_CR = 0; //disable LSPI module
        
        IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00 = 0x3; // set CS LPSPI4 module to use PCS0 "which CS pin do I use?"
      
        LPSPI4_CFGR1 = 0;                   //turning module to slave mode, and turn off internal CS           set 0x3000000 to swap pins
        LPSPI4_FCR   = 524,296;             //setting RXWATER to trigger flag when 8 words are in FIFO, TXWATER when there are under 8 words in the FIFO (may just poll the word size tho...)
        LPSPI4_TCR   = 0xC004001F;          //setting CPOL =1, CPHA = 1, Frame size = 31                          C004001F to tristate the MOSI pin.    
                                            //for master mode CPHA=0 = 0x8004001F CPHA = 1 =  0xC004001F
       
        LPSPI4_CR = 1; //enable LSPI module

  

}
 




      //variables fo r
      int a = 0;            //value for spi address
      //int16_t data = 0;     //value for main path data
      int16_t Main_A = 0;   //value for Main_A
      int16_t Main_B = 0;   //value for Main_B
      int16_t DIAG_A = 0;   //value for Main_A
      int16_t DIAG_B = 0;   //value for Main_B
      uint32_t reg_read = 0; 
      
      
      //0x09 MAIN_A
      //0x0A MAIN_B
      //0x0E DIAG_A
      //0x0F DIAG_B






//___________________________________________________________________________________________________
//|                                                                                              
//|                                                                                                  
//|                                                                                                    
//|                              Main Loop                                                                                
//|                                                                                                  
//|                                                                                                  
//|__________________________________________________________________________________________________                                                                                                  
    //here is where the magic happens. Spi module fills up FIFO buffers, and we read them. 
//for debugging
int start_time = 0;
int stop_time = 0;


  void loop() {

           
           //if start misaligned
           if ((not((micros() - start_time)>20))&(LPSPI4_FSR>>16 > 0)){ //if FIFO is filled to >0 during time in between messages
            
            while(LPSPI4_FSR>>16 > 0){ //empty fio
              reg_read = LPSPI4_RDR; 
            }
            }
           
  
           if(LPSPI4_FSR>>16 > 7){   //if more than 8 in RXFIFO that is the size of the frame. 
                  
                  start_time = micros();
                  
                  
                  while(LPSPI4_FSR>>16 > 0){ //empty the fifo
                    
                            reg_read = LPSPI4_RDR; //read register in fifo
                            
                            a = (reg_read&0x7C000000)>>26;    //parse address
                              
                            //data = (reg_read&0x3FFFC0)>>6;    //data 
                            //Serial.print(reg_read, HEX);
                            //Serial.print("  ");
                            //to see addresses in frame 
                            //Serial.print(a, HEX); 
                            //Serial.print("   ");

                            
                    
                            
                            if (a == 0x9){                       //look for Main A
                                      Main_A = decode_SPI_MESSAGE(reg_read);             //store Main A data
                                      //Serial.println(Main_A);
                                      //Serial.print("MAIN_A: ");
                                      //Serial.print(Main_A);
//                                      Serial.print("  ");
                              }
                    
                            else if (a == 0xA){              //store Main B data
                                     Main_B = decode_SPI_MESSAGE(reg_read);
//                                     Serial.print("MAIN_B: ");
//                                     Serial.print(Main_B);   
                              }


                           else if (a == 0x0E){              //store DIAG A data
                                     
                                     DIAG_A = decode_SPI_MESSAGE(reg_read);
                                     DIAG_A = DIAG_A&0xFFF;         //clear any header bits
                                     DIAG_A = sign_extend(DIAG_A); //sign extend to 16 bit value (when signed value, shifting most signifigant sign bit
                                    
                                     
                                    
                              }

                           else if (a == 0x0F){              //store DIAG B data
                                     DIAG_B = decode_SPI_MESSAGE(reg_read);
                                     DIAG_B = DIAG_B&0xFFF;         //clear any header bits
                                     DIAG_B = sign_extend(DIAG_B); //sign extend to 16 bit value (when signed value, shifting most signifigant sign bit
                                     //Serial.println(reg_read, HEX);   
                              }
                              
                  }
                  
                  //Serial.println("");        
                  send_analog_out(DIAG_A<<8);     //send Main_A out to analog_out
                  //Serial.println(DIAG_A);
                  //Serial.println(Main_B);
            }
  }





//___________________________________________________________________________________________________
//|                                                                                              
//|                                                                                                  
//|                                                                                                    
//|                              DAC analog out function                                                                                  
//|                                                                                                  
//|                                                                                                  
//|__________________________________________________________________________________________________                                                                                                  
        void send_analog_out(int16_t value){
          
                //this function takes in signed 16 bit value. 
                //it will map   (-32768 to 32767) ->  (0 to 3.3V) 
                
                //3.3V   corresponds to 32767  the top of the DAC output
                //1.65V  corresponds to   0  the middle of the DAC output
                // 0V    corresponds to -32768  the bottom of the DAC output
                
                
                 //DAC Spi comms
                 SPI2.beginTransaction(SPISettings(50000000, MSBFIRST, SPI_MODE2));         //init spi write
                 digitalWriteFast(36, LOW);                                                 // set the CS pin to LOW
                 
                 SPI2.transfer16(32767 + value);                                            //send a write command to the MCP4131 to write at registry address 0x00 a 16bit value
                 digitalWriteFast(36, HIGH);                                                //set the CS pin HIGH
                 SPI2.endTransaction();                                                     //ends spi write
          }



//___________________________________________________________________________________________________
//|                                                                                              
//|                                                                                                  
//|                                                                                                    
//|                              frame decode functions                                                                                  
//|                                                                                                  
//|                                                                                                  
//|__________________________________________________________________________________________________ 
 //this section has functions that decode the different message formats
 //returning a signed int value for input of the DAC 

int16_t decode_SPI_MESSAGE(uint32_t data){
  //this function gets the data from the MAIN message  
  // a 16 bit signed value input 
  // output is 16 bit signed value for DAC output code
   data = (data&0x3FFFC0)>>6;    //data 
   
  
  return data;
  }






 
 









//___________________________________________________________________________________________________
//|                                                                                              
//|                                                                                                  
//|                                                                                                    
//|                              Tools                                                                                
//|                                                                                                  
//|                                                                                                  
//|__________________________________________________________________________________________________    

uint16_t sign_extend(uint16_t num){
  //this function sign extends a 12bit value to 16 bit (for diag)
  int bit_num = 11; 
  int MSB = 0;
  MSB = bitRead(num,bit_num);

  //extend the bits
  bitWrite(num, 12, MSB);
  bitWrite(num, 13, MSB);
  bitWrite(num, 14, MSB);
  bitWrite(num, 15, MSB);
 
  return num; 
  }



//FOR FINDING STATE OF THE SYSTEM
//  Serial.print("LPSPI4_VERID:  "); Serial.println(LPSPI4_VERID);
//  Serial.print("LPSPI4_PARAM:  "); Serial.println( LPSPI4_PARAM);
//  Serial.print("LPSPI4_CR:     "); Serial.println( LPSPI4_CR);
//  Serial.print("LPSPI4_SR:     "); Serial.println(LPSPI4_SR);
//  Serial.print("LPSPI4_IER:    "); Serial.println(LPSPI4_IER);
//  Serial.print("LPSPI4_DER:    "); Serial.println(LPSPI4_DER);
//  Serial.print("LPSPI4_CFGR0:  "); Serial.println( LPSPI4_CFGR0);
//  Serial.print("LPSPI4_CFGR1:  "); Serial.println(LPSPI4_CFGR1);
//  Serial.print("LPSPI4_DMR0:   "); Serial.println(LPSPI4_DMR0);
//  Serial.print("LPSPI4_DMR1:   "); Serial.println(LPSPI4_DMR1);
//  Serial.print("LPSPI4_CCR:    "); Serial.println(LPSPI4_CCR);
//  Serial.print("LPSPI4_FCR:    "); Serial.println(LPSPI4_FCR);
//  Serial.print("LPSPI4_FSR:    "); Serial.println(LPSPI4_FSR);
//  Serial.print("LPSPI4_TCR:    "); Serial.println(LPSPI4_TCR);
//  //Serial.print("LPSPI4_TDR:    "); Serial.println(LPSPI4_TDR); CANNOT READ
//  Serial.print("LPSPI4_RSR:    "); Serial.println(LPSPI4_RSR);
//  Serial.print("LPSPI4_RDR:    "); Serial.println(LPSPI4_RDR);


//for debugging
//int start_time = 0;
//int stop_time = 0;
