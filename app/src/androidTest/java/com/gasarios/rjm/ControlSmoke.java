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
    private File checkpoint;
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
    private int fires() throws Exception {
        Matcher m=Pattern.compile("RJM: fire").matcher(logs());int count=0;
        while(m.find()) count++;return count;
    }
    private String[] saved() throws Exception {return read(checkpoint).trim().split("\\s+");}
    private void ensureMode(int mode) throws Exception {
        require(menu()==2,"mode change needs pause");
        for(int i=0;i<4&&Integer.parseInt(saved()[6])!=mode;i++) tap(width/2,530*scale);
        require(Integer.parseInt(saved()[6])==mode,"four-mode setting failed");
    }
    private void screenshot(String name) throws Exception {
        Bitmap image=getUiAutomation().takeScreenshot();
        try(FileOutputStream out=new FileOutputStream(new File(getTargetContext().getFilesDir(),name))) {
            require(image!=null&&image.compress(Bitmap.CompressFormat.PNG,100,out),"screenshot failed");
        }
    }
    private void tap(float x,float y) {
        long now=SystemClock.uptimeMillis();
        MotionEvent down=MotionEvent.obtain(now,now,MotionEvent.ACTION_DOWN,x,y,0);
        down.setSource(InputDevice.SOURCE_TOUCHSCREEN);sendPointerSync(down);down.recycle();
        MotionEvent up=MotionEvent.obtain(now,SystemClock.uptimeMillis(),MotionEvent.ACTION_UP,x,y,0);
        up.setSource(InputDevice.SOURCE_TOUCHSCREEN);sendPointerSync(up);up.recycle();SystemClock.sleep(500);
    }
    private void pointers(long downTime,int action,float[] xs,float y) {
        float[] ys=new float[xs.length];java.util.Arrays.fill(ys,y);pointers(downTime,action,xs,ys);
    }
    private void pointers(long downTime,int action,float[] xs,float[] ys) {
        MotionEvent.PointerProperties[] properties=new MotionEvent.PointerProperties[xs.length];
        MotionEvent.PointerCoords[] coords=new MotionEvent.PointerCoords[xs.length];
        for(int i=0;i<xs.length;i++) {
            properties[i]=new MotionEvent.PointerProperties();properties[i].id=i;properties[i].toolType=MotionEvent.TOOL_TYPE_FINGER;
            coords[i]=new MotionEvent.PointerCoords();coords[i].x=xs[i];coords[i].y=ys[i];coords[i].pressure=1;coords[i].size=1;
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
            checkpoint=new File(getTargetContext().getFilesDir(),"checkpoint-v1.txt");
            String[] before=read(checkpoint).trim().split("\\s+");
            require(before[0].equals("RJM_CHECKPOINT_2"),"v2 save missing");
            int oldMode=Integer.parseInt(before[6]);float oldZoom=Float.parseFloat(before[7]);
            tap(width/2,530*scale);
            String[] changed=read(checkpoint).trim().split("\\s+");
            require(Integer.parseInt(changed[6])==(oldMode+1)%4,"aim setting not saved");
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
            screenshot("control-smoke.png");
            float cx=width-150*scale,cy=564*scale,right=cx+60*scale;
            for(int mode=2;mode<=3;mode++) {
                ensureMode(mode);
                tap(width/2,353*scale); // Restart field restores all magazines.
                SystemClock.sleep(1500);require(menu()==0,"restart did not resume");
                int count=fires();tap(width/2,300*scale);
                require(fires()==count,"world tap fires in pad mode "+mode);
                now=SystemClock.uptimeMillis();
                pointers(now,MotionEvent.ACTION_DOWN,new float[]{cx},cy);SystemClock.sleep(500);
                require(fires()==count,"pad center fires");
                pointers(now,MotionEvent.ACTION_MOVE,new float[]{right},cy);SystemClock.sleep(1100);
                require(fires()>=count+2,"semi-auto pad hold does not repeat");
                screenshot(mode==2?"pad-3-held.png":"pad-4-held.png");
                pointers(now,MotionEvent.ACTION_MOVE,new float[]{cx},cy);
                count=fires();SystemClock.sleep(700);require(fires()==count,"neutral does not stop");
                pointers(now,MotionEvent.ACTION_MOVE,new float[]{right},cy);SystemClock.sleep(500);
                require(fires()>count,"center reentry does not resume");
                if(mode==2) {
                    // A left-button pointer must stay UI-owned, block the held
                    // pad, and never resume firing after the whole-set reload.
                    pointers(now,MotionEvent.ACTION_POINTER_DOWN|(1<<MotionEvent.ACTION_POINTER_INDEX_SHIFT),
                        new float[]{right,70*scale},new float[]{cy,510*scale});
                    count=fires();SystemClock.sleep(2300);
                    require(fires()==count,"left reload retained pad fire");
                    pointers(now,MotionEvent.ACTION_POINTER_UP|(1<<MotionEvent.ACTION_POINTER_INDEX_SHIFT),
                        new float[]{right,70*scale},new float[]{cy,510*scale});
                    pointers(now,MotionEvent.ACTION_MOVE,new float[]{cx},cy);
                    pointers(now,MotionEvent.ACTION_MOVE,new float[]{right},cy);SystemClock.sleep(500);
                    require(fires()==count,"reload rearms without actual release");
                    screenshot("pad-reload-blocked.png");
                }
                pointers(now,MotionEvent.ACTION_UP,new float[]{right},cy);
                count=fires();SystemClock.sleep(700);require(fires()==count,"release retained firing");
                screenshot(mode==2?"pad-3-released.png":"pad-4-released.png");
                tap(right,cy);require(fires()==count+1,"short pad tap not exactly one shot");
                count=fires();SystemClock.sleep(700);require(fires()==count,"short tap repeats");
                sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);SystemClock.sleep(500);
                require(menu()==2,"pad Back did not pause");
                Log.i("RJM_SMOKE","PASS pad="+(mode+1)+" hold, neutral, release, short tap and world exclusion");
            }
            require(Integer.parseInt(saved()[6])==3,"fourth mode not saved");
            screenshot("four-mode-settings.png");
            Log.i("RJM_SMOKE","PASS mode="+saved()[6]+" zoom="+oldZoom+" -> "+newZoom);
            result.putString("stream","PASS native two-finger pinch, four modes, pad hold/neutral/release, reload gate and saved aim/zoom\n");
            finish(Activity.RESULT_OK,result);
        } catch(Throwable failure) {
            Log.e("RJM_SMOKE","Control smoke failed",failure);
            result.putString("stream","FAIL "+failure+"\n");finish(Activity.RESULT_CANCELED,result);
        }
    }
}
