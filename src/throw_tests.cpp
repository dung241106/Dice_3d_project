#include "throw_sim.hpp"
#include "sound.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace dice {
int testThrows(){
    int checks=0;auto check=[&](bool ok,const char* label){checks++;if(!ok)throw std::runtime_error(label);};
    try{
        for(int i=0;i<4;i++){
            auto wave=synthWave(static_cast<Sound>(i));
            check(wave.size()>44&&std::string(wave.begin(),wave.begin()+4)=="RIFF"&&std::string(wave.begin()+8,wave.begin()+12)=="WAVE","PCM WAV header");
            check(wave[22]==1&&wave[34]==16&&(wave.size()-44)%2==0,"Mono 16-bit PCM payload");
        }
        ThrowSimulation wall(42);wall.setCount(1);wall.launch(1);
        wall.bodies[0].die.rotation={};wall.bodies[0].die.position={4.5,1,0};wall.bodies[0].velocity={5,0,0};wall.bodies[0].angular={};wall.step(1.0/120);
        check(wall.bodies[0].velocity.x<0&&wall.wallHits>0,"Wall reverses outward velocity");
        ThrowSimulation bounce(42);bounce.setCount(1);bounce.launch(0.5);
        bounce.bodies[0].die.rotation={};bounce.bodies[0].die.position={0,half,0};bounce.bodies[0].velocity={0,-4,0};bounce.bodies[0].angular={};bounce.step(1.0/120);
        check(bounce.bodies[0].velocity.y>0&&bounce.tableHits>0,"Floor bounces descending die");
        ThrowSimulation pair(8);pair.setCount(2);pair.launch(0.5);
        for(int i=0;i<2;i++){
            auto& b=pair.bodies[i];
            b.die.position={i==0?-0.75:0.75,1.5,0};
            b.velocity={i==0?3.0:-3.0,0,0};b.angular={};
        }
        pair.step(1.0/120);
        check(pair.pairHits>0&&pair.pairImpact>0&&pair.bodies[0].velocity.x<0&&pair.bodies[1].velocity.x>0,"Dice exchange impulses and separate");
        ThrowSimulation weak(5),strong(5);weak.setCount(1);strong.setCount(1);weak.launch(0);strong.launch(1);
        check(strong.bodies[0].velocity.length()>weak.bodies[0].velocity.length(),"Charge affects initial speed");
        ThrowSimulation sim(2026),same(2026);
        check(!sim.setCount(0)&&!sim.setCount(4)&&!sim.launch(-0.1)&&!sim.launch(1.1),"Reject invalid throw inputs");
        for(int i=0;i<12;i++){
            Quat q=Quat::axisAngle({1,2,3},i*pi/11);Vec3 direction=Vec3{1,0.3,-0.6}.unit();
            check(std::abs(supportPoint(q,direction).dot(direction)-supportExtent(q,direction))<1e-9,"Support point and extent agree");
        }
        for(int round=0;round<90;round++){
            int count=round%3+1;double power=(round%11)/10.0;
            sim.setCount(count);same.setCount(count);check(sim.launch(power)&&same.launch(power),"Launch accepts 1..3 dice");
            check(!sim.launch(power)&&!sim.setCount(1),"Lock count and duplicate launch");
            sim.paused=true;double before=sim.elapsed;sim.step(0.1);check(sim.elapsed==before,"Pause freezes physics");sim.paused=false;
            bool done=false;
            for(int frame=0;frame<1250&&!done;frame++){
                done=sim.step(1.0/120);same.step(1.0/120);
                for(int i=0;i<count;i++){
                    const auto& d=sim.bodies[i].die;
                    check(d.position.y>=supportHeight(d.rotation)-1e-5,"No floor penetration");
                    check(std::abs(d.position.x)+supportExtent(d.rotation,{1,0,0})<=sim.wallX+1e-5,"Inside X walls");
                    check(std::abs(d.position.z)+supportExtent(d.rotation,{0,0,1})<=sim.wallZ+1e-5,"Inside Z walls");
                }
            }
            check(done&&!sim.rolling,"Simulation settles within bounded time");
            auto r=sim.history.back();check(r.count==count&&r.total()>=count&&r.total()<=count*6,"Active dice only in sum");
            check(r.values==same.history.back().values,"Same seed and input reproduce simulation");
            for(int i=0;i<count;i++){
                check(r.values[i]==topFace(sim.bodies[i].die.rotation),"CSV value equals actual upward face");
                Vec3 up{};for(auto f:faces())if(f.value==r.values[i])up=sim.bodies[i].die.rotation.rotate(f.normal);
                check(up.y>0.999999&&sim.bodies[i].sleeping,"Settled face is horizontal");
            }
            check(!sim.step(1),"Round cannot finish twice");
        }
        auto dir=std::filesystem::current_path()/("throw-test-"+std::to_string(std::random_device{}()));auto file=dir/"throws.csv";
        check(sim.save(file)&&sim.save(file),"History file creation and replacement");
        ThrowSimulation loaded(1);check(loaded.load(file)&&loaded.history.size()==90&&loaded.cumulative()==sim.cumulative(),"Throw history round trip");
        for(int i=0;i<loaded.count;i++)check(topFace(loaded.bodies[i].die.rotation)==loaded.history.back().values[i],"Reload restores displayed last result");
        {std::ofstream out(file);out<<"id,count,power,d1,d2,d3,total,duration,table_hits,wall_hits\n1,1,0.5,9,0,0,9,2,1,0\n";}
        check(!loaded.load(file)&&loaded.history.size()==90,"Reject corrupt file without mutating history");
        std::filesystem::remove(file);std::filesystem::remove(dir);
        std::cout<<"PASS: "<<checks<<" throw/audio checks (90 simulated rounds)\n";return 0;
    }catch(const std::exception& e){std::cerr<<"THROW FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}
}
} // namespace dice
