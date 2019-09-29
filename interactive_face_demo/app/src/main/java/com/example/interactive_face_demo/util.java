package com.example.interactive_face_demo;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.util.Log;

public class util {
    public static Bitmap copyBitmap(Bitmap bitmap){
        return bitmap.copy(bitmap.getConfig(),true);
    }
    public static void drawRect(Bitmap bitmap, Rect rect){
        try {
            Canvas canvas = new Canvas(bitmap);
            Paint paint = new Paint();
            int r=255;//(int)(Math.random()*255);
            int g=0;//(int)(Math.random()*255);
            int b=0;//(int)(Math.random()*255);
            paint.setColor(Color.rgb(r, g, b));
            paint.setStrokeWidth(1+bitmap.getWidth()/500 );
            paint.setStyle(Paint.Style.STROKE);
            canvas.drawRect(rect, paint);
        }catch (Exception e){
            Log.i("Utils","[*] error"+e);
        }
    }
    public static void drawPoints(Bitmap bitmap, Point[] landmark){
        for (int i=0;i<landmark.length;i++){
            int x=landmark[i].x;
            int y=landmark[i].y;
            //Log.i("Utils","[*] landmarkd "+x+ "  "+y);
            drawRect(bitmap,new Rect(x-1,y-1,x+1,y+1));
        }
    }
}
