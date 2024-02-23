#include "pch.h"
#include "FilterExecutor.h"
#include "PartialMatchPattern.h"
#include "core/CommandParameter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace filter {



struct FilterExecutor::PImpl
{
	std::unique_ptr<Pattern> mPattern;
	std::vector<CString> mCandidates;
};

FilterExecutor::FilterExecutor() : in(new PImpl)
{
	in->mPattern.reset(new PartialMatchPattern());
}

FilterExecutor::~FilterExecutor()
{
}

void FilterExecutor::ClearCandidates()
{
	in->mCandidates.clear();
}

void FilterExecutor::AddCandidates(const CString& item)
{
	in->mCandidates.push_back(item);
}

void FilterExecutor::Execute(const CString& keyword, std::vector<CString>& result)
{
	auto& allCandidates = in->mCandidates;

	// 入力キーワードが空文字の場合は全てを候補に追加する
	if (keyword.IsEmpty()) {
		result.insert(result.end(), allCandidates.begin(), allCandidates.end());
		return;
	}

	// 入力キーワードによる絞り込みを行う
	std::vector<CString> candidatesTmp;
	candidatesTmp.reserve(allCandidates.size());

	Pattern* pattern = in->mPattern.get();

	soyokaze::core::CommandParameter commandParam(keyword);
	pattern->SetParam(commandParam);

	for (auto& candidate : allCandidates) {
		if (pattern->Match(candidate) != Pattern::Mismatch) {
			candidatesTmp.push_back(candidate);
		}
	}

	result.swap(candidatesTmp);
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze


