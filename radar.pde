//interfaccia con Processing
import processing.serial.*;
import java.util.ArrayList;

// Variabile per la comunicazione seriale
Serial myPort;             

int iAngle = 0;            // Angolo corrente del servo 
float iDistance = 0;       // Distanza misurata dal sensore in centimetri

/*
 * Classe Target: rappresenta un oggetto rilevato dal radar
 * Ogni target ha coordinate (x,y) e un valore alpha per l'effetto fade
 */
class Target {
  float x, y;              
  float alpha;             
  
  // Costruttore: crea un nuovo target con massima opacità
  Target(float x, float y) {
    this.x = x;
    this.y = y;
    this.alpha = 255;     
  }
}

// Lista dei target rilevati
ArrayList<Target> targets = new ArrayList<Target>();


float maxDistance = 40;    // Distanza massima di rilevamento in cm


void setup() {
  size(800, 800);          // Finestra quadrata 800x800 pixel
  smooth();                
  
  // Inizializza la comunicazione seriale
  myPort = new Serial(this, Serial.list()[0], 9600);
  myPort.clear();          
  myPort.bufferUntil('\n'); // Legge fino al carattere newline
}


void draw() {
  // Crea effetto fade dello sfondo
  fill(0, 10);             
  noStroke();
  rect(0, 0, width, height);
  
  // Sposta l'origine al centro della finestra per disegnare il radar
  pushMatrix();
  translate(width/2, height/2);
  
  // Disegna i componenti del radar nell'ordine corretto
  DrawRadarCircle();       // Griglia circolare del radar
  DrawRadarGreenLine();    // Linea verde che indica la direzione del servo
  DrawObjects();           // Punti rossi degli oggetti rilevati
  
  popMatrix();             // Ripristina il sistema di coordinate originale
  
  // Disegna le informazioni testuali nella parte inferiore
  DrawText();
}


void serialEvent(Serial myPort) {
  String data = myPort.readStringUntil('\n');
  if (data == null) return; //
  
  data = trim(data);         // Rimuove spazi e caratteri di fine riga
  
  // Separa i dati usando la virgola
  // Formato atteso: "angolo,distanza\n"
  String[] values = split(data, ',');
  
  if (values.length >= 2) {
    // Converte le stringhe in numeri
    iAngle = StringToInt(values[0]);      // Angolo del servo
    iDistance = StringToFloat(values[1]); // Distanza misurata
    
    // Valida i dati ricevuti
    if(iAngle >= 0 && iAngle <= 180 && iDistance > 0 && iDistance <= maxDistance) {

      float radarRadius = 0.45 * width;  // Raggio massimo del radar (45% della larghezza)
      
      // Mappa la distanza reale sul raggio del radar visualizzato
      float r = map(iDistance, 0, maxDistance, 0, radarRadius);
            
      // Converte in coordinate cartesiane (x, y)
      float x = r * cos(radians(iAngle));  // Componente X
      float y = r * sin(radians(iAngle));  // Componente Y
      
      // Aggiunge il nuovo target alla lista
      targets.add(new Target(x, y));
    }
  }
}

void DrawRadarCircle() {
  noFill();                    // Solo contorno, niente riempimento
  stroke(0, 255, 0, 150);     // Verde con trasparenza
  strokeWeight(2);             // Spessore linea
  
  float radarRadius = 0.45 * width;  // Raggio del radar
  
  // Disegna l'arco principale (semicerchio superiore)
  arc(0, 0, radarRadius*2, radarRadius*2, 0, PI);
  
  // Disegna le linee radiali ogni 30 gradi
  // Da 0° a 180° (da destra a sinistra)
  for(int angle = 0; angle <= 180; angle += 30){
    float rad = radians(angle);  // Converte in radianti
    line(0, 0, 
         radarRadius * cos(rad), 
         radarRadius * sin(rad));
  }
  
  // Disegna un cerchio ogni 10 cm
  for(int d = 10; d <= maxDistance; d += 10){
    float r = map(d, 0, maxDistance, 0, radarRadius);  // Mappa distanza su pixel
    arc(0, 0, r*2, r*2, 0, PI);  // Semicerchio
  }
}


void DrawRadarGreenLine() {
  stroke(0, 255, 0);          // linea verde centrale radar
  strokeWeight(3);            // spessore 
  float radarRadius = 0.45 * width;
  float rad = radians(iAngle);  // Usa l'angolo corrente del servo
  
  // Disegna la linea dal centro alla circonferenza
  line(0, 0, 
       radarRadius * cos(rad), 
       radarRadius * sin(rad));
}


void DrawObjects() { // Disegna i punti rossi degli oggetti rilevati
  strokeWeight(8);            // Punti grandi per buona visibilità
  
  // rimuovere elementi durante il loop
  for (int i = targets.size()-1; i >= 0; i--) {
    Target t = targets.get(i);
    
    // Colore rosso con trasparenza variabile
    stroke(255, 0, 0, t.alpha);
    point(t.x, t.y);          // Disegna il punto
    
    // Riduce gradualmente la trasparenza
    t.alpha -= 2;             // Velocità di dissolvenza
    
    // Rimuove il target quando diventa completamente trasparente
    if (t.alpha <= 0) {
      targets.remove(i);
    }
  }
}


void DrawText() { // Disegna le informazioni testuali nella parte inferiore
  // Sfondo nero per il testo
  fill(0);
  noStroke();
  rect(0, height*0.92, width, height*0.08);  // Rettangolo nella parte inferiore
  
  // Testo verde su sfondo nero
  fill(0, 255, 0);
  textSize(24);
  textAlign(LEFT);
  
  String distanceText;
  if (iDistance == 0) {
    distanceText = "---";     // Nessuna lettura valida
  } else {
    distanceText = nf(iDistance, 0, 1).replace('.', ',') + " cm";
  }
  
  // Visualizza le informazioni
  text("Angle: " + iAngle + "°", 280, height*0.96);
  text("Distance: " + distanceText, 480, height*0.96);
}

int StringToInt(String s){
  int value = 0;
  
  // Esamina ogni carattere della stringa
  for(int i = 0; i < s.length(); i++){
    char c = s.charAt(i);
    
    // Se è una cifra, la aggiunge al valore
    if(c >= '0' && c <= '9'){
      value = value * 10 + (c - '0');  // Converte char in int
    }
  }
  
  return value;
}


float StringToFloat(String s){
  float value = 0;
  
  try {
    value = Float.parseFloat(s);  
  } catch(Exception e) {
    // In caso di errore, restituisce 0
    println("Errore conversione float: " + s);
    value = 0;
  }
  
  return value;
}
