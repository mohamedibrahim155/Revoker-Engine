#include "ApplicationRenderer.h"

int main()
{
	ApplicationRenderer application;
	application.WindowInitialize(1920, 1080, "Ishtar Engine");
	application.Render();
	return 0;
}