#include "RecoilJumpMan/Mobile/TouchControls.h"
#include "RecoilJumpMan/Mobile/MobileAim.h"
#include "RecoilJumpMan/Mobile/ReloadQueue.h"
#include "RecoilJumpMan/Mobile/WeaponInput.h"
#include "RecoilJumpMan/Core/ViewportScaler.h"
#include "RecoilJumpMan/Camera/GameCamera.h"
#include "RecoilJumpMan/Combat/AimAssistResolver.h"
#include "RecoilJumpMan/Physics/RecoilMovementController.h"
#include "RecoilJumpMan/Physics/TileCollisionResolver.h"
#include "RecoilJumpMan/World/TileMap.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace rjm;
using namespace rjm::mobile;
void Require(bool value,const char* name) {if(!value) throw std::runtime_error(name);}
bool Near(float a,float b) {return std::fabs(a-b)<0.01f;}
void Down(TouchControls& c,int id,Vector2 p) {c.Process({TouchPhase::Down,id,p});}
void Up(TouchControls& c,int id) {c.Process({TouchPhase::Up,id,{}});}
void Move(TouchControls& c,int id,Vector2 p) {c.Process({TouchPhase::Move,id,p});}
void Cancel(TouchControls& c) {c.Process({TouchPhase::Cancel,-1,{}});}
Vector2 Project(const GameCamera& camera,Vector2 world) {
    const auto& c=camera.RawCamera();
    return {c.offset.x+(world.x-c.target.x)*c.zoom,c.offset.y+(-world.y-c.target.y)*c.zoom};
}
int main() {
    TouchControls c;c.BeginFrame();
    Down(c,0,{600,500});Up(c,0);Require(c.FirePressed()&&!c.FireHeld(),"short tap lost");
    c.BeginFrame();Require(!c.FirePressed(),"press repeats");
    Down(c,1,{600,400});Down(c,2,{70,420});
    Require(c.FireHeld()&&c.Bracing()&&!c.Pinching(),"brace+fire becomes pinch");
    Up(c,2);Require(c.FireHeld()&&!c.Bracing(),"pointer ownership");
    c.BlockFireUntilRelease();c.BeginFrame();Require(!c.FireHeld(),"empty input fires after reload");
    Up(c,1);Down(c,1,{800,400});Require(c.FireHeld(),"release does not rearm");
    Cancel(c);c.BeginFrame();Down(c,0,{70,590});
    Require(c.CurrentMenu()==Menu::Weapons&&!c.FirePressed(),"weapon UI leaks shot");
    Down(c,1,{550,300});Require(c.SelectedSlot()==-1,"menu gate");
    Up(c,0);Up(c,1);c.BeginFrame();Down(c,0,{550,300});
    Require(c.SelectedSlot()==1&&c.CurrentMenu()==Menu::Gameplay&&!c.FirePressed(),"selection leaks shot");
    Move(c,0,{800,500});Require(!c.FireHeld(),"menu pointer becomes fire");
    Up(c,0);c.BeginFrame();Down(c,0,{600,400});c.Back();
    Require(c.CurrentMenu()==Menu::Pause&&!c.FireHeld(),"back retains input");
    Up(c,0);c.BeginFrame();c.Back();Down(c,0,{600,400});
    Require(c.FirePressed(),"back blocks new tap");
    c.BeginFrame();Move(c,0,{607,400});Require(!c.AimChanged(),"jitter redirects");
    Move(c,0,{620,400});Require(c.AimChanged(),"drag ignored");
    Cancel(c);c.Pause();c.BeginFrame();c.Back();Down(c,4,{200,300});
    Require(c.FirePressed(),"cancel leaves gate");
    Cancel(c);c.BeginFrame();c.Process({TouchPhase::Down,2,{-30,200},false});
    Require(!c.FirePressed(),"outside viewport fires");
    std::cout<<"PASS short taps, brace/fire ownership, menus, lifecycle cancel and drag\n";

    Cancel(c);c.BeginFrame();Down(c,4,{400,300});Down(c,7,{800,300});
    Require(c.Pinching()&&!c.FirePressed()&&!c.FireHeld(),"pinch leaks initial batched shot");
    Move(c,4,{300,300});Move(c,7,{900,300});
    Require(Near(c.PinchScale(),1.5f),"pinch ratio/IDs");
    c.BeginFrame();Require(Near(c.PinchScale(),1),"pinch repeats last frame zoom");
    Move(c,4,{400,300});Move(c,7,{800,300});Require(Near(c.PinchScale(),2.0f/3),"pinch contraction");
    Up(c,4);c.BeginFrame();Move(c,7,{1000,300});
    Require(!c.FireHeld()&&!c.FirePressed()&&c.Pinching(),"remaining pinch finger becomes shot");
    Down(c,8,{700,500});Require(!c.FirePressed(),"third pinch finger shoots");
    Up(c,7);Up(c,8);c.BeginFrame();Down(c,0,{800,400});Require(c.FirePressed(),"pinch never rearms");
    Cancel(c);c.Pause();c.BeginFrame();Down(c,0,{230,650});Down(c,1,{1050,650});
    Move(c,0,{180,650});Require(c.Pinching()&&c.PinchScale()>1&&!c.FireHeld(),"paused zoom blocked");
    Cancel(c);c.Back();c.SetScreenSize(1560,720);c.BeginFrame();
    Down(c,0,{70,590});Up(c,0);c.BeginFrame();Down(c,0,{690,350});
    Require(c.SelectedSlot()==1&&c.CurrentMenu()==Menu::Gameplay,"wide panel hitbox mismatch");
    Cancel(c);c.BeginFrame();Down(c,0,{800,400});c.SetScreenSize(1600,720);
    Require(!c.FireHeld()&&!c.Pinching(),"resize retains stale pointer");
    Cancel(c);c.BeginFrame();Down(c,0,{400,300});Down(c,1,{800,300});
    Move(c,0,{300,300});Move(c,1,{900,300});Up(c,0);Up(c,1);
    Require(!c.Pinching()&&Near(c.PinchScale(),1.5f)&&!c.FirePressed(),"batched pinch loses final zoom / fires");
    std::cout<<"PASS pinch expansion/contraction, release gate, pause zoom and wide UI\n";

    MobileAim aim;aim.UpdateScreen({100,100},{100,0},{1280,720},AimMode::Character);
    Vector2 target=aim.Target({550,600});Require(aim.Valid()&&Near(target.x,550)&&Near(target.y,1600),"screen Y / held direction");
    aim.UpdateScreen({550,600},{650,600},{1280,720},AimMode::Character);
    Require(Near(aim.Target({550,600}).x,1550),"drag cannot redirect");
    aim.UpdateScreen({100,100},{101,102},{1280,720},AimMode::Character);Require(!aim.Valid(),"dead zone fires");
    aim.UpdateScreen({1530,680},{1000,360},{1560,720},AimMode::ScreenCenter);
    Require(aim.Valid()&&Near(aim.Direction().x,1)&&Near(aim.Direction().y,0),"center mode follows corner player");
    aim.UpdateScreen({1530,680},{1000,360},{1560,720},AimMode::Character);
    Require(aim.Direction().x<0&&aim.Direction().y>0,"character mode replaced by center");
    aim.UpdateScreen({1530,680},{785,362},{1560,720},AimMode::ScreenCenter);Require(!aim.Valid(),"center dead zone");
    ViewportScaler viewport(1280,720);viewport.Update(3120,1440);
    Require(!viewport.ContainsWindowPoint({0,720}),"desktop aspect changed");
    viewport.UpdateFullWidth(3120,1440);Vector2 p=viewport.WindowToVirtual({1560,720});
    Require(viewport.VirtualWidth()==1560&&Near(p.x,780)&&Near(p.y,360)&&viewport.ContainsWindowPoint({0,720}),"full-width mapping");
    GameCamera camera;camera.SetViewportSize(1560,720);camera.SetWorldBounds({-10000,-10000,20000,20000});
    camera.SnapTo({300,300});Vector2 anchor{380,340},before=Project(camera,anchor);
    camera.SetZoomAnchored(1.6f,anchor);Vector2 after=Project(camera,anchor);
    Require(Near(before.x,after.x)&&Near(before.y,after.y),"zoom shifts player anchor");
    camera.SetZoomAnchored(100,anchor);Require(Near(camera.Zoom(),1.8f),"zoom upper bound");
    camera.SetZoomAnchored(0.01f,anchor);Require(Near(camera.Zoom(),0.65f),"zoom lower bound");
    camera.SetWorldBounds({0,0,100,100});camera.SetZoomAnchored(0.8f,{50,50});
    Require(std::isfinite(camera.RawCamera().target.x)&&std::isfinite(camera.RawCamera().target.y),"collapsed bounds zoom");
    std::cout<<"PASS both aim origins, zoom-independent dead zone, aspect and anchored/clamped zoom\n";

    WeaponDefinition def;def.id="test";def.magazineSize=1;def.recoilForce=600;
    def.damage=12;def.bulletSpeed=900;def.reloadSeconds=0.25f;def.fireCooldownSeconds=0.01f;
    Gun gun(def);auto shot=gun.TryFire(ShotAim{{1,0},{0,-1}});
    Require(shot.fired&&shot.recoil.y>0&&Near(shot.recoil.x,0)&&shot.projectiles[0].velocity.x>0,"assist changes recoil");
    Require(!gun.TryReload(false,ReloadIntent::Manual),"air reload");
    WeaponInventory weapons;weapons.Equip(0,gun);
    auto slow=def;slow.reloadSeconds=1.75f;weapons.Equip(1,Gun(slow));
    auto full=def;full.reloadSeconds=1.0f;weapons.Equip(2,Gun(full));weapons.Select(1);
    ReloadQueue queue;queue.Request();queue.Update(weapons,false);weapons.Update(100,false);
    Require(queue.Pending()&&weapons.At(0)->AmmoInMagazine()==0,"air queue expires");
    Require(Near(weapons.CalculateSetReloadSeconds(),1.0f),"average ignores full magazines");
    queue.Update(weapons,true);Require(!queue.Pending()&&weapons.At(0)->IsReloading()&&weapons.At(1)->IsReloading(),"set not reloaded together");
    weapons.Update(100,false);Require(weapons.At(0)->AmmoInMagazine()==0,"airborne reload advances");
    weapons.Update(0.9f,true);Require(weapons.At(0)->IsReloading(),"individual time overrides set average");
    weapons.Update(0.11f,true);Require(!weapons.IsAnyReloading()&&weapons.At(0)->AmmoInMagazine()==1&&weapons.CurrentSlot()==1,"set finish/selection");
    weapons.At(0)->TryFire(Vector2{1,0});auto batch=weapons.TryReloadAll(true,ReloadIntent::Manual);
    weapons.Update(0.5f,true);queue.Request();queue.Update(weapons,true);weapons.Update(0.51f,true);
    Require(batch.StartedAny()&&!weapons.IsAnyReloading()&&!queue.Pending(),"repeat resets reload timer");
    WeaponInventory lower;def.reloadSeconds=0.01f;lower.Equip(0,Gun(def));Require(Near(lower.CalculateSetReloadSeconds(),0.2f),"reload lower clamp");
    def.reloadSeconds=9;lower.Equip(0,Gun(def));Require(Near(lower.CalculateSetReloadSeconds(),2.0f),"reload upper clamp");
    std::cout<<"PASS set-wide non-expiring queue and exact original average/clamp/timer\n";

    def.reloadSeconds=1;WeaponInventory switching;switching.Equip(0,Gun(def));switching.Equip(1,Gun(def));
    auto automatic=def;automatic.defaultFireMode=FireControlMode::FullAuto;automatic.supportsFullAuto=true;
    switching.Equip(2,Gun(automatic));switching.At(0)->TryFire(Vector2{1,0});
    auto request=ResolveWeaponFire(switching,true,false);
    Require(request.switched&&request.fire&&switching.CurrentSlot()==1&&!request.reload,"empty press doesn't switch/fire");
    switching.At(1)->TryFire(Vector2{1,0});request=ResolveWeaponFire(switching,true,false);
    Require(request.switched&&request.fire&&switching.CurrentSlot()==2,"slot order / short auto tap");
    switching.At(2)->TryFire(Vector2{1,0});switching.Update(1,true);switching.At(0)->FillMagazine();
    request=ResolveWeaponFire(switching,false,true);
    Require(request.switched&&!request.fire&&switching.CurrentSlot()==0,"held SMG fires semi-auto on switch");
    request=ResolveWeaponFire(switching,true,false);Require(request.fire,"fresh semi-auto press blocked");
    switching.At(0)->TryFire(Vector2{1,0});request=ResolveWeaponFire(switching,true,false);
    Require(request.reload&&!request.fire&&!request.switched,"all empty doesn't reserve");
    request=ResolveWeaponFire(switching,false,true);Require(!request.reload,"hold alone queues reload");
    switching.At(1)->FillMagazine();switching.At(1)->StartReload(1);
    request=ResolveWeaponFire(switching,true,false);
    Require(!request.reload&&!request.switched&&!request.fire,"unready loaded weapon treated as empty set");
    std::cout<<"PASS original auto-switch order, readiness, full/semi transition and all-empty input\n";

    AimAssistResolver resolver;AimAssistTuning tuning;tuning.directionOnly=true;tuning.maxCorrectionDegrees=6;
    tuning.useAssistedTargetForRecoil=false;tuning.viewportSize={1560,720};
    AimAssistCandidate candidate;candidate.worldPosition={500,20};candidate.screenPosition={1300,300};
    auto assisted=resolver.Resolve({0,0},{1000,0},{800,360},{candidate},tuning);
    Require(assisted.assisted&&Near(assisted.recoilWorldTarget.y,0)&&!assisted.lockOn,"central assist uses touch point or recoil");
    candidate.worldPosition={500,100};Require(!resolver.Resolve({0,0},{1000,0},{800,360},{candidate},tuning).assisted,"assist exceeds angle");
    candidate.worldPosition={500,20};candidate.hasLineOfSight=false;
    Require(!resolver.Resolve({0,0},{1000,0},{800,360},{candidate},tuning).assisted,"center assist through walls");
    candidate.hasLineOfSight=true;candidate.screenPosition.x=2000;
    Require(!resolver.Resolve({0,0},{1000,0},{800,360},{candidate},tuning).assisted,"center assist off screen");
    std::cout<<"PASS center-direction bullet assist, angular bounds and unchanged recoil\n";

    RecoilMovementController movement;Vector2 normal{},braced{};
    movement.ApplyRecoilImpulse(normal,{600,600});movement.ApplyBracedRecoilImpulse(braced,{600,600});
    Require(normal.y>0&&braced.y>=0&&braced.y<normal.y&&std::fabs(braced.x)<std::fabs(normal.x),"brace reduction");
    TileMap map;map.Resize(10,5,32);for(int x=0;x<10;++x) map.SetCollisionFlags(x,0,TileCollisionSolid);
    TileCollisionResolver collision;auto landed=collision.MoveBox({150,52},{0,-300},16,1.0f/30,map);
    Require(landed.grounded&&Near(landed.position.y,48)&&Near(landed.velocity.y,0),"tile landing");
    std::cout<<"PASS original recoil, brace and tile landing\n";
}
