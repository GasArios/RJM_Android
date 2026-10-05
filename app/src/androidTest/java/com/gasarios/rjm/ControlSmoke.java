package com.gasarios.rjm;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.os.SystemClock;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/** A separate test APK; no test hooks or input injectors ship in the game. */
public final class ControlSmoke extends Instrumentation {
    private float width, scale;
    @Override public void onCreate(Bundle args) {super.onCreate(args);start();}
    private void require(boolean value,String message) {if(!value) throw new AssertionError(message);}
    private String read(File file) throws Exception {
        try(FileInputStream in=new FileInputStream(file)) {return new String(in.readAllBytes(),StandardCharsets.UTF_8);}
    }
    private String logs() throws Exception {
        ParcelFileDescriptor fd=getUiAutomation().executeShellCommand("logcat -d -s raylib RJM_SMOKE");
        try(ParcelFileDescriptor.AutoCloseInputStream in=new ParcelFileDescriptor.AutoCloseInputStream(fd)) {
            return new String(in.readAllBytes(),StandardCharsets.UTF_8);
        }
    }
    private int menu() throws Exception {
        Matcher m=Pattern.compile("RJM: menu=(\\d+)").matcher(logs());int result=0;
        while(m.find()) result=Integer.parseInt(m.group(1));return result;
    }
    private void tap(float x,float y) {
        long now=SystemClock.uptimeMillis();
        MotionEvent down=MotionEvent.obtain(now,now,MotionEvent.ACTION_DOWN,x,y,0);
        down.setSource(InputDevice.SOURCE_TOUCHSCREEN);sendPointerSync(down);down.recycle();
        MotionEvent up=MotionEvent.obtain(now,SystemClock.uptimeMillis(),MotionEvent.ACTION_UP,x,y,0);
        up.setSource(InputDevice.SOURCE_TOUCHSCREEN);sendPointerSync(up);up.recycle();SystemClock.sleep(500);
    }
    private void pointers(long downTime,int action,float[] xs,float y) {
        MotionEvent.PointerProperties[] properties=new MotionEvent.PointerProperties[xs.length];
        MotionEvent.PointerCoords[] coords=new MotionEvent.PointerCoords[xs.length];
        for(int i=0;i<xs.length;i++) {
            properties[i]=new MotionEvent.PointerProperties();properties[i].id=i;properties[i].toolType=MotionEvent.TOOL_TYPE_FINGER;
            coords[i]=new MotionEvent.PointerCoords();coords[i].x=xs[i];coords[i].y=y;coords[i].pressure=1;coords[i].size=1;
        }
        MotionEvent e=MotionEvent.obtain(downTime,SystemClock.uptimeMillis(),action,xs.length,properties,coords,
            0,0,1,1,0,0,InputDevice.SOURCE_TOUCHSCREEN,0);
        sendPointerSync(e);e.recycle();SystemClock.sleep(120);
    }
    @Override public void onStart() {
        Bundle result=new Bundle();
        try {
            Intent launch=getTargetContext().getPackageManager().getLaunchIntentForPackage("com.gasarios.rjm");
            require(launch!=null,"launch intent missing");
            launch.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK|Intent.FLAG_ACTIVITY_NEW_TASK);
            Activity activity=startActivitySync(launch);SystemClock.sleep(5000);
            int[] size=new int[2];runOnMainSync(()->{size[0]=activity.getWindow().getDecorView().getWidth();size[1]=activity.getWindow().getDecorView().getHeight();});
            require(size[0]>size[1],"not landscape");width=size[0];scale=size[1]/720.0f;
            if(menu()==0) {sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);SystemClock.sleep(1000);}
            require(menu()==2,"Back did not pause");
            File checkpoint=new File(getTargetContext().getFilesDir(),"checkpoint-v1.txt");
            String[] before=read(checkpoint).trim().split("\\s+");
            require(before[0].equals("RJM_CHECKPOINT_2"),"v2 save missing");
            int oldMode=Integer.parseInt(before[6]);float oldZoom=Float.parseFloat(before[7]);
            tap(width/2,530*scale);
            String[] changed=read(checkpoint).trim().split("\\s+");
            require(Integer.parseInt(changed[6])==1-oldMode,"aim setting not saved");
            // Keep all pointers off pause buttons. The gesture must reach the
            // native NDK queue and save a changed zoom while remaining paused.
            Log.i("RJM_SMOKE","PAUSED_PINCH_BEGIN");
            long now=SystemClock.uptimeMillis();float y=650*scale;
            pointers(now,MotionEvent.ACTION_DOWN,new float[]{260*scale},y);
            pointers(now,MotionEvent.ACTION_POINTER_DOWN|(1<<MotionEvent.ACTION_POINTER_INDEX_SHIFT),new float[]{260*scale,width-260*scale},y);
            for(int step=1;step<=6;step++) {
                float x=(260-step*13)*scale;
                pointers(now,MotionEvent.ACTION_MOVE,new float[]{x,width-x},y);
            }
            float x=182*scale;
            pointers(now,MotionEvent.ACTION_POINTER_UP|(1<<MotionEvent.ACTION_POINTER_INDEX_SHIFT),new float[]{x,width-x},y);
            pointers(now,MotionEvent.ACTION_UP,new float[]{x},y);
            SystemClock.sleep(700);
            String[] zoomed=read(checkpoint).trim().split("\\s+");
            float newZoom=Float.parseFloat(zoomed[7]);
            require(newZoom>oldZoom+0.03f,"native pinch did not enlarge camera");
            require(menu()==2,"pinch resumed game");
            String recent=logs();int marker=recent.lastIndexOf("PAUSED_PINCH_BEGIN");
            require(marker>=0&&!recent.substring(marker).contains("RJM: fire"),"paused pinch fired");
            Bitmap screenshot=getUiAutomation().takeScreenshot();
            try(FileOutputStream out=new FileOutputStream(new File(getTargetContext().getFilesDir(),"control-smoke.png"))) {
                require(screenshot!=null&&screenshot.compress(Bitmap.CompressFormat.PNG,100,out),"screenshot failed");
            }
            Log.i("RJM_SMOKE","PASS mode="+zoomed[6]+" zoom="+oldZoom+" -> "+newZoom);
            result.putString("stream","PASS native two-finger pinch, paused input and saved aim/zoom\n");
            finish(Activity.RESULT_OK,result);
        } catch(Throwable failure) {
            Log.e("RJM_SMOKE","Control smoke failed",failure);
            result.putString("stream","FAIL "+failure+"\n");finish(Activity.RESULT_CANCELED,result);
        }
    }
}
