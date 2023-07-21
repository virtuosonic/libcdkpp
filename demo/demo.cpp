#include "../cdk.hpp"
#include <iostream>

int main()
{
	cdk::screen screen(initscr());
	cdk::label label;
	label = cdk::label(screen, {CENTER,5}, {"libcdkpp","demo","1.0"}, {});
	cdk::button button(screen, {CENTER,10},"Press me",NULL, {true,true});
	screen.refresh();
	button.activate(NULL);
	return 0;
}
