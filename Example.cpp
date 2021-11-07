#include "Fireworks.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class FireworksExample : public olc::PixelGameEngine {
public:
	FW::FireworkShow* show;

	FireworksExample() {
		sAppName = "Fireworks Demo";
	}

	bool OnUserCreate() override
	{
		unsigned int time_ui = unsigned int(time(NULL));
		srand(time_ui);

		show = new FW::FireworkShow(*this);

		return true;
	}

	bool OnUserDestroy() override {
		show->Cleanup();
		delete show;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		show->Update(fElapsedTime);
		show->Show();
		
		return true;
	}
};

int main() {
	FireworksExample demo;
	if (demo.Construct(1280, 720, 1, 1))
		demo.Start();

	return 0;
}