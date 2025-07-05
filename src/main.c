#include <sulkan/sulkan.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    skWindow window = skWindow_Create("Sulkan", 1920, 1080, false, false);

    while (!skWindow_ShouldClose(&window))
    {
        

        skWindow_Update(&window);
    }

    skWindow_Close(&window);

    return 0;
}
