i/*

  Prueba de web server con autoaprovisionamiento y envio de correo electrónico con la IP
  Marcelo Ferrarotti
  dia de los ñoquis de febrero (año bisiesto...claro)
  Al final del 2020 le agrego un leve manejo de servos...

  En uso la libreria de https://github.com/Hieromon/AutoConnect

  Para conectar a la WiFi, buscar un SSID "esp8266ap" con password "12345678"

  Acordarse de editar esto en VSCode y compilar con Arduino IDE!!! La linea 59 tiene un base64 que rompe el editor...

*/

//#include <ESP8266WiFi.h>          // WiFi.h para ESP32
//#include <ESP8266WebServer.h>     // WebServer.h para ESP32

// Cosas varias para el servo en el GPIO13 <--------- Al parecer esta biblioteca debe ir primero porque se arma bardo...
#include <Servo.h>
Servo myservo;
int pos = 0;

#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

//Datos para el correo electrónico
#include "ESP32_MailClient.h"
#define emailSenderAccount    "fulanodetal@gmail.com"
#define emailSenderPassword   "thekitcheninthemorning"
#define emailRecipient        "elreceptor@hotmail.com"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "DevKit ESP32 en marcha"
SMTPData smtpData;
void sendCallback(SendStatus info);




WebServer Server;
AutoConnect Portal(Server);

#define salida 2 //LED onboard

void rootPage() {
  char content[] = "Hola!";
  // Pagina web principal!
  String pagina = "<!DOCTYPE html>"
                  "<html>"
                  "<head>"
                  "<meta charset='utf-8' />"
                  "<title>Marcelo Web ESP32</title>"
                  "</head>"
                  "<body>"
                  "<center>"
                  "<h1>Marcelo Web ESP32</h1>"
                  "<img src='data:image/png;base64, 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' alt='Carita Feliz' />"
                  "<p><a href='/on'><button style='height:50px;width:100px'>ON</button></a></p>"
                  "<p><a href='/off'><button style='height:50px;width:100px'>OFF</button></a></p>"
                  "<p><a href='/flash'><button style='height:50px;width:100px'>Flash!</button></a></p>"
                  "<a href='/servito'><button style='height:50px;width:100px'>Servo!</button></a></p>"
                  "<p>Heap: " + String(ESP.getFreeHeap()) + " bytes</p>"
                  "<p><a href='/_ac'>Opciones</a></p>"
                  "</center>"
                  "</body>"
                  "</html>";

  Server.send(200, "text/html", pagina);
}

void servito() {
  Serial.println("Servo...");
  for (pos = 0; pos <= 190; pos += 1) { // de 0 a 190
    // in steps of 1 degree
    myservo.write(pos);                 // Se mueve el servo
    delay(5);                       
  }
  for (pos = 190; pos >= 0; pos -= 1) { // de 190 a 0
    myservo.write(pos);                 // Se mueve el servo
    delay(5);                           
  }
  rootPage();
}

void web_ON() {
  Serial.println("GPIO on");
  digitalWrite(salida, HIGH);
  rootPage();
}

void web_OFF() {
  Serial.println("GPIO off");
  digitalWrite(salida, LOW);
  rootPage();
}

void web_FLASH() {
  Serial.println("Fiestaaaaaa!!!");
  rootPage();
  for (int i = 0; i <= 21; i++) {   // Con numero impar queda igual que el inicio
    digitalWrite(salida, !digitalRead(salida));
    delay(50);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  pinMode(salida, OUTPUT);
  digitalWrite(salida, LOW);

  myservo.attach(13);

  Server.on("/", rootPage);
  Server.on("/on", web_ON);
  Server.on("/off", web_OFF);
  Server.on("/flash", web_FLASH);
  Server.on("/servito", servito);

  if (Portal.begin()) {
    Serial.println("WiFi conectada, IP: " + WiFi.localIP().toString());
  }
  mail_IP();
}

void loop() {
  Portal.handleClient();
}

void mail_IP() {
  // Mail de puesta en marcha
  // SMTP Server Email host, port, cuenta y password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // Nombre de fantasía del mail, por motivos de prolijidad
  smtpData.setSender("ESP32 de Marcelo", emailSenderAccount);

  // Prioridad del 1 al 5 (1 es muy alta)
  smtpData.setPriority("High");

  // Asunto
  smtpData.setSubject(emailSubject);

  // Cuerpo del mensaje en HTML + IP en uso
  smtpData.setMessage("<div style='color:#2f4468;'>http://" +  WiFi.localIP().toString() + "</h1><p>- IP actual</p></div>", true);

  // Destinatarios, se pueden agregar más de uno
  smtpData.addRecipient(emailRecipient); //smtpData.addRecipient("otra_cuenta@correo.com");

  //Envío en si del correo y llamado a función de status
  smtpData.setSendCallback(sendCallback);

  if (!MailClient.sendMail(smtpData))
    Serial.println("Error enviando mail, " + MailClient.smtpErrorReason());
  //Liberamos la memoria del proceso
  smtpData.empty();
}

// Función para ver el status
void sendCallback(SendStatus msg) {
  Serial.println(msg.info());


  if (msg.success()) {
    Serial.println("Enviado >----------------");
  }
}
