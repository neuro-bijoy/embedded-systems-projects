const int POS = 8;
const int NEG = 9;

const int DEAD_TIME = 100;   // 100 us

void setup()
{
    pinMode(POS, OUTPUT);
    pinMode(NEG, OUTPUT);

    // Both outputs OFF initially
    digitalWrite(POS, LOW);
    digitalWrite(NEG, LOW);
}

void pulse(int pin, int onTime)
{
    digitalWrite(pin, HIGH);
    delayMicroseconds(onTime);

    digitalWrite(pin, LOW);
    delayMicroseconds(500);
}

void loop()
{
    // Make sure negative side is OFF
    digitalWrite(NEG, LOW);
    delayMicroseconds(DEAD_TIME);

    // -------- Positive half cycle --------
    pulse(POS, 500);
    pulse(POS, 750);
    pulse(POS, 1250);
    pulse(POS, 2000);
    pulse(POS, 1250);
    pulse(POS, 750);
    pulse(POS, 500);

    // Turn positive side OFF
    digitalWrite(POS, LOW);

    // Dead time
    delayMicroseconds(DEAD_TIME);

    // -------- Negative half cycle --------
    pulse(NEG, 500);
    pulse(NEG, 750);
    pulse(NEG, 1250);
    pulse(NEG, 2000);
    pulse(NEG, 1250);
    pulse(NEG, 750);
    pulse(NEG, 500);

    // Turn negative side OFF
    digitalWrite(NEG, LOW);

    // Dead time before next positive half
    delayMicroseconds(DEAD_TIME);
}
