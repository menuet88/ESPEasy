#ifdef USES_P199
//#######################################################################################################
//#################################### Plugin 021: Level Control ########################################
//#######################################################################################################

#define PLUGIN_199
#define PLUGIN_ID_199       199
#define PLUGIN_NAME_199       "Aquarium - karmnik"
#define PLUGIN_VALUENAME1_199 "Output"

boolean Plugin_199(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static int timermainstate[TASKS_MAX];//licznik czasu
  static int timervibstate[TASKS_MAX];
  static int counter=0;


  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_199;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_199);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR("running"));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR("counter"));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("main motor"));
        event->String2 = formatGpioName_output(F("vibration motor"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // char tmpString[128];
      //addHtml(F("<TR><TD>Check Task:<TD>"));
        //addTaskSelect(F("p199_task"), PCONFIG(0));

        //LoadTaskSettings(PCONFIG(0)); // we need to load the values from another task for selection!
        //addHtml(F("<TR><TD>Check Value:<TD>"));
        //addTaskValueSelect(F("p199_value"), PCONFIG(1), PCONFIG(0));
        if(PCONFIG_FLOAT(0)==0)
          PCONFIG_FLOAT(0)=5;
        if(PCONFIG_FLOAT(1)==0)
          PCONFIG_FLOAT(1)=0.5;
        if(PCONFIG_FLOAT(2)==0)
          PCONFIG_FLOAT(2)=0.5;

      	addFormTextBox(F("Time on"), F("p199_on"), String(PCONFIG_FLOAT(0)), 8);

      	addFormTextBox(F("Vibration on time"), F("p199_vib_on"), String(PCONFIG_FLOAT(1)), 8);
        addFormTextBox(F("Vibration off time"), F("p199_vib_off"), String(PCONFIG_FLOAT(2)), 8);

    //    LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        float temp=getFormItemFloat(F("p199_on"));
        if(temp<0.1)temp=0.1;//zabezpieczenie sie przed zerowymi wartosciami
        PCONFIG_FLOAT(0) = temp;
        temp=getFormItemFloat(F("p199_vib_on"));
        if(temp<0.1)temp=0.1;
        PCONFIG_FLOAT(1) = temp;
        temp= getFormItemFloat(F("p199_vib_off"));
        if(temp<0.1)temp=0.1;
        PCONFIG_FLOAT(2) = temp;
        success = true;
        break;
      }

    case PLUGIN_SET_CONFIG://to sie wykonuje przy uzyciu: config,task,<taskname>,<time/interval>,<value>
      {
        String command = parseString(string, 1);
        if (command == F("time"))
        {
          String value = parseString(string, 2);
          float result=0;
          Calculate(value.c_str(), &result);
          PCONFIG_FLOAT(0) = result;
          SaveSettings();
          success = true;
        }
        else if (command == F("interval"))
        {
          String value = parseString(string, 2);
          float result=0;
          Calculate(value.c_str(), &result);
          PCONFIG_FLOAT(1) = result;
          SaveSettings();
          success = true;
        }
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("feeder"))
        {
          String value = parseString(string,2);//pobranie 1 parametru
          success = true;
          //TODO zabezpieczenie się przed ponownym uruchomieniem
          if(value==F("1"))//uruchomienie karmnika
          {
            if(timermainstate[event->TaskIndex])break;//jesli juz dziala, to wychodzimy
            timermainstate[event->TaskIndex]=PCONFIG_FLOAT(0)*10;//glowny timer
            timervibstate[event->TaskIndex]=PCONFIG_FLOAT(1)*10;//timer wibracji
            digitalWrite(CONFIG_PIN1,1);//oba wyjscia aktywne
            digitalWrite(CONFIG_PIN2,1);
            counter++;
            //wyslanie info o tym
            UserVar[event->BaseVarIndex]=1;
            UserVar[event->BaseVarIndex+1]=counter;
            sendData(event);

          }
          else if(value==F("0"))//zatrzymanie karmnika
          {
            if(!timermainstate[event->TaskIndex])break;//jesli juz nie dziala, to wychodzimy (MQTT sie nie zapelti )
            timermainstate[event->TaskIndex]=0;
            digitalWrite(CONFIG_PIN1,0);//oba wyjscia wylaczone
            digitalWrite(CONFIG_PIN2,0);
            //wyslanie info o tym
            UserVar[event->BaseVarIndex]=0;
            sendData(event);
          }

        }
        break;
      }

    case PLUGIN_GET_CONFIG:
      {
        String command = parseString(string, 1);
        if (command == F("getlevel"))
        {
          string = PCONFIG_FLOAT(0);
          success = true;
        }
        break;
      }

    case PLUGIN_INIT:
      {
        pinMode(CONFIG_PIN1, OUTPUT);
        pinMode(CONFIG_PIN2, OUTPUT);
        digitalWrite(CONFIG_PIN1,0);
        digitalWrite(CONFIG_PIN2,0);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (timermainstate[event->TaskIndex]){
          UserVar[event->BaseVarIndex]=1;
        } else {
          UserVar[event->BaseVarIndex]=0;
        }
        sendData(event);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
      //  int timermaintemp=timermainstate[event->TaskIndex];
      //  int timervibtemp=timervibstate[event->TaskIndex];
        if(timermainstate[event->TaskIndex])
        {//jesli karmmimy
          timermainstate[event->TaskIndex]--;
          if(!timermainstate[event->TaskIndex])//jak czas sie skonczy
          {
            //wylaczenie silniczkow
            digitalWrite(CONFIG_PIN1,0);
            digitalWrite(CONFIG_PIN2,0);
            timervibstate[event->TaskIndex]=0;//zakonczenie wibracji
            //wyslanie info o tym
            UserVar[event->BaseVarIndex]=0;
            sendData(event);
          }
          else
          {//ale jesli dziala
            timervibstate[event->TaskIndex]--;
            if(!timervibstate[event->TaskIndex])
            {//tooglowanie wibracja - rozne czasy stanu niskiego i wysokiego
              if(digitalRead(CONFIG_PIN2))
              {
                digitalWrite(CONFIG_PIN2,0);
                timervibstate[event->TaskIndex]=PCONFIG_FLOAT(2)*10;//ponowna inicjalizacja timera
              }
              else
              {
                digitalWrite(CONFIG_PIN2,1);
                timervibstate[event->TaskIndex]=PCONFIG_FLOAT(1)*10;//ponowna inicjalizacja timera
              }

            }
          }//if(!timermainstate[event->TaskIndex])//jak czas sie skonczy
        }//if(timermainstate[event->TaskIndex])

        success = true;
        break;
      }

  }
  return success;
}
#endif // USES_P199
