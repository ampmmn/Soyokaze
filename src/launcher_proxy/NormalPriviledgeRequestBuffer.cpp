#include "NormalPriviledgeRequestBuffer.h"

struct NormalPriviledgeRequestBuffer::PImpl
{
	void DiscardLeadingNulls();

	std::string mData;
};

NormalPriviledgeRequestBuffer::NormalPriviledgeRequestBuffer() : in(new PImpl)
{
}

NormalPriviledgeRequestBuffer::~NormalPriviledgeRequestBuffer()
{
}

void NormalPriviledgeRequestBuffer::Append(const char* data, std::size_t size)
{
	if (size == 0) {
		return;
	}

	in->mData.append(data, size);
}

bool NormalPriviledgeRequestBuffer::TryPopRequest(std::string& request)
{
	in->DiscardLeadingNulls();

	auto delimiter = in->mData.find('\n');
	if (delimiter == std::string::npos) {
		return false;
	}

	request.assign(in->mData, 0, delimiter);
	in->mData.erase(0, delimiter + 1);
	return true;
}

bool NormalPriviledgeRequestBuffer::HasPendingData()
{
	in->DiscardLeadingNulls();
	return in->mData.empty() == false;
}

void NormalPriviledgeRequestBuffer::PImpl::DiscardLeadingNulls()
{
	// リクエスト終端のNULを読み取り境界に関係なく取り除く
	while (mData.empty() == false && mData.front() == '\0') {
		mData.erase(0, 1);
	}
}
