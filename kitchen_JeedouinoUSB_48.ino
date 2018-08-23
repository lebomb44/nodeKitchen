////////
//
// Sketch Arduino pour le Plugin JEEDOUINO v097+ de JEEDOM
// Connection via USB avec le Démon Python 
//
// Généré le 2018-08-15 20:50:33.// Pour l'équipement Kitchen (EqID : 48 ).// Modèle de carte : anano.
////////
#define DEBUGtoSERIAL 0	// 0, ou 1 pour debug dans la console serie
#define UseWatchdog 0
#define UseDHT 1
#define UseDS18x20 1
#define UseTeleInfo 0
#define UseLCD16x2 0	// 0 = None(Aucun) / 1 = LCD Standard 6 pins / 2 = LCD via I2C
#define UseHCSR04 0
#define UsePwm_input 0 // Code obsolete (sera supprimé) - Entrée Numérique Variable (0-255 sur 10s) en PULL-UP
#define UseBMP180 0		// pour BMP085/180 Barometric Pressure & Temp Sensor
#define UseServo 0
#define UseWS2811 0	// Pour gerer les led stips a base de WS2811/2 avec l'excellente lib Adafruit_NeoPixel

// Vous permet d'inclure du sketch perso - voir Doc / FAQ.
// Il faut activer l'option dans la configuration du plugin.
// Puis choisir le nombre de variables utilisateur sous l'onglet Pins/GPIO de votre équipement.
#define UserSketch 0
// Tags pour rechercher l'emplacement pour votre code :
//UserVars
//UserSetup
//UserLoop

// CONFIGURATION VARIABLES
byte IP_JEEDOM[] = { 192, 168, 10, 4 };
#define NB_DIGITALPIN 14
#define NB_ANALOGPIN 6
#define NB_TOTALPIN ( NB_DIGITALPIN  + NB_ANALOGPIN)

// Etat des pins de l'arduino ( Mode )
char Status_pins[NB_TOTALPIN];
byte pin_id;
byte echo_pin;

String eqLogic = "";
String inString = "";
String Message = "";
byte BootMode;

// Pour la detection des changements sur pins en entree
byte PinValue;
byte OLDPinValue[NB_TOTALPIN ];
unsigned long AnalogPinValue;
unsigned long OLDAnalogPinValue[NB_TOTALPIN ];
unsigned long CounterPinValue[NB_TOTALPIN ];
unsigned long PinNextSend[NB_TOTALPIN ];
byte swtch[NB_TOTALPIN];
// pour envoi ver jeedom
String jeedom = "\0";
// reception commande
char c[250];
char buf[1];
int buf_etat=0;
byte n=0;
byte RepByJeedom=0;
// Temporisation sorties
unsigned long TempoPinHIGH[NB_TOTALPIN ]; // pour tempo pins sorties HIGH
unsigned long TempoPinLOW[NB_TOTALPIN ]; // pour tempo pins sorties LOW
unsigned long pinTempo=0;
unsigned long NextRefresh=0;
unsigned long ProbeNextSend=millis();
unsigned long timeout = 0;



// SETUP

void setup()
{
	jeedom.reserve(256);
	Message.reserve(16);
	inString.reserve(4);
	Serial.begin(115200); // Init du Port serie/USB
	Serial.setTimeout(5); // Timeout 5ms
	if (EEPROM.read(13) != 'J') Init_EEPROM();
	Load_EEPROM(1);


}

// LOOP

void loop()
{
	// On ecoute l'usb
	if (Serial.available()>0)
	{
		// on regarde si on recois des donnees
		n=0;
		timeout = millis()+1000;
		while (millis()<timeout)
		{
			buf_etat = Serial.readBytes(buf,1);
			if (buf_etat)
			{
				c[n] = buf[0];
				if (c[n]=='\r') c[n]='\n';
				if (c[n]=='\n') break;
				n++;
			}
			else break;
		}

		if (n && c[n]=='\n')
		{
			n--;
			// on les traites
			if (c[0]=='C' && c[n]=='C')		 // Configuration de l'etat des pins
			{
				// NB_TOTALPIN = NB_DIGITALPIN  + NB_ANALOGPIN

				if (n==(NB_TOTALPIN+1))						// Petite securite
				{
					for (int i = 0; i < NB_TOTALPIN; i++)
					{
						EEPROM.update(30+i, c[i+1]);			 // Sauvegarde mode des pins
					}
					Load_EEPROM(0);								// On met en place
					Serial.println(F("COK"));							// On reponds a JEEDOM
					ProbeNextSend=millis()+60000; 			// Décalage pour laisser le temps aux differents parametrages d'arriver de Jeedom
				}
			}
			else if (c[0]=='P' && c[n]=='G')	// On repond au ping
			{
				if (n==3)					// Petite securite
				{
					Serial.println(F("PINGOK"));								// On reponds a JEEDOM
					ProbeNextSend=millis()+10000; 			// Décalage pour laisser le temps aux differents parametrages d'arriver de Jeedom
				}
			}
			else if (c[0]=='E' && c[n]=='Q')	 // Recuperation de l' eqLogic de Jeedom concernant cet arduino
			{
				eqLogic = "";
				EEPROM.update(15, n);						// Sauvegarde de la longueur du eqLogic
				for (int i = 1; i < n; i++)
				{
					EEPROM.update(15+i, c[i]-'0');			 // Sauvegarde de l' eqLogic
					eqLogic += (char)c[i];
				}
				Serial.println(F("EOK"));							// On reponds a JEEDOM
				ProbeNextSend=millis()+60000; // Décalage pour laisser le temps aux differents parametrages d'arriver de Jeedom
			}
			else if (c[0]=='I' && c[n]=='P')	 // Recuperation de l' IP de Jeedom ( I192.168.000.044P )
			{
				if (n<17)					// Petite securite
				{
					int ip=0;
					inString="";
					for (int i = 1; i <= n; i++)	 //jusqu'a n car il faut un caractere non digit pour finir
					{
						if (isDigit(c[i]))
						{
							inString += (char)c[i];
						}
						else
						{
							IP_JEEDOM[ip]=inString.toInt();
							inString="";
							ip++;
						}
					}
					EEPROM.update(26, IP_JEEDOM[0]);					// Sauvegarde de l' IP
					EEPROM.update(27, IP_JEEDOM[1]);
					EEPROM.update(28, IP_JEEDOM[2]);
					EEPROM.update(29, IP_JEEDOM[3]);
					Serial.println(F("IPOK"));								// On reponds a JEEDOM
					ProbeNextSend=millis()+60000; // Décalage pour laisser le temps aux differents parametrages d'arriver de Jeedom
				}
			}
			else if (c[0]=='S' && c[n]=='S')	 // Modifie la valeur d'une pin sortie
			{
				for (int i = 1; i < n; i++)
				{
					if (isDigit(c[i])) c[i]=c[i]-'0';
				}

				pin_id=10*int(c[1])+int(c[2]);					// recuperation du numero de la pin

				Set_OutputPin(pin_id);
				Serial.println(F("SOK"));								// On reponds a JEEDOM
				ProbeNextSend=millis()+10000; // Décalage pour laisser le temps au differents parametrages d'arriver de Jeedom
			}
			else if ((c[0]=='S' || c[0]=='R') && c[n]=='C')       	// Reçoie la valeur SAUVEE d'une pin compteur (suite reboot)
			{                                       										// ou RESET suite sauvegarde equipement.
				if (n>3)					// Petite securite
				{

					for (int i = 1; i < n; i++)
					{
						if (isDigit(c[i])) c[i]=c[i]-'0';
					}

					if (c[0]=='R') CounterPinValue[pin_id]=0; // On reset la valeur si demandé.

					pin_id=10*int(c[1])+int(c[2]);					// récupération du numéro de la pin
					int multiple=1;
					for (int i = n-1; i >= 3; i--)					// récupération de la valeur
					{
						CounterPinValue[pin_id] += int(c[i])*multiple;
						multiple *= 10;
					}
					PinNextSend[pin_id]=millis()+2000;
					NextRefresh=millis()+60000;
					ProbeNextSend=millis()+10000; // Décalage pour laisser le temps au differents parametrages d'arriver de Jeedom
					Serial.println(F("SCOK"));							 // On reponds a JEEDOM
				}
			}
			else if (c[0]=='S' && c[n]=='F')	 // Modifie la valeur de toutes les pins sortie (suite reboot )
			{
				// NB_TOTALPIN = NB_DIGITALPIN  + NB_ANALOGPIN
				if (n==(NB_TOTALPIN+1))					// Petite securite
				{
					for (int i = 2; i < NB_TOTALPIN; i++)
					{
						switch (Status_pins[i])
						{
							case 'o':		//	output
							case 's':		//  switch
							case 'l':		 //	low_relais
							case 'h':		//	high_relais
							case 'u':		//	output_pulse
							case 'v':		//	low_pulse
							case 'w':		 //	high_pulse
								if (c[i+1]=='0')
								{
									PinWriteLOW(i);
								}
								else if (c[i+1]=='1')
								{
									PinWriteHIGH(i);
								}
								break;
						}
					}
					RepByJeedom=0; // Demande repondue, pas la peine de redemander a la fin de loop()
					Serial.println(F("SFOK"));								// On reponds a JEEDOM
					ProbeNextSend=millis()+20000; // Décalage pour laisser le temps au differents parametrages d'arriver de Jeedom
				}
			}
			else if (c[0]=='S' && (c[n]=='L' || c[n]=='H' || c[n]=='A')) // Modifie la valeur de toutes les pins sortie a LOW / HIGH / SWITCH / PULSE
			{
				if (n==2 || n==7)			// Petite securite  : S2L / S2H / S2A / SP00007L /SP00007H
				{
					//jeedom+=F("&REP=SOK");
					for (int i = 1; i < n; i++)
					{
						if (isDigit(c[i])) c[i]=c[i]-'0';
					}
					if (c[1]=='P') pinTempo = 10000*int(c[2])+1000*int(c[3])+100*int(c[4])+10*int(c[5])+int(c[6]);
					for (int i = 2; i < NB_TOTALPIN; i++)
					{
						TempoPinHIGH[i] = 0;
						TempoPinLOW[i] = 0;
						switch (Status_pins[i])
						{
							case 'o': // output
							case 's': // switch
							case 'l': // low_relais
							case 'h': // high_relais
							case 'u': // output_pulse
							case 'v': // low_pulse
							case 'w': // high_pulse
								if (c[n]=='L')
								{
									if (c[1] == 'P') TempoPinHIGH[i] = pinTempo;
									PinWriteLOW(i);
								}
								else if (c[n] == 'H')
								{
									if (c[1] == 'P') TempoPinLOW[i] = pinTempo;
									PinWriteHIGH(i);
								}
								else
								{
									if (swtch[i]==1) PinWriteLOW(i);
									else PinWriteHIGH(i);
								}
							break;
						}
					}
					Serial.println(F("SOK"));							// On reponds a JEEDOM
					ProbeNextSend=millis()+10000; // Décalage pour laisser le temps au differents parametrages d'arriver de Jeedom
				}
			}
			else if (c[0]=='B' && c[n]=='M')	 // Choix du BootMode
			{
				BootMode=int(c[1]-'0');
				EEPROM.update(14,  BootMode);
				ProbeNextSend=millis()+3000; // Décalage pour laisser le temps aux differents parametrages d'arriver de Jeedom
				Serial.println(F("BMOK"));									// On reponds a JEEDOM
			}
			else if (c[0]=='U' && c[n]=='R')	// User Action
			{
				Serial.println(F("SOK"));				 // On reponds a JEEDOM
				UserAction();
			}
			else
			{
				Serial.println(F("NOK"));										// On reponds a JEEDOM
			}
		}
	}
	// On ecoute les pins en entree
	//jeedom="";
	for (int i = 2; i < NB_TOTALPIN; i++)
	{
		byte BPvalue = 0;
		switch (Status_pins[i])
		{
			case 'i':		// input
			case 'p':		// input_pullup
				PinValue = digitalRead(i);
				if (PinValue!=OLDPinValue[i] && (PinNextSend[i]<millis() || NextRefresh<millis()))
				{
					OLDPinValue[i]=PinValue;
					jeedom += '&';
					jeedom += i;
					jeedom += '=';
					jeedom += PinValue;
					PinNextSend[i]=millis()+1000;		// Delai 1s pour eviter trop d'envois
				}
				break;
			case 'n':		// BP_input_pulldown
				BPvalue = 1;
			case 'q':		// BP_input_pullup
				PinValue = digitalRead(i);
				if (PinValue != OLDPinValue[i])
				{
					PinNextSend[i] = millis() + 50;   // Delai antirebond
					OLDPinValue[i] = PinValue;
					ProbeNextSend = millis() + 5000; // decale la lecture des sondes pour eviter un conflit
				}
				if (PinNextSend[i] < millis() && PinValue != swtch[i])
				{
					if (PinValue == BPvalue) CounterPinValue[i] += 1;
					OLDAnalogPinValue[i] = millis() + 250;   // Delai entre clicks
					swtch[i] = PinValue;
				}
				if (OLDAnalogPinValue[i] < millis() && CounterPinValue[i] != 0)
				{
					if (PinValue == BPvalue) CounterPinValue[i] = 99; // Appui long
					jeedom += '&';
					jeedom += i;
					jeedom += '=';
					jeedom += CounterPinValue[i];
					CounterPinValue[i] = 0;
					OLDAnalogPinValue[i] = millis() + 1000;
				}
				break;
			case 'a':		// analog_input
				AnalogPinValue = analogRead(i);
				if (AnalogPinValue!=OLDAnalogPinValue[i] && (PinNextSend[i]<millis() || NextRefresh<millis()))
				{
					if (abs(AnalogPinValue-OLDAnalogPinValue[i])>20)		// delta correctif pour eviter les changements negligeables
					{
						int j=i;
						if (i<54) j=i+40;			// petit correctif car  dans Jeedom toutes les pins Analog commencent a l'id 54+
						OLDAnalogPinValue[i]=AnalogPinValue;
						jeedom += '&';
						jeedom += j;
						jeedom += '=';
						jeedom += AnalogPinValue;
						PinNextSend[i]=millis()+5000;		// Delai 5s pour eviter trop d'envois
					}
				}
				break;
			case 'c':		// compteur_pullup CounterPinValue
				PinValue = digitalRead(i);
				if (PinValue!=OLDPinValue[i])
				{
					OLDPinValue[i]=PinValue;
					CounterPinValue[i]+=PinValue;
				}
				if (NextRefresh<millis() || PinNextSend[i]<millis())
				{
					jeedom += '&';
					jeedom += i;
					jeedom += '=';
					jeedom += CounterPinValue[i];
					PinNextSend[i]=millis()+10000;		// Delai 10s pour eviter trop d'envois
				}
				break;
		}
	}
	if (NextRefresh<millis())
	{
		NextRefresh=millis()+60000;	// Refresh auto toutes les 60s
		if (RepByJeedom) // sert a verifier que jeedom a bien repondu a la demande dans Load_eeprom
		{
			jeedom += F("&ASK=1"); // Sinon on redemande
		}
	}

	#if (UserSketch == 1)
		UserLoop(); // Appel de votre loop() permanent
		// if (NextRefresh<millis()) UserLoop(); // Appel de votre loop() toutes les 60s
	#endif

	if (jeedom!="") SendToJeedom();
}

void UserLoop()
{
		// Votre loop()
		// pour envoyer une valeur a jeedom, il suffit de remplir la variable jeedom comme cela :
		// jeedom += '&';
		// jeedom += u;	// avec u = numero de la pin "info" dans l'equipement jeedom - info pin number
		// jeedom += '=';
		// jeedom += info; // la valeur a envoyer - info value to send
		//
		// Ex:
		// jeedom += '&';
		// jeedom += 500;	// Etat pin 500
		// jeedom += '=';
		// jeedom += '1'; 	// '0' ou '1'
		//
		// jeedom += '&';
		// jeedom += 504;	// pin 504
		// jeedom += '=';
		// jeedom += millis(); 	// valeur numerique
		//
		// jeedom += '&';
		// jeedom += 506;	// pin 506
		// jeedom += '=';
		// jeedom += "Jeedouino%20speaking%20to%20Jeedom...";   // valeur string

		// /!\ attention de ne pas mettre de code bloquant (avec trop de "delays") - max time 2s
}
void UserAction()
{
		// Ens cas d'une reception d'une commande user action depuis jeedom
		// c[0]='U' & c[n]='R')
		//
		// c[1] = c[1]-'0';	==5 (user pin start at 500)
		// c[2] = c[2]-'0';
		// c[3] = c[3]-'0';
		// pin_id = 100 * int(c[1]) + 10 * int(c[2]) + int(c[3]); 	// pin action number
		//
		// c[4] to c[n-1] 	// pin action value
		//
		// Ex:
		// U5000R -> U 500 0 R = binary 0 pin 500
		// U5001R -> U 500 1 R = binary 1 pin 500
		// U502128R -> U 502 128 R = Slider Value 128 pin 502
		// U507[Jeedom] Message|Ceci est un testR -> U 507 [Jeedom] Message | Ceci est un test R = Message pin 507

		// /!\ attention de ne pas mettre de code bloquant (avec trop de "delays") - max time 2s
}

// FONCTIONS

void SendToJeedom()
{
	Serial.println(jeedom);
	delay(333);
	jeedom="";
}

void Set_OutputPin(int i)
{
	TempoPinHIGH[i]=0;
	TempoPinLOW[i]=0;
	switch (Status_pins[i])
	{
		case 'o':			//  output			// S131S pin 13 set to 1 (ou S130S pin 13 set to 0)
		case 'l':			 //  low_relais	// S13S pin 13 set to 0
		case 'h':			//  high_relais   // S13S pin 13 set to 1
			if (c[3]==0)
			{
				PinWriteLOW(i);
			}
			else
			{
				PinWriteHIGH(i);
			}
			break;

		case 's':   //  switch		// S13 pin 13 set to 1 si 0 sinon set to 0 si 1
				if (swtch[i]==1)
				{
					PinWriteLOW(i);
				}
				else
				{
					PinWriteHIGH(i);
				}
				break;

		//
		// ON VERIFIE SI UNE TEMPORISATION EST DEMANDEE SUR UNE DES SORTIES
		// On essai d'etre sur une precision de 0.1s mais ca peut fluctuer en fonction de la charge cpu
		// Testé seulement sur mega2560
		//
		case 'u':		//	output_pulse	 // Tempo ON : S1309999S : pin 13 set to 0 during 999.9 seconds then set to 1 (S1319999 : set to 1 then to 0)
			pinTempo=10000*int(c[4])+1000*int(c[5])+100*int(c[6])+10*int(c[7])+int(c[8]);
			// pinTempo est donc en dixieme de seconde
			pinTempo = pinTempo*100+millis();   // temps apres lequel la pin doit retourner dans l'autre etat.

			// Peut buguer quand millis() arrive vers 50jours si une tempo est en cours pendant la remise a zero de millis().
			// Risque faible si les tempo sont de l'ordre de la seconde (impulsions sur relais par ex.).
			if (c[3]==0)
			{
				TempoPinHIGH[i]=pinTempo;
				PinWriteLOW(i);
			}
			else if (c[3]==1)
			{
				TempoPinLOW[i]=pinTempo;
				PinWriteHIGH(i);
			}
			break;

		case 'v':		//	low_pulse			// Tempo ON : S139999S : pin 13 set to 0 during 999.9 seconds then set to 1
			if (c[3]==0)
			{
				pinTempo=10000*int(c[4])+1000*int(c[5])+100*int(c[6])+10*int(c[7])+int(c[8]);
				// pinTempo est donc en dixieme de seconde
				pinTempo = pinTempo*100+millis();	 // temps apres lequel la pin doit retourner dans l'autre etat.

				TempoPinHIGH[i]=pinTempo;
				PinWriteLOW(i);
			}
			else
			{
				PinWriteHIGH(i);
			}
			break;

		case 'w':		//	high_pulse		// Tempo ON : S139999S : pin 13 set to 1 during 999.9 seconds then set to 0
			if (c[3]==0)
			{
				PinWriteLOW(i);
			}
			else
			{
				pinTempo=10000*int(c[4])+1000*int(c[5])+100*int(c[6])+10*int(c[7])+int(c[8]);
				// pinTempo est donc en dixieme de seconde
				pinTempo = pinTempo*100+millis();	 // temps apres lequel la pin doit retourner dans l'autre etat.

				TempoPinLOW[i]=pinTempo;
				PinWriteHIGH(i);
			}
			break;

		case 'm':		//	pwm_output
			pinTempo=100*int(c[3])+10*int(c[4])+int(c[5]); 	// the duty cycle: between 0 (always off) and 255 (always on).
			analogWrite(i, pinTempo);
			break;
	}
}

void Load_EEPROM(int k)
{
	// on recupere le BootMode
	BootMode=EEPROM.read(14);
	// Recuperation de l'eqLogic
	eqLogic = "";
	n=EEPROM.read(15);				// Recuperation de la longueur du eqLogic
	if (n>0)		// bug probable si eqLogic_id<10 dans jeedom
	{
		for (int i = 1; i < n; i++)
		{
			eqLogic += EEPROM.read(15+i);
		}
	}

	// Recuperation de l'IP
	IP_JEEDOM[0]=EEPROM.read(26);
	IP_JEEDOM[1]=EEPROM.read(27);
	IP_JEEDOM[2]=EEPROM.read(28);
	IP_JEEDOM[3]=EEPROM.read(29);

	// on met en place le mode des pins
	jeedom="";
	byte y=1;
	for (int i = 2; i < NB_TOTALPIN; i++)
	{
		Status_pins[i] = EEPROM.read(30+i); // Etats des pins

		// INITIALISATION DES TABLEAUX DE TEMPO SORTIES
		TempoPinHIGH[i] = 0;
		TempoPinLOW[i] = 0;
		//
		switch (Status_pins[i])
		{
			case 'i':		// input
			case 'a':		// analog_input
			case 'n':		// BP_input_pulldown
				pinMode(i, INPUT);
				break;
			case 't':		// trigger pin
				pinMode(i, OUTPUT);
				digitalWrite(i, LOW);
				break;
			case 'z':		// echo pin
				pinMode(i, INPUT);
				break;
			case 'p':		// input_pullup
			case 'g':     // pwm_input
			case 'q':		// BP_input_pullup
				pinMode(i, INPUT_PULLUP);   // pour eviter les parasites en lecture, mais inverse l'etat de l'entree : HIGH = input open, LOW = input closed
				// Arduino Doc : An internal 20K-ohm resistor is pulled to 5V.
				swtch[i]=0; 	// init pour pwm_input
				OLDPinValue[i]=1;
				PinNextSend[i]=millis();
				break;
			case 'c':		// compteur_pullup
				pinMode(i, INPUT_PULLUP);   // pour eviter les parasites en lecture, mais inverse l'etat de l'entree : HIGH = input open, LOW = input closed
				// Arduino Doc : An internal 20K-ohm resistor is pulled to 5V.
				if (k)
				{
					jeedom += F("&CPT_");  // On demande à Jeedom de renvoyer la dernière valeur connue pour la pin i
					jeedom += i;
					jeedom += '=';
					jeedom += i;
				}
				break;

			case 'o':		//	output
			case 's':		//	switch
			case 'l':		//	low_relais
			case 'h':		//	high_relais
			case 'u':		//	output_pulse
			case 'v':		//	low_pulse
			case 'w':		//	high_pulse
				pinMode(i, OUTPUT);
				// restauration de l'etat des pins DIGITAL OUT au demarrage
				 if (k)
				 {
					switch (BootMode)
					{
						case 0:
							// On laisse tel quel
							break;
						case 1:
							PinWriteLOW(i);
							break;
						case 2:
							PinWriteHIGH(i);
							break;
						case 3:
							PinWriteHIGH(i);
							// On demade a Jeedom d'envoyer la valeur des pins
							if (y)
							{
								jeedom += F("&ASK=1");
								y=0;
								RepByJeedom=1; // sert a verifier que jeedom a bien repondu a la demande
							}
							break;
						case 4:
							if (EEPROM.read(110+i) == 0) PinWriteLOW(i);
							else PinWriteHIGH(i);
							break;
						case 5:
							PinWriteLOW(i);
							// On demade a Jeedom d'envoyer la valeur des pins
							if (y)
							{
								jeedom += F("&ASK=1");
								y=0;
								RepByJeedom=1; // sert a verifier que jeedom a bien repondu a la demande
							}
							break;
					}
				}
				// fin restauration

				break;

			case 'm':		//	pwm_output
				pinMode(i, OUTPUT);
				break;
		}
	}
  if (jeedom!="") SendToJeedom();
}

void PinWriteHIGH(long p)
{
	digitalWrite(p, HIGH);
	swtch[p]=1;
	jeedom += '&';
	jeedom += p;
	jeedom += F("=1");
	// Si bootmode=4 sauvegarde de l'etat de la pin (en sortie) - !!! Dangereux pour l'eeprom à long terme !!!
	if (BootMode==4) EEPROM.update(110+p, 1);
	#if (DEBUGtoSERIAL == 1)
		DebugSerial.print(F("SetPin "));
		DebugSerial.print(p);
		DebugSerial.println(F(" to 1"));
	#endif
}
void PinWriteLOW(long p)
{
	digitalWrite(p, LOW);
	swtch[p]=0;
	jeedom += '&';
	jeedom += p;
	jeedom += F("=0");
	// Si bootmode=4 sauvegarde de l'etat de la pin (en sortie) - !!! Dangereux pour l'eeprom à long terme !!!
	if (BootMode==4) EEPROM.update(110+p, 0);
}

void Init_EEPROM()
{
	// Un marqueur
	EEPROM.update(13,  'J');		 // JEEDOUINO

	// BootMode choisi au demarrage de l'arduino
	// 0 = Pas de sauvegarde - Toutes les pins sorties non modifiï¿½es au dï¿½marrage.
	// 1 = Pas de sauvegarde - Toutes les pins sorties mises ï¿½ LOW au dï¿½marrage.
	// 2 = Pas de sauvegarde - Toutes les pins sorties mises ï¿½ HIGH au dï¿½marrage.
	// 3 = Sauvegarde sur JEEDOM - Toutes les pins sorties mises suivant leur sauvegarde dans Jeedom. Jeedom requis, sinon pins mises ï¿½ OFF.
	// 4 = Sauvegarde sur EEPROM- Toutes les pins sorties mises suivant leur sauvegarde dans l\'EEPROM. Autonome, mais durï¿½e de vie de l\'eeprom fortement rï¿½duite.
	EEPROM.update(14,  2);
	BootMode=2;

	// Initialisation par default
	EEPROM.update(15,	0);
	for (int i = 16; i < 200; i++)
	{
		EEPROM.update(i, 1);  // Valeur des pins OUT au 1er demarrage ( mes relais sont actis a 0, donc je met 1 pour eviter de les actionner au 1er boot)
	}
	// fin initialisation
}
//int freeRam ()
//{
//  extern int __heap_start, *__brkval;
//  int v;
//  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
//}
