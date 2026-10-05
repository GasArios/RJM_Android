package com.gasarios.rjm;

import android.app.NativeActivity;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;

/** Owns Android lifecycle and system back; the C++ thread owns gameplay. */
public final class RjmActivity extends NativeActivity {
    static { System.loadLibrary("rjm"); }
    private static native void nativeBack();
    private static native void nativePause();
    private static native void nativeStorage(String path);
    private OnBackInvokedCallback backCallback;

    @Override public void onCreate(Bundle state) {
        nativeStorage(getFilesDir().getAbsolutePath());
        super.onCreate(state);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        if (Build.VERSION.SDK_INT >= 33) {
            backCallback = RjmActivity::nativeBack;
            getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                OnBackInvokedDispatcher.PRIORITY_DEFAULT, backCallback);
        }
        hideSystemBars();
    }
    @SuppressWarnings("deprecation")
    @Override public void onBackPressed() { nativeBack(); }
    @Override protected void onPause() { nativePause(); super.onPause(); }
    @Override public void onWindowFocusChanged(boolean focused) {
        if (!focused) nativePause();
        super.onWindowFocusChanged(focused);
        if (focused) hideSystemBars();
    }
    @Override protected void onDestroy() {
        if (Build.VERSION.SDK_INT >= 33 && backCallback != null)
            getOnBackInvokedDispatcher().unregisterOnBackInvokedCallback(backCallback);
        super.onDestroy();
    }
    @SuppressWarnings("deprecation")
    private void hideSystemBars() {
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    }
}
