#pragma once

#include <memory>
#include <string>
#include <vector>

namespace launcherapp { namespace utility { namespace aes {

class AES
{
public:
	AES();
	~AES();

	bool SetPassphrase(const std::string& passPhrase);
	bool Encrypt(const std::vector<uint8_t>& plainData, std::vector<uint8_t>& cryptData);
	bool Decrypt(const std::vector<uint8_t>& cryptData, std::vector<uint8_t>& plainData);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}}  // end of namespace launcherapp::utility::aes
