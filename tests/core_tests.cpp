#include "RecoilJumpMan/Mobile/TouchControls.h"
#include "RecoilJumpMan/Mobile/MobileAim.h"
#include "RecoilJumpMan/Mobile/ReloadQueue.h"
#include "RecoilJumpMan/Core/ViewportScaler.h"
#include "RecoilJumpMan/Physics/RecoilMovementController.h"
#include "RecoilJumpMan/Physics/TileCollisionResolver.h"
#include "RecoilJumpMan/World/TileMap.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace rjm;
using namespace rjm::mobile;
void Require(bool condition,const char* name) { if(!condition) throw std::runtime_error(name); }
bool Near(float a,float b) { return std::fabs(a-b)<0.01f; }
void Down(TouchControls& c,int id,Vector2 p) { c.Process({TouchPhase::Down,id,p}); }
void Up(TouchControls& c,int id) { c.Process({TouchPhase::Up,id,{}}); }

int main() {
    TouchControls c; c.BeginFrame();
    // A down+up between render frames must retain exactly one semi-auto press.
    Down(c,0,{600,500}); Up(c,0);
    Require(c.FirePressed() && !c.FireHeld(),"short tap lost");
    c.BeginFrame(); Require(!c.FirePressed(),"press repeats");
    Down(c,1,{600,400}); Down(c,2,{70,420});
    Require(c.FireHeld() && c.Bracing(),"multi-touch fire+brace");
    Up(c,2); Require(c.FireHeld() && !c.Bracing(),"pointer ID ownership");
    c.BlockFireUntilRelease(); c.BeginFrame(); Require(!c.FireHeld(),"empty tap fires after reload");
    Up(c,1); Down(c,1,{800,400}); Require(c.FireHeld(),"fire not rearmed on release");
    c.Process({TouchPhase::Cancel,-1,{}}); Require(!c.FireHeld(),"cancel retains fire");
    c.BeginFrame(); Down(c,0,{70,590});
    Require(c.CurrentMenu()==Menu::Weapons && !c.FirePressed(),"weapon UI leaks shot");
    Down(c,1,{550,300}); Require(c.SelectedSlot()==-1,"menu touch gate");
    Up(c,0); Up(c,1); c.BeginFrame(); Down(c,0,{550,300});
    Require(c.SelectedSlot()==1 && c.CurrentMenu()==Menu::Gameplay && !c.FirePressed(),"weapon selection leaks shot");
    c.Process({TouchPhase::Move,0,{800,500}}); Require(!c.FireHeld(),"menu finger becomes fire");
    Up(c,0); c.BeginFrame(); Down(c,0,{600,400});
    c.Back(); Require(c.CurrentMenu()==Menu::Pause && !c.FireHeld(),"back retains input");
    Up(c,0); c.BeginFrame(); c.Back(); Down(c,0,{600,400});
    Require(c.FirePressed(),"back without active touch blocks next tap");
    c.Process({TouchPhase::Move,0,{603,400}}); c.BeginFrame();
    c.Process({TouchPhase::Move,0,{607,400}}); Require(!c.AimChanged(),"finger jitter redirects aim");
    c.Process({TouchPhase::Move,0,{620,400}}); Require(c.AimChanged(),"intentional drag ignored");
    c.Process({TouchPhase::Cancel,-1,{}}); c.Pause(); c.BeginFrame(); c.Back();
    Down(c,4,{200,300}); Require(c.FirePressed(),"lifecycle cancel leaves permanent gate");
    c.Process({TouchPhase::Cancel,-1,{}}); c.BeginFrame();
    c.Process({TouchPhase::Down,2,{-30,200},false}); Require(!c.FirePressed(),"letterbox fires");
    std::cout<<"PASS touch routing, short tap, pointer ownership, cancellation and menu gates\n";

    MobileAim aim; aim.Update({100,100},{100,0});
    Vector2 target=aim.Target({550,600});
    Require(aim.Valid() && Near(target.x,550) && Near(target.y,-400),"held direction changed with player");
    aim.Update({550,600},{650,600}); target=aim.Target({550,600});
    Require(Near(target.x,1550)&&Near(target.y,600),"drag cannot redirect");
    aim.Update({100,100},{101,102}); Require(!aim.Valid(),"zero length aim fired");
    ViewportScaler viewport(1280,720); viewport.Update(3120,1440);
    Vector2 p=viewport.WindowToVirtual({1560,720});
    Require(Near(p.x,640)&&Near(p.y,360)&&!viewport.ContainsWindowPoint({0,720}),"S24 letterbox mapping");
    std::cout<<"PASS held aim and screen mapping\n";

    WeaponDefinition def; def.id="test"; def.magazineSize=1; def.recoilForce=600;
    def.damage=12; def.bulletSpeed=900; def.reloadSeconds=0.25f; def.fireCooldownSeconds=0.01f;
    Gun gun(def); ShotResult shot=gun.TryFire(ShotAim{{1,0},{0,-1}});
    Require(shot.fired && shot.recoil.y>0 && Near(shot.recoil.x,0) && shot.projectiles[0].velocity.x>0,"projectile assist changes recoil");
    Require(!gun.TryReload(false,ReloadIntent::Manual),"air reload");
    WeaponInventory weapons; weapons.Equip(0,gun); weapons.Equip(1,Gun(def));
    ReloadQueue queue; queue.Request(0); weapons.Select(1);
    queue.Update(weapons,false); weapons.Update(100,false);
    Require(queue.Pending(0)&&weapons.At(0)->AmmoInMagazine()==0,"airborne reservation expires");
    queue.Update(weapons,true); Require(weapons.At(0)->IsReloading()&&!queue.Pending(0),"landing does not start original gun reload");
    weapons.Update(1,false); Require(weapons.At(0)->AmmoInMagazine()==0,"reload advances while airborne");
    weapons.Update(0.3f,true); Require(weapons.At(0)->AmmoInMagazine()==1&&weapons.CurrentSlot()==1,"reload switches gun or doesn't finish");
    std::cout<<"PASS projectile/recoil separation and per-gun grounded reload queue\n";

    RecoilMovementController movement; Vector2 normal{},braced{};
    movement.ApplyRecoilImpulse(normal,{600,600}); movement.ApplyBracedRecoilImpulse(braced,{600,600});
    Require(normal.y>0&&braced.y>=0&&braced.y<normal.y&&std::fabs(braced.x)<std::fabs(normal.x),"brace recoil reduction");
    TileMap map; map.Resize(10,5,32); for(int x=0;x<10;++x) map.SetCollisionFlags(x,0,TileCollisionSolid);
    TileCollisionResolver collision; auto landed=collision.MoveBox({150,52},{0,-300},16,1.0f/30,map);
    Require(landed.grounded&&Near(landed.position.y,48)&&Near(landed.velocity.y,0),"tile landing");
    std::cout<<"PASS original recoil, brace and tile landing\n";
}
