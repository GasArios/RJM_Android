#include "RecoilJumpMan/Mobile/Platform.h"
#include <atomic>
#include <mutex>
#ifdef __ANDROID__
#include <jni.h>
#include <android/input.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#endif

namespace {
std::mutex eventMutex, storageMutex;
std::vector<rjm::mobile::TouchEvent> pending;
std::atomic<bool> back{false}, pause{false};
std::string storage=".";
void Enqueue(rjm::mobile::TouchEvent event) {
    std::lock_guard<std::mutex> lock(eventMutex);
    // Keep down/up/cancel events. Coalesce consecutive motion for the same pointer.
    if (event.phase==rjm::mobile::TouchPhase::Move && !pending.empty() &&
        pending.back().phase==event.phase && pending.back().id==event.id) pending.back()=event;
    else pending.push_back(event);
}
}
namespace rjm::mobile {
std::vector<TouchEvent> DrainTouchEvents() {
    std::lock_guard<std::mutex> lock(eventMutex);
    std::vector<TouchEvent> events; events.swap(pending); return events;
}
bool ConsumeBack() { return back.exchange(false); }
bool ConsumePause() { return pause.exchange(false); }
std::string StoragePath() { std::lock_guard<std::mutex> lock(storageMutex); return storage; }
}
#ifdef __ANDROID__
extern "C" JNIEXPORT void JNICALL Java_com_gasarios_rjm_RjmActivity_nativeBack(JNIEnv*,jclass) { back=true; }
extern "C" JNIEXPORT void JNICALL Java_com_gasarios_rjm_RjmActivity_nativePause(JNIEnv*,jclass) { pause=true; }
extern "C" JNIEXPORT void JNICALL Java_com_gasarios_rjm_RjmActivity_nativeStorage(JNIEnv* env,jclass,jstring path) {
    const char* value=env->GetStringUTFChars(path,nullptr);
    if(value) { std::lock_guard<std::mutex> lock(storageMutex); storage=value; env->ReleaseStringUTFChars(path,value); }
}
// Invoked by our small hook in the pinned raylib Android backend.
extern "C" int rjm_android_input(android_app* app,AInputEvent* event) {
    using namespace rjm::mobile;
    if(AInputEvent_getType(event)==AINPUT_EVENT_TYPE_KEY) {
        if(AKeyEvent_getKeyCode(event)==AKEYCODE_BACK) {
            if(AKeyEvent_getAction(event)==AKEY_EVENT_ACTION_UP) back=true;
            return 1;
        }
        return 0;
    }
    if(AInputEvent_getType(event)!=AINPUT_EVENT_TYPE_MOTION ||
        (AInputEvent_getSource(event)&AINPUT_SOURCE_TOUCHSCREEN)!=AINPUT_SOURCE_TOUCHSCREEN) return 0;
    const int action=AMotionEvent_getAction(event), masked=action&AMOTION_EVENT_ACTION_MASK;
    if(masked==AMOTION_EVENT_ACTION_CANCEL) { Enqueue({TouchPhase::Cancel,-1,{}}); return 1; }
    if(!app->window) return 1;
    const float sx=static_cast<float>(GetScreenWidth())/ANativeWindow_getWidth(app->window);
    const float sy=static_cast<float>(GetScreenHeight())/ANativeWindow_getHeight(app->window);
    auto emit=[&](TouchPhase phase,size_t index) {
        Enqueue({phase,AMotionEvent_getPointerId(event,index),{AMotionEvent_getX(event,index)*sx,AMotionEvent_getY(event,index)*sy}});
    };
    if(masked==AMOTION_EVENT_ACTION_MOVE) {
        for(size_t i=0;i<AMotionEvent_getPointerCount(event);++i) emit(TouchPhase::Move,i);
    } else {
        const size_t index=(action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)>>AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        if(masked==AMOTION_EVENT_ACTION_DOWN || masked==AMOTION_EVENT_ACTION_POINTER_DOWN) emit(TouchPhase::Down,index);
        if(masked==AMOTION_EVENT_ACTION_UP || masked==AMOTION_EVENT_ACTION_POINTER_UP) emit(TouchPhase::Up,index);
    }
    return 1;
}
#endif
