#include "framecounter.h"
#include "myglheaders.h"

unsigned counter = 0;
double last_time = 0.0;
double cur_time = 0.0;
double delta_time = 0.0;

void frameCompleted()
{
    counter++;
    cur_time = glfwGetTime();
    delta_time = cur_time - last_time;
    last_time = cur_time;
}
unsigned frameCounter()
{
    return counter;
}
double frameSeconds()
{
    return delta_time;
}
double timeElapsed()
{
    return cur_time;
}