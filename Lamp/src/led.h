#define PWM_R LPC_TMR32B0->MR3
#define PWM_G LPC_TMR32B1->MR0
#define PWM_B LPC_TMR32B1->MR1
#define PWM_W LPC_TMR32B1->MR2

void setRed(uint8_t setValue);
void setGreen(uint8_t setValue);
void setBlue(uint8_t setValue);
void setWhite(uint8_t setValue);
void testRed();
void testGreen();
void testBlue();
void testWhite();
void setColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white);
void blinkRed();
void blinkBlue();
void blinkWhite();
void blinkGreen();
void colorWheel(uint32_t delay);

