#include <Arduino.h>
#include <Wire.h> //Utilisee pour le bus I2C

//Pins des capteurs ultrasons
const int trigPin1 = 5;
const int echoPin1 = 18;
const int trigPin2 = 4;
const int echoPin2 = 0;

const int channel = 0; // Canal PWM à utiliser
const int frequency = 50;// Fréquence PWM en Hz const
int resolution = 8; // Résolution PWM en bits (8 bits pour une sortie allant de 0 à 255)

const int pwmPin = 13; // Broche de sortie PWM

#define SOUND_SPEED 0.034 //Vitesse du son pour avoir la distance en cm

long duration1; // Durée de reception du signal ultrason du capteur 1 depuis qu'il est envoyé
float distanceCm1;// Distance de l'objet du capteur 1 en cm

long duration2;
float distanceCm2;

int result; // Variable renvoyee a la raspberri pour donner un resultat


bool vehiculeEntrant = false; // bool indiquant si un vehicule entre
bool barriereOccupee = false; // bool indiquant si la barriere est obstruee

static int angle = 0; // angle de la barriere desire

void fermerBarriere(void)
{
  while (angle != 13)
  {
    if (angle<13)
    {
      angle++;
      ledcWrite(channel,angle);
    }
    else if (angle>13)
    {
      angle--;
      ledcWrite(channel,angle);
    }
    delay(500); // Ralentie la fermeture
  } // Regle la barriere a un angle de 13
}

void ouvrirBarriere(void)
{
  while (angle != 20)
  {
    if (angle<20)
    {
      angle++;
      ledcWrite(channel,angle);
    }
    else if (angle>20)
    {
      angle--;
      ledcWrite(channel,angle);
    }
    delay(500); // Ralentie l'ouverture
  } // Regle la barriere a un angle de 20
}



int lireDistance(void) // lit la distance du capteur 1
{
  // mise a 0 du trigger
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  // etat haut du trigger pour 10ms
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);

  // lis l'echo et donne le temps de parcours de l'onde sonore en ms
  duration1 = pulseIn(echoPin1, HIGH);
  // calcule la distance
  distanceCm1 = duration1 * SOUND_SPEED/2;
  return distanceCm1; //retourne la distance
}

void receiveEvent(int howMany){
  byte myByte = Wire.read(); // lis le byte arrivant
  Serial.print("receivedByte: ");
  Serial.println(myByte); // affiche le byte

  
  if (myByte == 10) // si le byte recu est un ordre
  {
    Serial.print("if 10 ");
    distanceCm1 = lireDistance(); //lire la distance du capteur 1
    if(distanceCm1>20) result = 10; // renvoyer 10 s'il n'y a pas d'obstacle
    else result = 5; // renvoyer 5 s'il y an a un
  }

}

void requestEvent(){
  Wire.write(result); // renvoie si le capteur 1 a detecte un obstacle
}

void setup() {
  Wire.begin(0x54); // Rejoindre le bus à l'adresse #13
	Wire.onReceive(receiveEvent); // Preparer une fonction spécifique a la reception de donnee
  Wire.onRequest(requestEvent);
  Serial.begin(115200); // Commencer une communication serie
  pinMode(trigPin1, OUTPUT); // regler trigPin 1 comme Output
  pinMode(echoPin1, INPUT); // regler echoPin 1 comme Input
  pinMode(trigPin2, OUTPUT); // regler trigPin 2 comme Output
  pinMode(echoPin2, INPUT); // regler echoPin 2 comme Input

  // Configuration de la bibliothèque ledc
  ledcSetup(channel, frequency, resolution);
  ledcAttachPin(pwmPin, channel);

  fermerBarriere(); // On ouvre la barrière pour commencer
}

void loop() {

  // Detection du capteur 1 (commente plus haut)
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);

  duration1 = pulseIn(echoPin1, HIGH);
  distanceCm1 = duration1 * SOUND_SPEED/2;

  // Detection du capteur 2 (commente plus haut)
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);

  duration2 = pulseIn(echoPin2, HIGH);
  distanceCm2 = duration2 * SOUND_SPEED/2;

  if(distanceCm1<20) { // Si un vehicule arrive devant la barriere
    ouvrirBarriere();
    vehiculeEntrant = true;
  }
  else if(vehiculeEntrant) { 
    if(distanceCm2<20) { // Si le véhicule entrant est sous la barriere ouverte
      ouvrirBarriere();
      barriereOccupee = true;
    }
    else if((barriereOccupee) and (vehiculeEntrant)) { // Si le véhicule entrant n'est plus devant ou n'est plus sous la barriere
      fermerBarriere();
      vehiculeEntrant = false;
      barriereOccupee = false;
    }
    else { // Si le véhicule entrant n'est plus detecte devant la barriere et s'apprete a aller sous la barriere 
      ouvrirBarriere();
    }
  }
  else { // Si rien n'est detecte devant la barriere
    fermerBarriere();
  }
  
  // Afficher les informations sur le moniteur serie
  Serial.print("Distance1 (cm): ");
  Serial.println(distanceCm1);
  Serial.print("Distance2 (cm): ");
  Serial.println(distanceCm2);

  Serial.print("vehiculeEntrant: ");
  Serial.println(vehiculeEntrant);
  Serial.print("barriereOccupee: ");
  Serial.println(barriereOccupee);

  Serial.print("position servo: ");
  Serial.println(ledcRead(channel));
  Serial.print("angle desire: ");
  Serial.println(ledcRead(angle));

  delay(1000);
}