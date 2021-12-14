#include <time_measure.h>

#include <assert.h>

#include <GLFW/glfw3.h>

static int no_last_time = 1;
static double last_time = 0.0;
static double dt = 0.0;

void dt_on_new_frame()
{
    if (no_last_time)
    {
        last_time = glfwGetTime();
        no_last_time = 0;
    }

    double curr_time = glfwGetTime();
    dt = curr_time - last_time;
    last_time = curr_time;
}

double dt_get()
{
    assert(!no_last_time);
    return dt;
}
