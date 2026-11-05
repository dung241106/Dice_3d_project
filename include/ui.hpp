#pragma once
#include <string>
#include <vector>

namespace ui {
struct Color {double r,g,b;};
constexpr Color background{0.035,0.060,0.10},panel{0.062,0.094,0.145},raised{0.10,0.145,0.21},
    white{0.92,0.95,0.99},muted{0.61,0.70,0.81},accent{0.30,0.84,0.79},gold{1.0,0.75,0.38};
struct Button {double x,y,w,h;int id;bool enabled;};
extern std::vector<Button> buttons;
void initializeFonts(double scale);
void releaseFonts();
void color(Color c);
void rectangle(double x,double y,double w,double h,Color c);
void box(double x,double y,double w,double h,Color c,double radius=12);
void text(double x,double y,const std::string& value,Color c=white,int size=18);
double textWidth(const std::string& value,int size=18);
void button(double x,double y,double w,const std::string& label,int id,bool selected=false,bool enabled=true,double h=38);
int hit(double x,double y);
} // namespace ui
