#pragma once

namespace utility {

// $B8z2L2;$r:F@8$9$k$?$a$N%/%i%9(B
class Sound
{
public:
	struct ITEM;
private:
	Sound();
	~Sound();

public:
	static Sound* Get();

	// $BHsF14|$G2;@<%U%!%$%k$r:F@8$9$k(B
	bool PlayAsync(LPCTSTR filePath);
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of utility

