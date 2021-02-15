/*
    Monitor CSI example for the Approximate Library (Processing client)
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021
*/

import processing.serial.*;

Serial esp32;

public class CSIPoint {
    public int subCarrier;
    public PVector v;
}

CSIPoint sample[] = new CSIPoint[52];

void setup() {
  size(1024, 480);
  esp32 = new Serial(this, "/dev/tty.SLAB_USBtoUART", 9600);
}

void draw() {
  String line;
   if (esp32.available() > 0) {
    line = esp32.readStringUntil('\n');
    if (line != null) {
      String[] csiPacket = split(line.trim(), '\t');
      
      if(csiPacket.length == 52) {
        for(int n = -26; n < 26; ++n) {
          CSIPoint p = new CSIPoint();
          int a = int(split(csiPacket[n+26], ',')[0]);    //real
          int bi = int(split(csiPacket[n+26], ',')[1]);   //imaginary
          p.v = new PVector(a, bi);
          p.subCarrier = n;
          
          sample[n+26] = p;
        }
        
        background(0);
        noFill();
        
        translate(0, height/2);
       
        stroke(50, 50, 50);
        line(0,0,width,0);
       
        int w = width/53;
        stroke(255, 0, 0);
        beginShape();
        for(int n = 0; n < 52; ++n) {
          curveVertex(w*n, map(sample[n].v.mag(), 0, 50, 0, -200));
        }
        endShape();
      }
    }
  }
}
